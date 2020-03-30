/**
 * @brief A program to convert non-linear deformation field file formats
 *
 */

/*
 * Original Author: Oliver Hinds
 *
 * Copyright © 2016 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */

#include "gcamorph.h"
#include "mri_circulars.h"
#include "version.h"

namespace filetypes {
enum FileType { UNKNOWN, M3Z, FSL, ITK, VOX };
}

struct Parameters {
  std::string         in_warp;
  std::string         out_warp;
  std::string         in_src_geom;
  filetypes::FileType in_type;
  filetypes::FileType out_type;
  bool                downsample;
};

static struct Parameters P = {
    "", "", "", filetypes::UNKNOWN, filetypes::UNKNOWN, false};

static void printUsage();
static bool parseCommandLine(int argc, char *argv[], Parameters &P);

static char vcid[] =
    "$Id: mri_warp_convert.cpp,v 1.1 2016/06/16 19:57:06 ohinds Exp $";
const char *Progname = nullptr;

GCAM *readM3Z(const std::string &warp_file)
// Read an m3z file. Just calls down to GCAMread
{
  GCAM *gcam = GCAMread(warp_file.c_str());
  if (gcam == nullptr) {
    std::cerr << "ERROR readM3Z: cannot read " << warp_file << std::endl;
    exit(1);
  }

  return gcam;
}

GCAM *readFSL(const std::string &warp_file)
// Read in an FSL warp. This is the code that used to reside in
// mri_warp_convert.c.
{
  MRI *mri = MRIread(warp_file.c_str());
  if (mri == nullptr)
    ErrorExit(ERROR_NOFILE, "%s: could not read warp volume %s\n", Progname,
              warp_file.c_str());

  MATRIX *m = MRIgetVoxelToRasXform(mri);

  // NOTE: this assumes a standard siemens image orientation in which
  // case a neurological orientation means that the first frame is
  // flipped

  if (MatrixDeterminant(m) > 0) {
    fprintf(stdout, "non-negative Jacobian determinant -- converting to "
                    "radiological ordering\n");
  }
  {
    // 2012/feb/08: tested with anisotropic voxel sizes

    MRI * mri2 = nullptr;
    int   c = 0, r = 0, s = 0;
    float v;

    mri2 = MRIcopy(mri, nullptr);
    for (c = 0; c < mri->width; c++) {
      for (r = 0; r < mri->height; r++) {
        for (s = 0; s < mri->depth; s++) {
          // only flip first frame (by negating relative shifts)
          v = MRIgetVoxVal(mri, c, r, s, 0) / mri->xsize;
          if (MatrixDeterminant(m) > 0)
            MRIsetVoxVal(mri2, c, r, s, 0, -v);
          else
            MRIsetVoxVal(mri2, c, r, s, 0, v);

          v = MRIgetVoxVal(mri, c, r, s, 1) / mri->ysize;
          MRIsetVoxVal(mri2, c, r, s, 1, v);

          v = MRIgetVoxVal(mri, c, r, s, 2) / mri->zsize;
          MRIsetVoxVal(mri2, c, r, s, 2, v);
        }
      }
    }
    MRIfree(&mri);
    mri = mri2;
  }
  MatrixFree(&m);

  // this does all the work! (gcamorph.c)
  GCA_MORPH *gcam = GCAMalloc(mri->width, mri->height, mri->depth);
  GCAMinitVolGeom(gcam, mri, mri);

  // not sure if removing singularities is ever a bad thing
#if 1
  GCAMremoveSingularitiesAndReadWarpFromMRI(gcam, mri);
#else
  GCAMreadWarpFromMRI(gcam, mri);
#endif

  return gcam;
}

GCAM *readITK(const std::string &warp_file, const std::string &src_geom)
// Write an ITK warp file. ITK (and Ants) uses Left Posterior Superior
// coordinates.
{

  MRI *itk = MRIread(warp_file.c_str());
  if (itk == nullptr) {
    std::cerr << "ERROR: couldn't read input ITK warp from " << warp_file
              << std::endl;
    return nullptr;
  }

  MRI *src = MRIread(src_geom.c_str());
  if (src == nullptr) {
    std::cerr << "ERROR: couldn't read source geometry from " << src_geom
              << std::endl;
    return nullptr;
  }

  GCA_MORPH *gcam = GCAMalloc(itk->width, itk->height, itk->depth);
  GCAMinitVolGeom(gcam, src, itk);
  gcam->type = GCAM_VOX;

  MATRIX *ras2lps     = MatrixIdentity(4, nullptr);
  ras2lps->rptr[1][1] = -1;
  ras2lps->rptr[2][2] = -1;

  MATRIX *ref_vox2lps =
      MatrixMultiplyD(ras2lps, MRIgetVoxelToRasXform(itk), nullptr);
  MATRIX *mov_vox2lps =
      MatrixMultiplyD(ras2lps, MRIgetVoxelToRasXform(src), nullptr);
  MATRIX *mov_lps2vox = MatrixInverse(mov_vox2lps, nullptr);

  VECTOR *orig_ind = VectorAlloc(4, MATRIX_REAL);
  VECTOR *dest_ind = VectorAlloc(4, MATRIX_REAL);
  VECTOR *orig_lps = VectorAlloc(4, MATRIX_REAL);
  VECTOR *dest_lps = VectorAlloc(4, MATRIX_REAL);
  VECTOR *warp_lps = VectorAlloc(4, MATRIX_REAL);

  VECTOR_ELT(warp_lps, 4) = 0;
  VECTOR_ELT(orig_ind, 4) = 1;
  for (int s = 0; s < itk->depth; s++) {
    for (int c = 0; c < itk->width; c++) {
      for (int r = 0; r < itk->height; r++) {
        GCA_MORPH_NODE *node = &gcam->nodes[c][r][s];
        node->origx          = c;
        node->origy          = r;
        node->origz          = s;
        node->xn             = c;
        node->yn             = r;
        node->zn             = s;

        VECTOR3_LOAD(orig_ind, c, r, s);
        orig_lps = MatrixMultiplyD(ref_vox2lps, orig_ind, orig_lps);

        VECTOR3_LOAD(warp_lps, MRIgetVoxVal(itk, c, r, s, 0),
                     MRIgetVoxVal(itk, c, r, s, 1),
                     MRIgetVoxVal(itk, c, r, s, 2));
        dest_lps = VectorAdd(orig_lps, warp_lps, dest_lps);
        dest_ind = MatrixMultiplyD(mov_lps2vox, dest_lps, dest_ind);

        node->x = VECTOR_ELT(dest_ind, 1);
        node->y = VECTOR_ELT(dest_ind, 2);
        node->z = VECTOR_ELT(dest_ind, 3);
      }
    }
  }

  MRIfree(&itk);
  MRIfree(&src);
  VectorFree(&orig_ind);
  VectorFree(&orig_lps);
  VectorFree(&warp_lps);
  VectorFree(&dest_lps);
  VectorFree(&dest_ind);
  MatrixFree(&ras2lps);
  MatrixFree(&ref_vox2lps);
  MatrixFree(&mov_vox2lps);
  MatrixFree(&mov_lps2vox);

  return gcam;
}

GCAM *readVOX(const std::string &warp_file)
// Read a warp file with same-geometry image-space displacements.
{

  MRI *vox = MRIread(warp_file.c_str());
  if (vox == nullptr) {
    std::cerr << "ERROR: couldn't read input VOX warp from " << warp_file
              << std::endl;
    return nullptr;
  }

  GCA_MORPH *gcam = GCAMalloc(vox->width, vox->height, vox->depth);
  GCAMinitVolGeom(gcam, vox, vox);
  gcam->type = GCAM_VOX;

  for (int s = 0; s < vox->depth; s++) {
    for (int c = 0; c < vox->width; c++) {
      for (int r = 0; r < vox->height; r++) {
        GCA_MORPH_NODE *node = &gcam->nodes[c][r][s];
        node->origx          = c;
        node->origy          = r;
        node->origz          = s;
        node->xn             = c;
        node->yn             = r;
        node->zn             = s;
        node->x              = c + MRIgetVoxVal(vox, c, r, s, 0);
        node->y              = r + MRIgetVoxVal(vox, c, r, s, 1);
        node->z              = s + MRIgetVoxVal(vox, c, r, s, 2);
      }
    }
  }
  MRIfree(&vox);
  return gcam;
}

void writeM3Z(const std::string &fname, GCAM *gcam, bool downsample = false)
// Write an m3z file. Just calls down to GCAMwrite
{
  GCA_MORPH *out = downsample ? GCAMdownsample2(gcam) : gcam;
  GCAMwrite(out, fname.c_str());
  if (downsample) {
    GCAMfree(&out);
  }
}

void writeFSL(const std::string &fname, const GCAM *gcam)
// Write an FSL warp file.
// NOT IMPLEMENTED
{
  std::cerr << "ERROR writeFSL is not implemented, sorry!" << std::endl;
  exit(1);

  return;
}

void writeITK(const std::string &fname, GCAM *gcam)
// Write an ITK warp file format.
{
  MATRIX *ras2lps     = MatrixIdentity(4, nullptr);
  ras2lps->rptr[1][1] = -1;
  ras2lps->rptr[2][2] = -1;

  MATRIX *ref_vox2ras = VGgetVoxelToRasXform(&gcam->atlas, nullptr, 0);
  MATRIX *ref_vox2lps = MatrixMultiplyD(ras2lps, ref_vox2ras, nullptr);
  MATRIX *mov_vox2ras = VGgetVoxelToRasXform(&gcam->image, nullptr, 0);
  MATRIX *mov_vox2lps = MatrixMultiplyD(ras2lps, mov_vox2ras, nullptr);

  MRI *itk = MRIallocSequence(gcam->atlas.width, gcam->atlas.height,
                              gcam->atlas.depth, MRI_FLOAT, 3);
  MRIsetResolution(itk, gcam->atlas.xsize, gcam->atlas.ysize,
                   gcam->atlas.zsize);
  MRIsetVox2RASFromMatrix(itk, ref_vox2ras);
  MRIcopyVolGeomToMRI(itk, &gcam->atlas);

  int     x, y, z;
  float   xw, yw, zw;
  MATRIX *orig_ind        = VectorAlloc(4, MATRIX_REAL);
  MATRIX *dest_ind        = VectorAlloc(4, MATRIX_REAL);
  VECTOR_ELT(orig_ind, 4) = 1;
  VECTOR_ELT(dest_ind, 4) = 1;
  MATRIX *orig_wld_lps    = VectorAlloc(4, MATRIX_REAL);
  MATRIX *dest_wld_lps    = VectorAlloc(4, MATRIX_REAL);
  bool    samesize = itk->width == gcam->width && itk->height == gcam->height &&
                  itk->depth == gcam->depth;
  for (x = 0; x < itk->width; x++)
    for (y = 0; y < itk->height; y++)
      for (z = 0; z < itk->depth; z++) {
        VECTOR3_LOAD(orig_ind, x, y, z);
        MatrixMultiplyD(ref_vox2lps, orig_ind, orig_wld_lps);
        if (samesize) {
          GCA_MORPH_NODE *node = &gcam->nodes[x][y][z];
          xw                   = node->x;
          yw                   = node->y;
          zw                   = node->z;
        } else {
          GCAMsampleMorph(gcam, x, y, z, &xw, &yw, &zw);
        }
        VECTOR3_LOAD(dest_ind, xw, yw, zw);
        MatrixMultiplyD(mov_vox2lps, dest_ind, dest_wld_lps);

        MRIsetVoxVal(itk, x, y, z, 0,
                     VECTOR_ELT(dest_wld_lps, 1) - VECTOR_ELT(orig_wld_lps, 1));
        MRIsetVoxVal(itk, x, y, z, 1,
                     VECTOR_ELT(dest_wld_lps, 2) - VECTOR_ELT(orig_wld_lps, 2));
        MRIsetVoxVal(itk, x, y, z, 2,
                     VECTOR_ELT(dest_wld_lps, 3) - VECTOR_ELT(orig_wld_lps, 3));
      }
  MatrixFree(&orig_ind);
  MatrixFree(&dest_ind);
  MatrixFree(&orig_wld_lps);
  MatrixFree(&dest_wld_lps);

  if (MRIwriteType(itk, fname.c_str(), ITK_MORPH) != 0) {
    std::cerr << "Error writing ITK warp to " << fname << std::endl;
  }
  MRIfree(&itk);

  MatrixFree(&ras2lps);
  MatrixFree(&ref_vox2ras);
  MatrixFree(&ref_vox2lps);
  MatrixFree(&mov_vox2ras);
  MatrixFree(&mov_vox2lps);

  return;
}

void writeVOX(const std::string &fname, GCAM *gcam)
// Write a warp file with same-geometry image-space displacements.
{
  MATRIX *ref_vox2ras = VGgetVoxelToRasXform(&gcam->atlas, nullptr, 0);
  MRI *   out         = MRIallocSequence(gcam->atlas.width, gcam->atlas.height,
                              gcam->atlas.depth, MRI_FLOAT, 3);
  MRIsetResolution(out, gcam->atlas.xsize, gcam->atlas.ysize,
                   gcam->atlas.zsize);
  MRIsetVox2RASFromMatrix(out, ref_vox2ras);
  MRIcopyVolGeomToMRI(out, &gcam->atlas);

  bool samesize = out->width == gcam->width && out->height == gcam->height &&
                  out->depth == gcam->depth;
  float x, y, z;
  for (int c = 0; c < out->width; c++)
    for (int r = 0; r < out->height; r++)
      for (int s = 0; s < out->depth; s++) {
        if (samesize) {
          GCA_MORPH_NODE *node = &gcam->nodes[c][r][s];
          x                    = node->x;
          y                    = node->y;
          z                    = node->z;
        } else {
          GCAMsampleMorph(gcam, c, r, s, &x, &y, &z);
        }
        MRIsetVoxVal(out, c, r, s, 0, x - c);
        MRIsetVoxVal(out, c, r, s, 1, y - r);
        MRIsetVoxVal(out, c, r, s, 2, z - s);
      }
  if (MRIwrite(out, fname.c_str()) != 0) {
    std::cerr << "Error writing VOX warp to " << fname << std::endl;
  }
  MRIfree(&out);
  MatrixFree(&ref_vox2ras);
}

int main(int argc, char *argv[]) {
  std::cout << vcid << std::endl << std::endl;

  // Default initialization
  int nargs = handleVersionOption(argc, argv, "mri_warp_convert");
  if (nargs && argc - nargs == 1) {
    exit(0);
  }
  argc -= nargs;
  Progname = argv[0];
  argc--;
  argv++;
  ErrorInit(NULL, NULL, NULL);

  // Parse command line
  if (!parseCommandLine(argc, argv, P)) {
    // printUsage();
    exit(1);
  }

  // Read input transform and convert to RAS2RAS:
  GCA_MORPH *gcam = nullptr;
  switch (P.in_type) {
  case filetypes::M3Z:
    gcam = readM3Z(P.in_warp.c_str());
    break;
  case filetypes::FSL:
    gcam = readFSL(P.in_warp.c_str());
    break;
  case filetypes::ITK:
    gcam = readITK(P.in_warp.c_str(), P.in_src_geom);
    break;
  case filetypes::VOX:
    gcam = readVOX(P.in_warp.c_str());
    break;
  default:
    ErrorExit(ERROR_BADFILE, "%s: Unknown input type for %s", Progname,
              P.in_warp.c_str());
  }

  if (!gcam) {
    ErrorExit(ERROR_BADFILE, "%s: can't read input file %s", Progname,
              P.in_warp.c_str());
  }

  switch (P.out_type) {
  case filetypes::M3Z:
    writeM3Z(P.out_warp.c_str(), gcam, P.downsample);
    break;
  case filetypes::FSL:
    writeFSL(P.out_warp.c_str(), gcam);
    break;
  case filetypes::ITK:
    writeITK(P.out_warp.c_str(), gcam);
    break;
  case filetypes::VOX:
    writeVOX(P.out_warp.c_str(), gcam);
    break;
  default:
    ErrorExit(ERROR_BADFILE, "%s: Unknown output type for %s", Progname,
              P.out_warp.c_str());
  }

  GCAMfree(&gcam);
  printf("%s successful.\n", Progname);
  return (0);
}

#include "mri_warp_convert.help.xml.h"
static void printUsage() {
  outputHelpXml(mri_warp_convert_help_xml, mri_warp_convert_help_xml_len);
}

/*!
 \fn int parseNextCommand(int argc, char **argv)
 \brief Parses the command-line for next command
 \param   argc  number of command line arguments
 \param   argv  pointer to a character pointer
 \param      P  reference to parameters
 \returns       number of used arguments for this command
 */
static int parseNextCommand(int argc, char *argv[], Parameters &P) {
  bool have_input  = false;
  bool have_output = false;

  int   nargs = 0;
  char *option;

  option = argv[0] + 1; // remove '-'
  if (option[0] == '-') {
    option = option + 1; // remove second '-'
  }
  StrUpper(option);

  if (!strcmp(option, "INM3Z")) {
    if (have_input) {
      std::cerr << std::endl
                << std::endl
                << "ERROR: Only one input warp can be specified" << std::endl
                << std::endl;
      printUsage();
      exit(1);
    }
    have_input = true;

    P.in_warp = std::string(argv[1]);
    P.in_type = filetypes::M3Z;
    nargs     = 1;
    std::cout << "--inm3z: " << P.in_warp << " input M3Z warp." << std::endl;
  } else if (!strcmp(option, "INFSL")) {
    if (have_input) {
      std::cerr << std::endl
                << std::endl
                << "ERROR: Only one input warp can be specified" << std::endl
                << std::endl;
      printUsage();
      exit(1);
    }
    have_input = true;

    P.in_warp = std::string(argv[1]);
    P.in_type = filetypes::FSL;
    nargs     = 1;
    std::cout << "--infsl: " << P.in_warp << " input FSL warp." << std::endl;
  } else if (!strcmp(option, "INITK")) {
    if (have_input) {
      std::cerr << std::endl
                << std::endl
                << "ERROR: Only one input warp can be specified" << std::endl
                << std::endl;
      printUsage();
      exit(1);
    }
    have_input = true;

    P.in_warp = std::string(argv[1]);
    P.in_type = filetypes::ITK;
    nargs     = 1;
    std::cout << "--initk: " << P.in_warp << " input ITK warp." << std::endl;
  } else if (!strcmp(option, "INVOX")) {
    if (have_input) {
      std::cerr << std::endl
                << std::endl
                << "ERROR: Only one input warp can be specified" << std::endl
                << std::endl;
      printUsage();
      exit(1);
    }
    have_input = true;

    P.in_warp = std::string(argv[1]);
    P.in_type = filetypes::VOX;
    nargs     = 1;
    std::cout << "--invox: " << P.in_warp << " input VOX warp." << std::endl;
  } else if (!strcmp(option, "OUTM3Z")) {
    if (have_output) {
      std::cerr << std::endl
                << std::endl
                << "ERROR: Only one output warp can be specified" << std::endl
                << std::endl;
      printUsage();
      exit(1);
    }
    have_output = true;

    P.out_warp = std::string(argv[1]);
    P.out_type = filetypes::M3Z;
    nargs      = 1;
    std::cout << "--outm3z: " << P.out_warp << " output M3Z." << std::endl;
  } else if (!strcmp(option, "OUTFSL")) {
    if (have_output) {
      std::cerr << std::endl
                << std::endl
                << "ERROR: Only one output warp can be specified" << std::endl
                << std::endl;
      printUsage();
      exit(1);
    }
    have_output = true;

    P.out_warp = std::string(argv[1]);
    P.out_type = filetypes::FSL;
    nargs      = 1;
    std::cout << "--outfsl: " << P.out_warp << " output FSL warp." << std::endl;
  } else if (!strcmp(option, "OUTITK")) {
    if (have_output) {
      std::cerr << std::endl
                << std::endl
                << "ERROR: Only one output warp can be specified" << std::endl
                << std::endl;
      printUsage();
      exit(1);
    }
    have_output = true;

    P.out_warp = std::string(argv[1]);
    P.out_type = filetypes::ITK;
    nargs      = 1;
    std::cout << "--outitk: " << P.out_warp << " output ITK warp." << std::endl;
  } else if (!strcmp(option, "OUTVOX")) {
    if (have_output) {
      std::cerr << std::endl
                << std::endl
                << "ERROR: Only one output warp can be specified" << std::endl
                << std::endl;
      printUsage();
      exit(1);
    }
    have_output = true;

    P.out_warp = std::string(argv[1]);
    P.out_type = filetypes::VOX;
    nargs      = 1;
    std::cout << "--outvox: " << P.out_warp << " output VOX warp." << std::endl;
  } else if (!strcmp(option, "INSRCGEOM")) {
    P.in_src_geom = std::string(argv[1]);
    nargs         = 1;
    std::cout << "--insrcgeom: " << P.in_src_geom << " src image (geometry)."
              << std::endl;
  } else if (!strcmp(option, "DOWNSAMPLE")) {
    if (!P.downsample)
      std::cout << "--downsample: save M3Z warp at half resolution."
                << std::endl;
    P.downsample = true;
    nargs        = 0;
  } else if (!strcmp(option, "HELP")) {
    printUsage();
    exit(1);
  } else {
    std::cerr << std::endl
              << std::endl
              << "ERROR: Option: " << argv[0] << " unknown (see --help) !! "
              << std::endl
              << std::endl;
    exit(1);
  }

  fflush(stdout);

  return (nargs);
}

/*!
 \fn int parseCommandLine(int argc, char **argv)
 \brief Parses the command-line
 \param   argc  number of command line arguments
 \param   argv  pointer to a character pointer
 \param      P  reference to parameters
 \returns       if all necessary parameters were set
 */
static bool parseCommandLine(int argc, char *argv[], Parameters &P) {
  int nargs;
  int inputargs = argc;
  for (; argc > 0 && ISOPTION(*argv[0]); argc--, argv++) {
    nargs = parseNextCommand(argc, argv, P);
    argc -= nargs;
    argv += nargs;
  }

  if (inputargs == 0) {
    printUsage();
    exit(1);
  }

  // validate src geom exists if input type is ITK
  if (P.in_type == filetypes::ITK && P.in_src_geom.empty()) {
    std::cerr << std::endl
              << std::endl
              << "ERROR: ITK input warp requires --insrcgeom" << std::endl
              << std::endl;
    return false;
  }
  if (P.out_type != filetypes::M3Z && P.downsample) {
    std::cerr << std::endl << std::endl;
    std::cerr << "ERROR: --downsample flag only valid for output type M3Z"
              << std::endl
              << std::endl;
    return false;
  }

  return true;
}
