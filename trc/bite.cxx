/**
 * @brief Tractography data and methods for an individual voxel
 *
 * Tractography data and methods for an individual voxel
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

#include <bite.h>

int   Bite::mNumDir, Bite::mNumB0, Bite::mNumTract, Bite::mNumBedpost;
float Bite::mFminPath;
std::vector<unsigned int> Bite::mBaselineImages;
std::vector<float>        Bite::mGradients, Bite::mBvalues;

Bite::Bite(MRI *Dwi, MRI **Phi, MRI **Theta, MRI **F, MRI **V0, MRI **F0,
           MRI *D0, int CoordX, int CoordY, int CoordZ)
    : mCoordX(CoordX), mCoordY(CoordY), mCoordZ(CoordZ) {
  float fsum;
  float vx;
  float vy;
  float vz;

  mDwi.clear();
  mPhiSamples.clear();
  mThetaSamples.clear();
  mFSamples.clear();
  mPhi.clear();
  mTheta.clear();
  mF.clear();

  // DWI intensity values
  for (int idir = 0; idir < mNumDir; idir++) {
    mDwi.push_back(MRIgetVoxVal(Dwi, mCoordX, mCoordY, mCoordZ, idir));
  }

  // Initialize s0
  mS0 = 0;
  for (auto ibase = mBaselineImages.begin(); ibase < mBaselineImages.end();
       ibase++) {
    mS0 += mDwi[*ibase];
  }
  mS0 /= mNumB0;

  // Samples of phi, theta, f
  for (int isamp = 0; isamp < mNumBedpost; isamp++) {
    for (int itract = 0; itract < mNumTract; itract++) {
      mPhiSamples.push_back(
          MRIgetVoxVal(Phi[itract], mCoordX, mCoordY, mCoordZ, isamp));
      mThetaSamples.push_back(
          MRIgetVoxVal(Theta[itract], mCoordX, mCoordY, mCoordZ, isamp));
      mFSamples.push_back(
          MRIgetVoxVal(F[itract], mCoordX, mCoordY, mCoordZ, isamp));
    }
  }

  fsum = 0;
  for (int itract = 0; itract < mNumTract; itract++) {
    // Initialize phi, theta
    vx = MRIgetVoxVal(V0[itract], mCoordX, mCoordY, mCoordZ, 0),
    vy = MRIgetVoxVal(V0[itract], mCoordX, mCoordY, mCoordZ, 1),
    vz = MRIgetVoxVal(V0[itract], mCoordX, mCoordY, mCoordZ, 2);
    mPhi.push_back(atan2(vy, vx));
    mTheta.push_back(acos(vz / sqrt(vx * vx + vy * vy + vz * vz)));

    // Initialize f
    mF.push_back(MRIgetVoxVal(F0[itract], mCoordX, mCoordY, mCoordZ, 0));
    fsum += MRIgetVoxVal(F0[itract], mCoordX, mCoordY, mCoordZ, 0);
  }

  // Initialize d
  mD = MRIgetVoxVal(D0, mCoordX, mCoordY, mCoordZ, 0);
  // mD = log(mDwi[mNumDir-1] / mS0 / (1-fsum);
}

Bite::~Bite() = default;

//
// Set variables that are common for all voxels
//
void Bite::SetStatic(const char *GradientFile, const char *BvalueFile,
                     int NumTract, int NumBedpost, float FminPath) {
  float         val;
  std::ifstream gfile(GradientFile, std::ios::in);
  std::ifstream bfile(BvalueFile, std::ios::in);

  if (!gfile) {
    std::cout << "ERROR: Could not open " << GradientFile << std::endl;
    exit(1);
  }
  if (!bfile) {
    std::cout << "ERROR: Could not open " << BvalueFile << std::endl;
    exit(1);
  }

  std::cout << "Loading b-values from " << BvalueFile << std::endl;
  mBvalues.clear();
  while (bfile >> val) {
    mBvalues.push_back(val);
  }

  mNumDir = mBvalues.size();

  // Find baseline images (the ones with the lowest b-value)
  val = *min_element(mBvalues.begin(), mBvalues.end());
  mBaselineImages.clear();
  for (auto ival = mBvalues.begin(); ival < mBvalues.end(); ival++) {
    if (*ival == val) {
      mBaselineImages.push_back(ival - mBvalues.begin());
    }
  }

  mNumB0 = mBaselineImages.size();

  std::cout << "Loading gradients from " << GradientFile << std::endl;
  mGradients.clear();
  mGradients.resize(3 * mNumDir);
  for (int ii = 0; ii < 3; ii++) {
    for (int idir = 0; idir < mNumDir; idir++) {
      if (!(gfile >> val)) {
        std::cout << "ERROR: Dimensions of " << BvalueFile << " and "
                  << GradientFile << " do not match" << std::endl;
        exit(1);
      }
      mGradients.at(ii + 3 * idir) = val;
    }
  }

  if (gfile >> val) {
    std::cout << "ERROR: Dimensions of " << BvalueFile << " and "
              << GradientFile << " do not match" << std::endl;
    exit(1);
  }

  mNumTract   = NumTract;
  mNumBedpost = NumBedpost;
  mFminPath   = FminPath;
}

int Bite::GetNumTract() { return mNumTract; }

int Bite::GetNumDir() { return mNumDir; }

int Bite::GetNumB0() { return mNumB0; }

int Bite::GetNumBedpost() { return mNumBedpost; }

float Bite::GetLowBvalue() { return mBvalues[mBaselineImages[0]]; }

//
// Draw samples from marginal posteriors of diffusion parameters
//
void Bite::SampleParameters() {
  const int isamp =
      static_cast<int>(round(drand48() * (mNumBedpost - 1))) * mNumTract;
  std::vector<float>::const_iterator samples;

  samples = mPhiSamples.begin() + isamp;
  copy(samples, samples + mNumTract, mPhi.begin());

  samples = mThetaSamples.begin() + isamp;
  copy(samples, samples + mNumTract, mTheta.begin());

  samples = mFSamples.begin() + isamp;
  copy(samples, samples + mNumTract, mF.begin());
}

//
// Compute likelihood given that voxel is off path
//
void Bite::ComputeLikelihoodOffPath() {
  double like = 0;
  auto   ri   = mGradients.begin();
  auto   bi   = mBvalues.begin();
  auto   sij  = mDwi.begin();

  for (int idir = mNumDir; idir > 0; idir--) {
    double       sbar    = 0;
    double       fsum    = 0;
    const double bidj    = (*bi) * mD;
    auto         fjl     = mF.begin();
    auto         phijl   = mPhi.begin();
    auto         thetajl = mTheta.begin();

    for (int itract = mNumTract; itract > 0; itract--) {
      const double iprod =
          (ri[0] * cos(*phijl) + ri[1] * sin(*phijl)) * sin(*thetajl) +
          ri[2] * cos(*thetajl);

      sbar += *fjl * exp(-bidj * iprod * iprod);
      fsum += *fjl;

      fjl++;
      phijl++;
      thetajl++;
    }

    sbar += (1 - fsum) * exp(-bidj);
    sbar *= mS0;
    like += pow(*sij - sbar, 2);

    ri += 3;
    bi++;
    sij++;
  }

  mLikelihood0 = static_cast<float>(log(like / 2)) * mNumDir / 2;
}

//
// Compute likelihood given that voxel is on path
//
void Bite::ComputeLikelihoodOnPath(float PathPhi, float PathTheta) {
  double like = 0;
  auto   ri   = mGradients.begin();
  auto   bi   = mBvalues.begin();
  auto   sij  = mDwi.begin();

  // Choose which anisotropic compartment in voxel corresponds to path
  ChoosePathTractAngle(PathPhi, PathTheta);

  // Calculate likelihood by replacing the chosen tract orientation from path
  for (int idir = mNumDir; idir > 0; idir--) {
    double       sbar    = 0;
    double       fsum    = 0;
    const double bidj    = (*bi) * mD;
    auto         fjl     = mF.begin();
    auto         phijl   = mPhi.begin();
    auto         thetajl = mTheta.begin();

    for (int itract = 0; itract < mNumTract; itract++) {
      double iprod;
      if (itract == mPathTract) {
        iprod = (ri[0] * cos(PathPhi) + ri[1] * sin(PathPhi)) * sin(PathTheta) +
                ri[2] * cos(PathTheta);
      } else {
        iprod = (ri[0] * cos(*phijl) + ri[1] * sin(*phijl)) * sin(*thetajl) +
                ri[2] * cos(*thetajl);
      }

      sbar += *fjl * exp(-bidj * iprod * iprod);
      fsum += *fjl;

      fjl++;
      phijl++;
      thetajl++;
    }

    sbar += (1 - fsum) * exp(-bidj);
    sbar *= mS0;
    like += pow(*sij - sbar, 2);

    ri += 3;
    bi++;
    sij++;
  }

  mLikelihood1 = static_cast<float>(log(like / 2)) * mNumDir / 2;
}

//
// Find tract closest to path orientation
//
void Bite::ChoosePathTractAngle(float PathPhi, float PathTheta) {
  double maxprod = 0;
  auto   fjl     = mF.begin();
  auto   phijl   = mPhi.begin();
  auto   thetajl = mTheta.begin();

  for (int itract = 0; itract < mNumTract; itract++) {
    if (*fjl > mFminPath) {
      const double iprod =
          (cos(PathPhi) * cos(*phijl) + sin(PathPhi) * sin(*phijl)) *
              sin(PathTheta) * sin(*thetajl) +
          cos(PathTheta) * cos(*thetajl);

      if (iprod > maxprod) {
        mPathTract = itract;
        maxprod    = iprod;
      }
    }

    fjl++;
    phijl++;
    thetajl++;
  }

  if (maxprod == 0) {
    mPathTract = 0;
  }
}

//
// Find tract that changes the likelihood the least
//
void Bite::ChoosePathTractLike(float PathPhi, float PathTheta) {
  double mindlike = std::numeric_limits<double>::max();

  for (int jtract = 0; jtract < mNumTract; jtract++) {
    if (mF[jtract] > mFminPath) {
      double dlike;
      double like = 0;
      auto   ri   = mGradients.begin();
      auto   bi   = mBvalues.begin();
      auto   sij  = mDwi.begin();

      // Calculate likelihood by replacing the chosen tract orientation from
      // path
      for (int idir = mNumDir; idir > 0; idir--) {
        double       sbar    = 0;
        double       fsum    = 0;
        const double bidj    = (*bi) * mD;
        auto         fjl     = mF.begin();
        auto         phijl   = mPhi.begin();
        auto         thetajl = mTheta.begin();

        for (int itract = 0; itract < mNumTract; itract++) {
          double iprod;
          if (itract == jtract) {
            iprod =
                (ri[0] * cos(PathPhi) + ri[1] * sin(PathPhi)) * sin(PathTheta) +
                ri[2] * cos(PathTheta);
          } else {
            iprod =
                (ri[0] * cos(*phijl) + ri[1] * sin(*phijl)) * sin(*thetajl) +
                ri[2] * cos(*thetajl);
          }

          sbar += *fjl * exp(-bidj * iprod * iprod);
          fsum += *fjl;

          fjl++;
          phijl++;
          thetajl++;
        }

        sbar += (1 - fsum) * exp(-bidj);
        sbar *= mS0;
        like += pow(*sij - sbar, 2);

        ri += 3;
        bi++;
        sij++;
      }

      like  = log(like / 2) * mNumDir / 2;
      dlike = fabs(like - static_cast<double>(mLikelihood0));

      if (dlike < mindlike) {
        mPathTract = jtract;
        //        mLikelihood1 = (float) like;
        mindlike = dlike;
      }
    }
  }

  if (mindlike == std::numeric_limits<double>::max()) {
    mPathTract = 0;
    //    mLikelihood1 =
  }
}

//
// Compute prior given that voxel is off path
//
void Bite::ComputePriorOffPath() {
  auto fjl     = mF.begin() + mPathTract;
  auto thetajl = mTheta.begin() + mPathTract;

  // cout << (*fjl) << " " << log((*fjl - 1) * log(1 - *fjl)) << " "
  //     << log(((double)*fjl - 1) * log(1 - (double)*fjl)) << endl;

  if (1) {
    mPrior0 = log((*fjl - 1) * log(1 - *fjl)) - log(fabs(sin(*thetajl)));
  } else {
    mPrior0 = 0;
  }
}

//
// Compute prior given that voxel is on path
//
void Bite::ComputePriorOnPath() { mPrior1 = 0; }

bool Bite::IsAllFZero() {
  return (*max_element(mF.begin(), mF.end()) < mFminPath);
}

bool Bite::IsFZero() { return (mF[mPathTract] < mFminPath); }

bool Bite::IsThetaZero() { return (mTheta[mPathTract] == 0); }

float Bite::GetLikelihoodOffPath() { return mLikelihood0; }

float Bite::GetLikelihoodOnPath() { return mLikelihood1; }

float Bite::GetPriorOffPath() { return mPrior0; }

float Bite::GetPriorOnPath() { return mPrior1; }

float Bite::GetPosteriorOffPath() { return mLikelihood0 + mPrior0; }

float Bite::GetPosteriorOnPath() { return mLikelihood1 + mPrior1; }
