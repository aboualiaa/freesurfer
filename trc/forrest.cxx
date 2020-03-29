/**
 * @file  forrest.cxx
 * @brief Random forrest classifier for white-matter segmentation
 *
 * Random forrest classifier for white-matter segmentation
 */
/*
 * Original Author: Anastasia Yendiki
 * CVS Revision Info:
 *
 * Copyright © 2031 The General Hospital Corporation (Boston, MA) "MGH"
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

#include <forrest.h>

const int Forrest::mSampleStep = 2;

Forrest::Forrest()
    : mNx(0), mNy(0), mNz(0), mNumTrain(0), mMask(0), mAseg(0), mOrient(0) {
  int dirs[45] = {0,  0,  0, 1, 0, 0,  -1, 0, 0,  0,  1,  0,  0,  -1, 0,
                  0,  0,  1, 0, 0, -1, 1,  1, 1,  -1, 1,  1,  1,  -1, 1,
                  -1, -1, 1, 1, 1, -1, -1, 1, -1, 1,  -1, -1, -1, -1, -1};

  // Directions in which to look for neighboring labels
  mNumLocal = 15;
  mDirLocal.insert(mDirLocal.begin(), dirs, dirs + 45);

  mNumNear = 14;
  mDirNear.insert(mDirNear.begin(), dirs + 3, dirs + 45);
}

Forrest::~Forrest() {
  MRIfree(&mMask);

  if (mAseg)
    MRIfree(&mAseg);

  if (mOrient)
    MRIfree(&mOrient);
}

//
// Read data for test subject
//
void Forrest::ReadTestSubject(const char *TestDir, const char *MaskFile,
                              const char *AsegFile, const char *OrientFile) {
  std::string dirname = std::string(TestDir) + "/", fname;

  if (mMask)
    MRIfree(&mMask);

  fname = dirname + MaskFile;
  std::cout << "Loading test brain mask from " << fname << std::endl;
  mMask = MRIread(fname.c_str());
  if (!mMask) {
    std::cout << "ERROR: Could not read " << fname << std::endl;
    exit(1);
  }

  mNx = mMask->width;
  mNy = mMask->height;
  mNz = mMask->depth;

  if (mAseg)
    MRIfree(&mAseg);

  if (AsegFile) {
    fname = dirname + AsegFile;
    std::cout << "Loading test anatomical segmentation from " << fname
              << std::endl;
    mAseg = MRIread(fname.c_str());
    if (!mAseg) {
      std::cout << "ERROR: Could not read " << fname << std::endl;
      exit(1);
    }
  }

  if (mOrient)
    MRIfree(&mOrient);

  if (OrientFile) {
    fname = dirname + OrientFile;
    std::cout << "Loading test diffusion orientations from " << fname
              << std::endl;
    mOrient = MRIread(fname.c_str());
    if (!mOrient) {
      std::cout << "ERROR: Could not read " << fname << std::endl;
      exit(1);
    }
  }
}

//
// Read data for training subjects
//
void Forrest::ReadTrainingSubjects(const char *TrainListFile,
                                   const char *MaskFile, const char *AsegFile,
                                   const char *        OrientFile,
                                   std::vector<char *> TractFileList) {
  std::string              dirname;
  std::vector<std::string> dirlist;
  std::ifstream            listfile(TrainListFile, std::ios::in);

  if (!listfile) {
    std::cout << "ERROR: Could not open " << TrainListFile << std::endl;
    exit(1);
  }

  while (listfile >> dirname)
    dirlist.push_back(dirname + "/");

  mNumTrain = 0;

  for (std::vector<std::string>::const_iterator idir = dirlist.begin();
       idir < dirlist.end(); idir++) {
    int                nx, ny, nz;
    MRI *              maskvol = NULL, *asegvol = NULL, *orientvol = NULL;
    std::vector<MRI *> tractvols;
    std::string        fname;

    //
    // Read volumes
    //
    fname = *idir + MaskFile;
    std::cout << "Loading training brain mask from " << fname << std::endl;
    maskvol = MRIread(fname.c_str());
    if (!maskvol) {
      std::cout << "ERROR: Could not read " << fname << std::endl;
      exit(1);
    }

    if (AsegFile) {
      fname = *idir + AsegFile;
      std::cout << "Loading training anatomical segmentation from " << fname
                << std::endl;
      asegvol = MRIread(fname.c_str());
      if (!asegvol) {
        std::cout << "ERROR: Could not read " << fname << std::endl;
        exit(1);
      }
    }

    if (OrientFile) {
      fname = *idir + OrientFile;
      std::cout << "Loading training diffusion orientations from " << fname
                << std::endl;
      orientvol = MRIread(fname.c_str());
      if (!orientvol) {
        std::cout << "ERROR: Could not read " << fname << std::endl;
        exit(1);
      }
    }

    for (std::vector<char *>::const_iterator ifile = TractFileList.begin();
         ifile < TractFileList.end(); ifile++) {
      MRI *tractvol;

      fname = *idir + *ifile;
      std::cout << "Loading training tract label from " << fname << std::endl;
      tractvol = MRIread(fname.c_str());
      if (!tractvol) {
        std::cout << "ERROR: Could not read " << fname << std::endl;
        exit(1);
      }

      tractvols.push_back(tractvol);
    }

    //
    // Save training samples of features
    //
    nx = maskvol->width;
    ny = maskvol->height;
    nz = maskvol->depth;

    for (int iz0 = 0; iz0 < nz; iz0 += mSampleStep)
      for (int iy0 = 0; iy0 < ny; iy0 += mSampleStep)
        for (int ix0 = 0; ix0 < nx; ix0 += mSampleStep)
          if (MRIgetVoxVal(maskvol, ix0, iy0, iz0, 0) > 0) {
            std::vector<unsigned int> tractids;

            mNumTrain++;

            // Save features for this voxel: Spatial location
            mTrainXyz.push_back(ix0);
            mTrainXyz.push_back(iy0);
            mTrainXyz.push_back(iz0);

            // Save features for this voxel: Underlying anatomy
            if (asegvol) {
              const float seg0 = MRIgetVoxVal(asegvol, ix0, iy0, iz0, 0);

              // Save local neighbor labels
              for (std::vector<int>::const_iterator idir = mDirLocal.begin();
                   idir < mDirLocal.end(); idir += 3) {
                const int ix = ix0 + idir[0], iy = iy0 + idir[1],
                          iz = iz0 + idir[2];
                const float seg =
                    MRIgetVoxVal(asegvol, ((ix > -1 && ix < nx) ? ix : ix0),
                                 ((iy > -1 && iy < ny) ? iy : iy0),
                                 ((iz > -1 && iz < nz) ? iz : iz0), 0);

                mTrainAsegIdsLocal.push_back((unsigned int)seg);
              }

              // Save nearest neighbor labels
              for (std::vector<int>::const_iterator idir = mDirNear.begin();
                   idir < mDirNear.end(); idir += 3) {
                int dist = 0, ix = ix0 + idir[0], iy = iy0 + idir[1],
                    iz    = iz0 + idir[2];
                float seg = seg0;

                while ((ix > -1) && (ix < nx) && (iy > -1) && (iy < ny) &&
                       (iz > -1) && (iz < nz) && (seg == seg0)) {
                  seg = MRIgetVoxVal(asegvol, ix, iy, iz, 0);
                  dist++;

                  ix += idir[0];
                  iy += idir[1];
                  iz += idir[2];
                }

                mTrainAsegIdsNear.push_back((unsigned int)seg);
                // mTrainAsegDist.push_back(dist);
              }
            }

            // Save features for this voxel: Diffusion orientation
            if (orientvol) {
              mTrainOrient.push_back(MRIgetVoxVal(orientvol, ix0, iy0, iz0, 0));
              mTrainOrient.push_back(MRIgetVoxVal(orientvol, ix0, iy0, iz0, 1));
              mTrainOrient.push_back(MRIgetVoxVal(orientvol, ix0, iy0, iz0, 2));
            }

            // Does this voxel belong to any tracts?
            for (std::vector<MRI *>::const_iterator ivol = tractvols.begin();
                 ivol < tractvols.end(); ivol++)
              if (MRIgetVoxVal(*ivol, ix0, iy0, iz0, 0) > 0)
                tractids.push_back(ivol - tractvols.begin() + 1);

            mTrainTractIds.push_back(tractids);
          }

    //
    // Free volumes
    //
    MRIfree(&maskvol);

    if (asegvol)
      MRIfree(&asegvol);

    if (orientvol)
      MRIfree(&orientvol);

    std::cout << "bla" << std::endl;
    for (std::vector<MRI *>::iterator ivol = tractvols.begin();
         ivol < tractvols.end(); ivol++)
      MRIfree(&(*ivol));
  }
}

//
// Return volume dimensions based on test subject's mask
//
int Forrest::GetNx() { return mNx; }

int Forrest::GetNy() { return mNy; }

int Forrest::GetNz() { return mNz; }

//
// Return total number of training samples
//
int Forrest::GetNumTrain() { return mNumTrain; }

//
// Check that a point is inside the mask
//
bool Forrest::IsInMask(int CoordX, int CoordY, int CoordZ) {
  return (CoordX > -1) && (CoordX < mNx) && (CoordY > -1) && (CoordY < mNy) &&
         (CoordZ > -1) && (CoordZ < mNz) &&
         (MRIgetVoxVal(mMask, CoordX, CoordY, CoordZ, 0) > 0);
}

//
// Return feature from test subject: Anatomical segmentation
//
std::vector<unsigned int> Forrest::GetTestAseg(int CoordX, int CoordY,
                                               int CoordZ) {
  std::vector<unsigned int> testaseg;

  if (mAseg) {
    const float seg0 = MRIgetVoxVal(mAseg, CoordX, CoordY, CoordZ, 0);

    // Save local neighbor labels
    for (std::vector<int>::const_iterator idir = mDirLocal.begin();
         idir < mDirLocal.end(); idir += 3) {
      const int ix = CoordX + idir[0], iy = CoordY + idir[1],
                iz = CoordZ + idir[2];
      const float seg =
          MRIgetVoxVal(mAseg, ((ix > -1 && ix < mNx) ? ix : CoordX),
                       ((iy > -1 && iy < mNy) ? iy : CoordY),
                       ((iz > -1 && iz < mNz) ? iz : CoordZ), 0);

      testaseg.push_back((unsigned int)seg);
    }

    // Save nearest neighbor labels
    for (std::vector<int>::const_iterator idir = mDirNear.begin();
         idir < mDirNear.end(); idir += 3) {
      int dist = 0, ix = CoordX + idir[0], iy = CoordY + idir[1],
          iz    = CoordZ + idir[2];
      float seg = seg0;

      while ((ix > -1) && (ix < mNx) && (iy > -1) && (iy < mNy) && (iz > -1) &&
             (iz < mNz) && (seg == seg0)) {
        seg = MRIgetVoxVal(mAseg, ix, iy, iz, 0);
        dist++;

        ix += idir[0];
        iy += idir[1];
        iz += idir[2];
      }

      testaseg.push_back((unsigned int)seg);
    }
  }

  return testaseg;
}

//
// Return feature from test subject: Diffusion orientation
//
std::vector<float> Forrest::GetTestOrient(int CoordX, int CoordY, int CoordZ) {
  std::vector<float> testorient;

  if (mOrient) {
    testorient.push_back(MRIgetVoxVal(mOrient, CoordX, CoordY, CoordZ, 0));
    testorient.push_back(MRIgetVoxVal(mOrient, CoordX, CoordY, CoordZ, 1));
    testorient.push_back(MRIgetVoxVal(mOrient, CoordX, CoordY, CoordZ, 2));
  }

  return testorient;
}

//
// Return a training sample of a feature: Spatial location
//
std::vector<int> Forrest::GetTrainXyz(int SampleIndex) {
  std::vector<int>::const_iterator itrain;
  std::vector<int>                 sample;

  if (SampleIndex >= mNumTrain || SampleIndex < 0) {
    std::cout << "ERROR: Cannot access sample " << SampleIndex
              << " out of a total of " << mNumTrain << std::endl;
    exit(1);
  }

  itrain = mTrainXyz.begin() + (SampleIndex - 1) * 3;
  sample.insert(sample.end(), itrain, itrain + 3);

  return sample;
}

//
// Return a training sample of a feature: Anatomical segmentation
//
std::vector<unsigned int> Forrest::GetTrainAseg(int SampleIndex) {
  std::vector<unsigned int>::const_iterator itrain;
  std::vector<unsigned int>                 sample;

  if (SampleIndex >= mNumTrain || SampleIndex < 0) {
    std::cout << "ERROR: Cannot access sample " << SampleIndex
              << " out of a total of " << mNumTrain << std::endl;
    exit(1);
  }

  if (!mTrainAsegIdsLocal.empty()) {
    itrain = mTrainAsegIdsLocal.begin() + (SampleIndex - 1) * mNumLocal;
    sample.insert(sample.end(), itrain, itrain + mNumLocal);
  }

  if (!mTrainAsegIdsNear.empty()) {
    itrain = mTrainAsegIdsNear.begin() + (SampleIndex - 1) * mNumNear;
    sample.insert(sample.end(), itrain, itrain + mNumNear);
  }

  return sample;
}

//
// Return a training sample of a feature: Diffusion orientation
//
std::vector<float> Forrest::GetTrainOrient(int SampleIndex) {
  std::vector<float>::const_iterator itrain;
  std::vector<float>                 sample;

  if (SampleIndex >= mNumTrain || SampleIndex < 0) {
    std::cout << "ERROR: Cannot access sample " << SampleIndex
              << " out of a total of " << mNumTrain << std::endl;
    exit(1);
  }

  if (!mTrainOrient.empty()) {
    itrain = mTrainOrient.begin() + (SampleIndex - 1) * 3;
    sample.insert(sample.end(), itrain, itrain + 3);
  }

  return sample;
}

//
// Return tract membership for a training sample
//
std::vector<unsigned int> Forrest::GetTrainTractIds(int SampleIndex) {
  if (SampleIndex >= mNumTrain || SampleIndex < 0) {
    std::cout << "ERROR: Cannot access sample " << SampleIndex
              << " out of a total of " << mNumTrain << std::endl;
    exit(1);
  }

  return mTrainTractIds[SampleIndex];
}
