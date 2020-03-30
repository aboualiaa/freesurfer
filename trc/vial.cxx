/**
 * @brief Holds utilities for probabilistic tractography
 *
 * Holds utilities for probabilistic tractography
 */
/*
 * Original Author: Anastasia Yendiki
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
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

#include <vial.h>

//
// Affine registration class
//
AffineReg::AffineReg() = default;

AffineReg::AffineReg(std::vector<float> &InToOut) {
  if (InToOut.size() != 16) {
    std::cout << "ERROR: Affine transform must be a 4x4 matrix" << std::endl;
    exit(1);
  }

  mInToOut.resize(16);
  copy(InToOut.begin(), InToOut.end(), mInToOut.begin());
}

AffineReg::~AffineReg() = default;

bool AffineReg::IsEmpty() { return mInToOut.empty(); }

//
// Read affine registration matrix from file and set input/output resolutions
//
void AffineReg::ReadXfm(const char *XfmFile, const MRI *InRefVol,
                        const MRI *OutRefVol) {
  // Read registration matrix from file
  mInToOut.clear();

  if (XfmFile != nullptr) {
    float         val;
    std::ifstream infile(XfmFile, std::ios::in);

    if (!infile) {
      std::cout << "ERROR: Could not open " << XfmFile << std::endl;
      exit(1);
    }

    std::cout << "Loading affine registration from " << XfmFile << std::endl;
    while (infile >> val) {
      mInToOut.push_back(val);
    }

    if (mInToOut.size() != 16) {
      std::cout << "ERROR: File " << XfmFile << " must contain a 4x4 matrix"
                << std::endl;
      exit(1);
    }
  } else { // Identity by default
    float id[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    mInToOut.insert(mInToOut.begin(), id, id + 16);
  }

  // Get resolution of input images
  mInVoxelSize.resize(3);
  if (InRefVol != nullptr) {
    mInVoxelSize[0] = InRefVol->xsize;
    mInVoxelSize[1] = InRefVol->ysize;
    mInVoxelSize[2] = InRefVol->zsize;
  } else {
    fill(mInVoxelSize.begin(), mInVoxelSize.end(), 1.0);
  }

  // Get resolution of output images
  mOutVoxelSize.resize(3);
  if (OutRefVol != nullptr) {
    mOutVoxelSize[0] = OutRefVol->xsize;
    mOutVoxelSize[1] = OutRefVol->ysize;
    mOutVoxelSize[2] = OutRefVol->zsize;
  } else {
    fill(mOutVoxelSize.begin(), mOutVoxelSize.end(), 1.0);
  }
}

//
// Apply an affine transform to a single point
//
void AffineReg::ApplyXfm(std::vector<float> &               OutPoint,
                         std::vector<float>::const_iterator InPoint) {
  auto               in2out = mInToOut.begin();
  std::vector<float> pin;
  std::vector<float> pout;

  pin.resize(4);
  pout.resize(4);

  for (int i = 0; i < 3; i++) {
    pin[i] = InPoint[i] * mInVoxelSize[i];
  }
  pin[3] = 1;

  for (int i = 0; i < 4; i++) {
    float psum = 0;
    for (int j = 0; j < 4; j++) {
      psum += (*in2out) * pin[j];
      in2out++;
    }
    pout[i] = psum;
  }

  for (int i = 0; i < 3; i++) {
    OutPoint[i] = pout[i] / pout[3] / mOutVoxelSize[i];
  }
}

//
// Decompose an affine transform into its parameters
//
void AffineReg::DecomposeXfm() {
  float       t11 = mInToOut[0];
  float       t12 = mInToOut[1];
  float       t13 = mInToOut[2];
  float       t21 = mInToOut[4];
  float       t22 = mInToOut[5];
  float       t23 = mInToOut[6];
  float       t31 = mInToOut[8];
  float       t32 = mInToOut[9];
  float       t33 = mInToOut[10];
  float       yznorm;
  float       zxnorm;
  float       sinx;
  float       cosx;
  float       siny;
  float       cosy;
  float       sinz;
  float       cosz;
  const float x2 = t11 * t11 + t21 * t21 + t31 * t31;
  const float y2 = t12 * t12 + t22 * t22 + t32 * t32;
  const float z2 = t13 * t13 + t23 * t23 + t33 * t33;
  const float xy = t11 * t12 + t21 * t22 + t31 * t32;
  const float yz = t12 * t13 + t22 * t23 + t32 * t33;
  const float zx = t13 * t11 + t23 * t21 + t33 * t31;
  const float cx = 0.0;
  const float cy = 0.0;
  const float cz = 0.0; // TODO: center of geometry

  // Translation
  mTranslate.resize(3);
  mTranslate[0] = mInToOut[3] - cx + t11 * cx + t12 * cy + t13 * cz;
  mTranslate[1] = mInToOut[7] - cy + t21 * cx + t22 * cy + t23 * cz;
  mTranslate[2] = mInToOut[11] - cz + t31 * cx + t32 * cy + t33 * cz;

  // Scaling and shearing
  mScale.resize(3);
  mShear.resize(3);

  mScale[0] = sqrt(x2);
  mScale[1] = sqrt(y2 - xy * xy / x2);
  mShear[0] = xy / (mScale[0] * mScale[1]);
  zxnorm    = zx / mScale[0];
  yznorm    = yz / mScale[1] - mShear[0] * zx / mScale[0];
  mScale[2] = sqrt(z2 - zxnorm * zxnorm - yznorm * yznorm);
  mShear[1] = zxnorm / mScale[2];
  mShear[2] = yznorm / mScale[2];

  // Remove scaling portion of affine transform
  // T = (Rotate * Shear * Scale) * (Scale)^-1
  t11 /= mScale[0];
  t12 /= mScale[1];
  t13 /= mScale[2];
  t21 /= mScale[0];
  t22 /= mScale[1];
  t23 /= mScale[2];
  t31 /= mScale[0];
  t32 /= mScale[1];
  t33 /= mScale[2];

  // Remove shearing portion of affine transform
  // T = (Rotate * Shear) * (Shear)^-1
  t13 += (t11 * (mShear[0] * mShear[2] - mShear[1]) - t12 * mShear[2]);
  t12 += (-t11 * mShear[0]);
  t23 += (t21 * (mShear[0] * mShear[2] - mShear[1]) - t22 * mShear[2]);
  t22 += (-t21 * mShear[0]);
  t33 += (t31 * (mShear[0] * mShear[2] - mShear[1]) - t32 * mShear[2]);
  t32 += (-t31 * mShear[0]);

  // Rotation
  // T = Rotate_x * Rotate_y * Rotate_z
  //   = [ cosy*cosz                    -cosy*sinz                   siny
  //       cosx*sinz + cosz*sinx*siny   cosx*cosz - sinx*siny*sinz   -cosy*sinx
  //       sinx*sinz - cosx*cosz*siny   cosz*sinx + cosx*siny*sinz   cosx*cosy ]
  mRotate.resize(3);
  cosy = sqrt(t11 * t11 + t12 * t12);

  if (cosy < 1e-4) {
    sinx = -t32;
    cosx = t22;
    siny = -t13;

    mRotate[0] = atan2(sinx, cosx);
    mRotate[1] = atan2(siny, static_cast<float>(0.0));
    mRotate[2] = 0.0;
  } else {
    sinx = t23 / cosy;
    cosx = t33 / cosy;
    siny = -t13;
    sinz = t12 / cosy;
    cosz = t11 / cosy;

    mRotate[0] = atan2(sinx, cosx);
    mRotate[1] = atan2(siny, cosy);
    mRotate[2] = atan2(sinz, cosz);
  }
}

//
// Print scaling matrix
//
void AffineReg::PrintScale() {
  if (mScale.empty()) {
    DecomposeXfm();
  }

  std::cout << "Scale matrix:" << std::endl
            << mScale[0] << "\t" << 0 << "\t" << 0 << std::endl
            << 0 << "\t" << mScale[1] << "\t" << 0 << std::endl
            << 0 << "\t" << 0 << "\t" << mScale[2] << std::endl;
}

//
// Print shearing matrix
//
void AffineReg::PrintShear() {
  if (mShear.empty()) {
    DecomposeXfm();
  }

  std::cout << "Shear matrix:" << std::endl
            << 1 << "\t" << mShear[0] << "\t" << mShear[1] << std::endl
            << 0 << "\t" << 1 << "\t" << mShear[2] << std::endl
            << 0 << "\t" << 0 << "\t" << 1 << std::endl;
}

//
// Print rotation matrix
//
void AffineReg::PrintRotate() {
  float sinx;
  float cosx;
  float siny;
  float cosy;
  float sinz;
  float cosz;

  if (mRotate.empty()) {
    DecomposeXfm();
  }

  sinx = sin(mRotate[0]);
  cosx = cos(mRotate[0]);
  siny = sin(mRotate[1]);
  cosy = cos(mRotate[1]);
  sinz = sin(mRotate[2]);
  cosz = cos(mRotate[2]);

  std::cout << "Rotation matrix:" << std::endl
            << cosy * cosz << "\t" << -cosy * sinz << "\t" << siny << std::endl
            << cosx * sinz + cosz * sinx * siny << "\t"
            << cosx * cosz - sinx * siny * sinz << "\t" << -cosy * sinx
            << std::endl
            << sinx * sinz - cosx * cosz * siny << "\t"
            << cosz * sinx + cosx * siny * sinz << "\t" << cosx * cosy
            << std::endl;
}

//
// Return components of affine transform
//
std::vector<float> AffineReg::GetTranslate() {
  if (mTranslate.empty()) {
    DecomposeXfm();
  }

  return mTranslate;
}

std::vector<float> AffineReg::GetRotate() {
  if (mRotate.empty()) {
    DecomposeXfm();
  }

  return mRotate;
}

std::vector<float> AffineReg::GetShear() {
  if (mShear.empty()) {
    DecomposeXfm();
  }

  return mShear;
}

std::vector<float> AffineReg::GetScale() {
  if (mScale.empty()) {
    DecomposeXfm();
  }

  return mScale;
}

#ifndef NO_CVS_UP_IN_HERE
/*
//
// Non-linear registration class
//
NonlinReg::NonlinReg() {
mMorph = std::shared_ptr<gmp::VolumeMorph>(new gmp::VolumeMorph);
}

NonlinReg::~NonlinReg() {}

bool NonlinReg::IsEmpty() { return (mMorph->m_template == 0); }

//
// Read a non-linear transform from file
//
void NonlinReg::ReadXfm(const char *XfmFile, MRI *OutRefVol) {
unsigned int zlibBuffer = 5;

std::ifstream xfile(XfmFile, std::ios::in);	// Just to check if file exists
if (!xfile) {
std::cout << "ERROR: Could not open " << XfmFile << endl;
exit(1);
}
xfile.close();

std::cout << "Loading non-linear registration from " << XfmFile << endl;
mMorph->m_template = OutRefVol;

try {
mMorph->load(XfmFile, zlibBuffer);
}
catch (const char* msg) {
std::cout << "Exception caught while loading registration: " << msg << endl;
exit(1);
}

mMorph->m_interpolationType = SAMPLE_NEAREST;
mMorph->invert();
}

//
// Apply a non-linear transform to a single point
//
void NonlinReg::ApplyXfm(vector<float> &OutPoint,
                 vector<float>::const_iterator InPoint) {
Coords3d inpt, outpt;

for (int k = 0; k < 3; k++)
inpt(k) = InPoint[k];

outpt = mMorph->image(inpt);

for (int k = 0; k < 3; k++)
OutPoint[k] = (float) outpt(k);
}
*/

//
// Non-linear registration class
//
NonlinReg::NonlinReg() : mMorph(nullptr) {}

NonlinReg::~NonlinReg() = default;

bool NonlinReg::IsEmpty() { return (mMorph == nullptr); }

//
// Read a non-linear transform from file
//
void NonlinReg::ReadXfm(const char *XfmFile, MRI *OutRefVol) {
  std::ifstream xfile(XfmFile, std::ios::in); // Just to check if file exists
  if (!xfile) {
    std::cout << "ERROR: Could not open " << XfmFile << std::endl;
    exit(1);
  }
  xfile.close();

  std::cout << "Loading non-linear registration from " << XfmFile << std::endl;
  mMorph = GCAMreadAndInvertNonTal(XfmFile);

  if (mMorph == nullptr) {
    exit(1);
  }

  mMorph->gca = gcaAllocMax(1, 1, 1, OutRefVol->width, OutRefVol->height,
                            OutRefVol->depth, 0, 0);
}

//
// Apply a non-linear transform to a single point
//
void NonlinReg::ApplyXfm(std::vector<float> &               OutPoint,
                         std::vector<float>::const_iterator InPoint) {
  float inpoint[3];

  copy(InPoint, InPoint + 3, inpoint);

  GCAMmorphPlistFromAtlas(1, inpoint, mMorph, &OutPoint[0]);
}

//
// Apply the inverse of a non-linear transform to a single point
//
void NonlinReg::ApplyXfmInv(std::vector<float> &               OutPoint,
                            std::vector<float>::const_iterator InPoint) {
  float inpoint[3];

  copy(InPoint, InPoint + 3, inpoint);

  GCAMmorphPlistToSource(1, inpoint, mMorph, &OutPoint[0]);
}
#endif
