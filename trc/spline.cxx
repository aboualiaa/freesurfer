/**
 * @file  spline.cxx
 * @brief A Catmull-Rom spline
 *
 * A Catmull-Rom spline
 */
/*
 * Original Author: Anastasia Yendiki
 * CVS Revision Info:
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

#include <spline.h>

//
// Compute finite differences of a curve
// This is used for computing tangent vectors and curvatures of streamlines
//
void CurveFiniteDifferences(std::vector<float> &      DiffPoints,
                            const std::vector<float> &CurvePoints,
                            const unsigned int        DiffStep) {
  const unsigned int                 ds2    = 2 * DiffStep;
  const unsigned int                 offset = 3 * DiffStep;
  std::vector<float>::const_iterator ipt;
  std::vector<float>::iterator       iout;
  std::vector<float>                 diff(CurvePoints.size());

  if (DiffPoints.size() != CurvePoints.size()) {
    std::cout << "ERROR: Input and output vector sizes do not match"
              << std::endl;
    exit(1);
  }

  // Approximate derivatives by 2nd-order central finite differences,
  // everywhere except at the start and end of the spline, where 1st-order
  // forward and backward finite differences are used, respectively
  ipt  = CurvePoints.begin();
  iout = diff.begin();

  for (; ipt < CurvePoints.begin() + offset; ipt++) {
    *iout = (*(ipt + offset) - *ipt) / static_cast<float>(DiffStep);
    iout++;
  }

  for (; ipt < CurvePoints.end() - offset; ipt++) {
    *iout = (*(ipt + offset) - *(ipt - offset)) / static_cast<float>(ds2);
    iout++;
  }

  for (; ipt < CurvePoints.end(); ipt++) {
    *iout = (*ipt - *(ipt - offset)) / static_cast<float>(DiffStep);
    iout++;
  }

  // Smooth finite differences
  ipt  = diff.begin();
  iout = DiffPoints.begin();

  for (; ipt < diff.begin() + 3; ipt++) {
    *iout = (*ipt + *(ipt + 3)) / 2.0;
    iout++;
  }

  for (; ipt < diff.end() - 3; ipt++) {
    *iout = (*(ipt - 3) + *ipt + *(ipt + 3)) / 3.0;
    iout++;
  }

  for (; ipt < diff.end(); ipt++) {
    *iout = (*(ipt - 3) + *ipt) / 2.0;
    iout++;
  }
}

//
// Smooth a curve
// This is used before computing finite differences on a streamline
//
void CurveSmooth(std::vector<float> &    SmoothPoints,
                 const std::vector<int> &DiscretePoints) {
  auto ipt     = DiscretePoints.begin();
  auto ismooth = SmoothPoints.begin();

  if (SmoothPoints.size() != DiscretePoints.size()) {
    std::cout << "ERROR: Input and output vector sizes do not match"
              << std::endl;
    exit(1);
  }

  for (; ipt < DiscretePoints.begin() + 3; ipt++) {
    *ismooth = static_cast<float>(*ipt);
    ismooth++;
  }

  for (; ipt < DiscretePoints.end() - 3; ipt++) {
    *ismooth = (*(ipt - 3) + 2 * (*ipt) + *(ipt + 3)) / 4.0;
    ismooth++;
  }

  for (; ipt < DiscretePoints.end(); ipt++) {
    *ismooth = static_cast<float>(*ipt);
    ismooth++;
  }
}

//
// Fill gaps between the points of a curve
// Gaps could result after mapping a curve's points to a higher-resolution space
//
std::vector<int> CurveFill(const std::vector<int> &InPoints) {
  std::vector<int>   outpts;
  std::vector<float> point(3);
  std::vector<float> step(3, 0);

  for (auto ipt = InPoints.begin(); ipt < InPoints.end(); ipt += 3) {
    float dmax = 1; // This will not remove duplicate points

    if (ipt < InPoints.end() - 3) {
      // Calculate step for filling in gap between points
      for (int k = 0; k < 3; k++) {
        float dist = ipt[k + 3] - ipt[k];

        step[k] = dist;
        dist    = fabs(dist);

        if (dist > dmax) {
          dmax = dist;
        }
      }

      if (dmax > 0) {
        for (int k = 0; k < 3; k++) {
          step[k] /= dmax;
        }
      }
    }

    for (int k = 0; k < 3; k++) {
      point[k] = static_cast<float>(ipt[k]);
    }

    for (int istep = static_cast<int>(round(dmax)); istep > 0; istep--) {
      for (int k = 0; k < 3; k++) {
        outpts.push_back(static_cast<int>(round(point[k])));
        point[k] += step[k];
      }
    }
  }

  return outpts;
}

//
// Catmull-Rom cubic spline class
//
Spline::Spline(const char *ControlPointFile, const char *MaskFile) {
  ReadControlPoints(ControlPointFile);
  mMask   = nullptr;
  mVolume = nullptr;
  ReadMask(MaskFile);
}

Spline::Spline(const std::vector<int> &ControlPoints, MRI *Mask) {
  SetControlPoints(ControlPoints);
  mMask   = MRIcopy(Mask, nullptr);
  mVolume = MRIclone(mMask, nullptr);
}

Spline::Spline(const int NumControl, MRI *Mask) : mNumControl(NumControl) {
  mMask   = MRIcopy(Mask, nullptr);
  mVolume = MRIclone(mMask, nullptr);
}

Spline::Spline() : mNumControl(0), mMask(nullptr), mVolume(nullptr) {}

Spline::~Spline() {
  if (mMask != nullptr) {
    MRIfree(&mMask);
  }
  if (mVolume != nullptr) {
    MRIfree(&mVolume);
  }
}

//
// Check if two consecutive control points overlap
//
bool Spline::IsDegenerate() {
  std::vector<int>::const_iterator icpt;

  for (icpt = mControlPoints.begin(); icpt != mControlPoints.end() - 3;
       icpt += 3) {
    if ((icpt[0] == icpt[3]) && (icpt[1] == icpt[4]) && (icpt[2] == icpt[5])) {
      return true;
    }
  }

  return false;
}

//
// Interpolate spline given its control points
//
bool Spline::InterpolateSpline() {
  const int        ncpts = mNumControl - 1;
  auto             icpt  = mControlPoints.begin();
  std::vector<int> newpoint(3);

  if (IsDegenerate()) {
    std::cout << "ERROR: Degenerate spline segment" << std::endl;
    PrintControlPoints();
    return false;
  }

  mAllPoints.clear();
  mArcLength.clear();
  MRIclear(mVolume);

  for (int kcpt = 1; kcpt <= ncpts; kcpt++) {
    float t  = 0;
    float dt = 0;
    float newt;

    // Append current control point to spline
    if (!IsInMask(icpt)) {
      return false;
    }
    mAllPoints.insert(mAllPoints.end(), icpt, icpt + 3);
    mArcLength.push_back(0);
    MRIsetVoxVal(mVolume, icpt[0], icpt[1], icpt[2], 0, 1);

    // Initialize arc length step size
    for (int k = 0; k < 3; k++) {
      dt += pow(icpt[k] - icpt[k + 3], 2);
    }
    dt = 1.0 / sqrt(dt);

    while (t < 1) {
      float tmin = 0;
      float tmax = 1;
      bool  incstep;
      bool  decstep;
      auto  ipt = mAllPoints.end() - 3;

      do {
        newt = t + dt;

        if (newt > 1) {
          incstep = false;
          decstep = true;
        } else {
          // Interpolate new point
          CatmullRomInterpolate(newpoint, newt, (kcpt == 1) ? icpt : (icpt - 3),
                                icpt, icpt + 3,
                                (kcpt == ncpts) ? (icpt + 3) : (icpt + 6));

          // Check that the new point is adjacent to the previous point
          incstep = true;
          decstep = false;

          for (int k = 0; k < 3; k++) {
            switch (abs(ipt[k] - newpoint[k])) {
            case 0:
              break;
            case 1:
              incstep = false;
              break;
            default:
              incstep = false;
              decstep = true;
              break;
            }
          }
        }

        // Adjust arc length step size if neccessary
        if (incstep) {
          tmin = dt;
          dt   = (dt + tmax) / 2;
        } else if (decstep) {
          tmax = dt;
          dt   = (tmin + dt) / 2;
        }
      } while (incstep || decstep);

      t = newt;

      // Check if the next control point has been reached
      if ((newpoint[0] == icpt[3]) && (newpoint[1] == icpt[4]) &&
          (newpoint[2] == icpt[5])) {
        break;
      }

      // Append interpolated point to spline
      if (!IsInMask(newpoint.begin())) {
        return false;
      }
      mAllPoints.insert(mAllPoints.end(), newpoint.begin(), newpoint.end());
      mArcLength.push_back(t);
      MRIsetVoxVal(mVolume, newpoint[0], newpoint[1], newpoint[2], 0, 1);
    }

    icpt += 3;
  }

  // Append final control point to spline
  if (!IsInMask(icpt)) {
    return false;
  }
  mAllPoints.insert(mAllPoints.end(), icpt, icpt + 3);
  mArcLength.push_back(0);
  MRIsetVoxVal(mVolume, icpt[0], icpt[1], icpt[2], 0, 1);

  return true;
}

//
// Fit spline control points to a curve
// Uses "dominant" points on the curve (cf. Park and Lee, Computer-Aided Design
// 39:439-451, 2007)
//
bool Spline::FitControlPoints(const std::vector<int> &InputPoints) {
  bool               success;
  const int          npts   = static_cast<int>(InputPoints.size()) / 3;
  const float        seglen = (npts - 1) / (float)(mNumControl - 1);
  const float        ds     = 1 / seglen;
  const int          lentot = npts - 1;
  const int          mingap = (int)ceil(seglen / 2);
  float              s      = 0;
  std::vector<bool>  isfinal(mNumControl, false);
  std::vector<int>   peeksegs;
  std::vector<float> peekcurvs;
  std::vector<float> arcparam(npts);
  std::vector<std::vector<int>::const_iterator> peekpts;
  std::vector<std::vector<int>::const_iterator> dompts(mNumControl);
  auto                                          iarc = arcparam.begin();

  std::cout << "INFO: Minimum allowable distance between dominant points is "
            << mingap << std::endl;

  mAllPoints.resize(InputPoints.size());
  copy(InputPoints.begin(), InputPoints.end(), mAllPoints.begin());

  ComputeTangent(false);
  ComputeNormal(false);
  ComputeCurvature(false);

  const float curvtot =
      accumulate(mCurvature.begin() + 1, mCurvature.end() - 1, 0.0);
  const float mincurv =
      accumulate(mCurvature.begin() + mingap, mCurvature.end() - mingap, 0.0) /
      (npts - 2 * mingap);

  std::cout << "INFO: Minimum allowable curvature at a dominant point is "
            << mincurv << std::endl;

  // Split curve into segments of equal length
  // We will pick only one dominant point in each of these segments,
  // to ensure that control points end up fairly spread out along the length
  // of the streamline
  for (auto idom = dompts.begin(); idom < dompts.end() - 1; idom++) {
    *idom = mAllPoints.begin() + 3 * static_cast<int>(ceil(s));
    s += seglen;
  }

  *(dompts.end() - 1) = mAllPoints.end() - 3; // Last point

  // Find points where local peeks in curvature occur
  std::cout << "INFO: Points where local peeks in curvature occur are"
            << std::endl;

  for (auto icurv = mCurvature.begin() + mingap;
       icurv < mCurvature.end() - mingap; icurv++) {
    if (*icurv > mincurv && *icurv > *(icurv - 1) && *icurv > *(icurv + 1)) {
      const int kpt = icurv - mCurvature.begin();
      const int idx = (int)floor(ds * kpt);
      auto      ipt = mAllPoints.begin() + 3 * kpt;

      // Save info for this candidate point
      peeksegs.push_back(idx);
      peekcurvs.push_back(*icurv);
      peekpts.push_back(ipt);

      std::cout << " " << *ipt << " " << *(ipt + 1) << " " << *(ipt + 2)
                << " (curv = " << *icurv << ")" << std::endl;
    }
  }

  *isfinal.begin()     = true;
  *(isfinal.end() - 1) = true;

  // Go down the list of candidate points to pick out dominant points
  while (!peekcurvs.empty()) {
    // Find candidate point with maximum curvature
    const int ipeek =
        max_element(peekcurvs.begin(), peekcurvs.end()) - peekcurvs.begin();
    const int                        iseg     = peeksegs.at(ipeek);
    const bool                       isLfinal = isfinal.at(iseg);
    const bool                       isRfinal = isfinal.at(iseg + 1);
    std::vector<int>::iterator       imatch;
    std::vector<int>::const_iterator ipt   = peekpts.at(ipeek);
    std::vector<int>::const_iterator iptL  = dompts.at(iseg);
    std::vector<int>::const_iterator iptR  = dompts.at(iseg + 1);
    const int                        distL = (ipt - iptL) / 3;
    const int                        distR = (iptR - ipt) / 3;

    // Find which end of the segment the candidate point is closest to
    if (distL < distR && !isLfinal && !(isRfinal && distR <= mingap)) {
      // Move left end of segment to this dominant point
      dompts.at(iseg)  = ipt;
      isfinal.at(iseg) = true;
    } else if (!isRfinal && !(isLfinal && distL <= mingap)) {
      // Move right end of segment to this dominant point
      dompts.at(iseg + 1)  = ipt;
      isfinal.at(iseg + 1) = true;
    }

    // Remove point from list of candidate points
    peeksegs.erase(peeksegs.begin() + ipeek);
    peekcurvs.erase(peekcurvs.begin() + ipeek);
    peekpts.erase(peekpts.begin() + ipeek);

    // Also remove any other candidate points that are in the same segment
    imatch = find(peeksegs.begin(), peeksegs.end(), iseg);

    while (imatch != peeksegs.end()) {
      int irem = imatch - peeksegs.begin();

      peeksegs.erase(peeksegs.begin() + irem);
      peekcurvs.erase(peekcurvs.begin() + irem);
      peekpts.erase(peekpts.begin() + irem);

      imatch = find(peeksegs.begin(), peeksegs.end(), iseg);
    }
  }

  std::cout << "INFO: Intermediate dominant points are" << std::endl;
  for (auto idom = dompts.begin(); idom < dompts.end(); idom++) {
    std::cout << " " << *(*idom) << " " << *(*idom + 1) << " " << *(*idom + 2)
              << std::endl;
  }

  // Find segment ends that have not been moved yet
  for (auto ifinal = isfinal.begin(); ifinal < isfinal.end(); ifinal++) {
    if (!*ifinal) {
      const int kdom = ifinal - isfinal.begin();
      int       lenL;
      int       lenR;
      float     curvL;
      float     curvR;
      float     dshape;
      float     dshapemin = std::numeric_limits<double>::infinity();
      std::vector<int>::const_iterator iptL = dompts.at(kdom - 1);
      std::vector<int>::const_iterator iptR = dompts.at(kdom + 1);
      auto icurv = mCurvature.begin() + (iptL - mAllPoints.begin()) / 3;

      // Find point in the segment between the two neighboring dominant points
      // to split the segment into 2 subsegments with minimum difference in
      // "shape index" (to balance the subsegments' total curvature and length)
      lenL = mingap;
      lenR = (iptR - iptL) / 3 - mingap;

      curvL = accumulate(icurv + 1, icurv + mingap - 1, 0.0);
      curvR = accumulate(icurv + mingap + 1, icurv + (iptR - iptL) / 3, 0.0);

      iptL += 3 * mingap;
      iptR -= 3 * mingap;
      icurv += mingap;

      for (auto ipt = iptL; ipt <= iptR; ipt += 3) {
        dshape = fabs((lenL - lenR) / static_cast<float>(lentot) +
                      (curvL - curvR) / curvtot);

        if (dshape < dshapemin) {
          dompts.at(kdom) = ipt;
          dshape          = dshapemin;
        }

        lenL++;
        lenR--;

        curvL += *icurv;
        curvR -= *(icurv + 1);

        icurv++;
      }
    }
  }

  std::cout << "INFO: Final dominant points are" << std::endl;
  for (auto idom = dompts.begin(); idom < dompts.end(); idom++) {
    std::cout << " " << *(*idom) << " " << *(*idom + 1) << " " << *(*idom + 2)
              << std::endl;
  }

  // Parameterize the curve based on these dominant points
  for (int kdom = 1; kdom < mNumControl; kdom++) {
    const int seglen =
        static_cast<int>(dompts.at(kdom) - dompts.at(kdom - 1)) / 3;
    const float ds = 1.0 / seglen;
    float       s  = kdom - 1.0;

    for (int kpt = seglen; kpt > 0; kpt--) {
      *iarc = s;
      s += ds;
      iarc++;
    }
  }

  *iarc = static_cast<float>(mNumControl - 1);

  CatmullRomFit(InputPoints, arcparam);

  success = InterpolateSpline();

  // If spline interpolation after fitting to these dominant points fails,
  // default to equidistant dominant points
  if (!success) {
    std::cout << "WARN: Defaulting to equidistant dominant points" << std::endl;

    CatmullRomFit(InputPoints);

    success = InterpolateSpline();
  }

  return success;
}

//
// Find which spline segment a point belongs to
//
unsigned int Spline::PointToSegment(unsigned int PointIndex) {
  unsigned int iseg = 0;

  if (PointIndex > mArcLength.size() - 1) {
    std::cout << "ERROR: Specified point index (" << PointIndex
              << ") is outside acceptable range (0-" << mArcLength.size() - 1
              << ")" << std::endl;
    exit(1);
  }

  for (auto iarc = mArcLength.begin() + PointIndex; iarc > mArcLength.begin();
       iarc--) {
    if (*iarc < *(iarc - 1)) {
      iseg++;
    }
  }

  return iseg;
}

//
// Compute tangent vectors along spline (either from analytical derivatives
// or from finite difference approximation)
//
void Spline::ComputeTangent(const bool DoAnalytical) {
  std::vector<float>::const_iterator id1;

  if (DoAnalytical) { // Compute analytically using parametric form
    int                              kcpt  = 1;
    const int                        ncpts = mNumControl - 1;
    auto                             icpt  = mControlPoints.begin();
    std::vector<int>::const_iterator ipt;
    auto                             iarc = mArcLength.begin();
    std::vector<float>               newderiv(3);

    mDerivative1.clear();

    for (ipt = mAllPoints.begin(); ipt != mAllPoints.end() - 3; ipt += 3) {
      CatmullRomDerivative1(newderiv, *iarc, (kcpt == 1) ? icpt : (icpt - 3),
                            icpt, icpt + 3,
                            (kcpt == ncpts) ? (icpt + 3) : (icpt + 6));
      mDerivative1.insert(mDerivative1.end(), newderiv.begin(), newderiv.end());

      iarc++;
      if (*iarc == 0) { // Have reached next control point
        kcpt++;
        icpt += 3;
      }
    }

    // First derivative at final control point
    CatmullRomDerivative1(newderiv, *iarc, icpt - 3, icpt, icpt, icpt);
    mDerivative1.insert(mDerivative1.end(), newderiv.begin(), newderiv.end());

    id1 = mDerivative1.begin();
  } else { // Approximate using finite differences
    const unsigned int diffstep = 3;
    std::vector<float> smoothpts(mAllPoints.size());

    mFiniteDifference1.resize(mAllPoints.size());

    // Smooth discrete point coordinates
    CurveSmooth(smoothpts, mAllPoints);

    // Approximate first derivative by smoothed finite differences
    // of the point coordinates
    CurveFiniteDifferences(mFiniteDifference1, smoothpts, diffstep);

    id1 = mFiniteDifference1.begin();
  }

  // Compute tangent vectors from first derivative
  mTangent.resize(mAllPoints.size());

  for (auto itang = mTangent.begin(); itang < mTangent.end(); itang += 3) {
    const float nrm = sqrt(id1[0] * id1[0] + id1[1] * id1[1] + id1[2] * id1[2]);

    if (nrm > 0) { // Normalize
      for (int k = 0; k < 3; k++) {
        itang[k] = id1[k] / nrm;
      }
    } else {
      fill(itang, itang + 3, 0.0);
    }

    id1 += 3;
  }
}

//
// Compute normal vectors along spline (either from analytical derivatives
// or from finite difference approximation)
//
void Spline::ComputeNormal(const bool DoAnalytical) {
  std::vector<float>::const_iterator id2;
  std::vector<float>::const_iterator itang = mTangent.begin();

  if (DoAnalytical) { // Compute analytically using parametric form
    int                              kcpt  = 1;
    const int                        ncpts = mNumControl - 1;
    auto                             icpt  = mControlPoints.begin();
    std::vector<int>::const_iterator ipt;
    auto                             iarc = mArcLength.begin();
    std::vector<float>               newderiv(3);

    mDerivative2.clear();

    for (ipt = mAllPoints.begin(); ipt != mAllPoints.end() - 3; ipt += 3) {
      CatmullRomDerivative2(newderiv, *iarc, (kcpt == 1) ? icpt : (icpt - 3),
                            icpt, icpt + 3,
                            (kcpt == ncpts) ? (icpt + 3) : (icpt + 6));
      mDerivative2.insert(mDerivative2.end(), newderiv.begin(), newderiv.end());

      iarc++;
      if (*iarc == 0) { // Have reached next control point
        kcpt++;
        icpt += 3;
      }
    }

    // Second derivative at final control point
    CatmullRomDerivative2(newderiv, *iarc, icpt - 3, icpt, icpt, icpt);
    mDerivative2.insert(mDerivative2.end(), newderiv.begin(), newderiv.end());

    id2 = mDerivative2.begin();
  } else { // Approximate using finite differences
    const unsigned int diffstep = 3;

    mFiniteDifference2.resize(mFiniteDifference1.size());

    // Approximate second derivative by smoothed finite differences
    // of the first derivative
    CurveFiniteDifferences(mFiniteDifference2, mFiniteDifference1, diffstep);

    id2 = mFiniteDifference2.begin();
  }

  // Compute normal vectors from first and second derivative
  mNormal.resize(mAllPoints.size());

  for (auto inorm = mNormal.begin(); inorm < mNormal.end(); inorm += 3) {
    const float dot = id2[0] * itang[0] + id2[1] * itang[1] + id2[2] * itang[2];
    float       nrm = 0;

    for (int k = 0; k < 3; k++) {
      inorm[k] = id2[k] - dot * itang[k];
      nrm += inorm[k] * inorm[k];
    }

    nrm = sqrt(nrm);

    if (nrm > 0) { // Normalize
      for (int k = 0; k < 3; k++) {
        inorm[k] /= nrm;
      }
    }

    id2 += 3;
    itang += 3;
  }
}

//
// Compute curvature along spline (either from analytical derivatives
// or from finite difference approximation)
//
void Spline::ComputeCurvature(const bool DoAnalytical) {
  std::vector<float>::const_iterator id1;
  std::vector<float>::const_iterator id2;

  mCurvature.resize(mAllPoints.size() / 3);

  if (DoAnalytical) { // Compute analytically using parametric form
    id1 = mDerivative1.begin();
    id2 = mDerivative2.begin();
  } else { // Approximate using finite differences
    id1 = mFiniteDifference1.begin();
    id2 = mFiniteDifference2.begin();
  }

  // Curvature = |r' x r''| / |r'|^3
  for (auto icurv = mCurvature.begin(); icurv < mCurvature.end(); icurv++) {
    *icurv = sqrt((pow(id1[1] * id2[2] - id1[2] * id2[1], 2) +
                   pow(id1[2] * id2[0] - id1[0] * id2[2], 2) +
                   pow(id1[0] * id2[1] - id1[1] * id2[0], 2)) /
                  pow(pow(id1[0], 2) + pow(id1[1], 2) + pow(id1[2], 2), 3));

    id1 += 3;
    id2 += 3;
  }
}

//
// Read control points from file
//
void Spline::ReadControlPoints(const char *ControlPointFile) {
  float         coord;
  std::ifstream infile(ControlPointFile, std::ios::in);

  if (!infile) {
    std::cout << "ERROR: Could not open " << ControlPointFile << std::endl;
    exit(1);
  }

  std::cout << "Loading spline control points from " << ControlPointFile
            << std::endl;
  mControlPoints.clear();
  while (infile >> coord) {
    mControlPoints.push_back(static_cast<int>(round(coord)));
  }

  if (mControlPoints.size() % 3 != 0) {
    std::cout << "ERROR: File " << ControlPointFile
              << " must contain triplets of coordinates" << std::endl;
    exit(1);
  }

  mNumControl = mControlPoints.size() / 3;
}

//
// Read mask volume from file
//
void Spline::ReadMask(const char *MaskFile) {
  if (mMask != nullptr) {
    MRIfree(&mMask);
  }
  if (mVolume != nullptr) {
    MRIfree(&mVolume);
  }

  std::cout << "Loading spline mask from " << MaskFile << std::endl;
  mMask   = MRIread(MaskFile);
  mVolume = MRIclone(mMask, nullptr);
}

//
// Set control points
//
void Spline::SetControlPoints(const std::vector<int> &ControlPoints) {
  mControlPoints.resize(ControlPoints.size());
  copy(ControlPoints.begin(), ControlPoints.end(), mControlPoints.begin());

  mNumControl = mControlPoints.size() / 3;
}

//
// Set mask volume
//
void Spline::SetMask(MRI *Mask) {
  if (mMask != nullptr) {
    MRIfree(&mMask);
  }
  if (mVolume != nullptr) {
    MRIfree(&mVolume);
  }

  mMask   = MRIcopy(Mask, nullptr);
  mVolume = MRIclone(mMask, nullptr);
}

//
// Write spline to volume
//
void Spline::WriteVolume(const char *VolumeFile, const bool ShowControls) {
  if (ShowControls) {
    for (auto icpt = mControlPoints.begin(); icpt != mControlPoints.end();
         icpt += 3) {
      MRIsetVoxVal(mVolume, icpt[0], icpt[1], icpt[2], 0, 2);
    }
  }

  std::cout << "Writing spline volume to " << VolumeFile << std::endl;
  MRIwrite(mVolume, VolumeFile);
}

//
// Write spline coordinates to file
//
void Spline::WriteAllPoints(const char *TextFile) {
  std::ofstream outfile(TextFile, std::ios::out);
  if (!outfile) {
    std::cout << "ERROR: Could not open " << TextFile << " for writing"
              << std::endl;
    exit(1);
  }

  std::cout << "Writing coordinates along spline to " << TextFile << std::endl;

  for (auto ipt = mAllPoints.begin(); ipt < mAllPoints.end(); ipt += 3) {
    outfile << ipt[0] << " " << ipt[1] << " " << ipt[2] << std::endl;
  }
}

//
// Write tangent vectors along spline to file
//
void Spline::WriteTangent(const char *TextFile) {
  std::ofstream outfile(TextFile, std::ios::out);
  if (!outfile) {
    std::cout << "ERROR: Could not open " << TextFile << " for writing"
              << std::endl;
    exit(1);
  }

  std::cout << "Writing tangent vectors along spline to " << TextFile
            << std::endl;

  for (auto itang = mTangent.begin(); itang < mTangent.end(); itang += 3) {
    outfile << itang[0] << " " << itang[1] << " " << itang[2] << std::endl;
  }
}

//
// Write normal vectors along spline to file
//
void Spline::WriteNormal(const char *TextFile) {
  std::ofstream outfile(TextFile, std::ios::out);
  if (!outfile) {
    std::cout << "ERROR: Could not open " << TextFile << " for writing"
              << std::endl;
    exit(1);
  }

  std::cout << "Writing normal vectors along spline to " << TextFile
            << std::endl;

  for (auto inorm = mNormal.begin(); inorm < mNormal.end(); inorm += 3) {
    outfile << inorm[0] << " " << inorm[1] << " " << inorm[2] << std::endl;
  }
}

//
// Write curvatures along spline to file
//
void Spline::WriteCurvature(const char *TextFile) {
  std::ofstream outfile(TextFile, std::ios::out);
  if (!outfile) {
    std::cout << "ERROR: Could not open " << TextFile << " for writing"
              << std::endl;
    exit(1);
  }

  std::cout << "Writing curvatures along spline to " << TextFile << std::endl;

  for (auto icurv = mCurvature.begin(); icurv < mCurvature.end(); icurv++) {
    outfile << *icurv << std::endl;
  }
}

//
// Write the intensity values of each of a set of input volumes along the spline
//
void Spline::WriteValues(std::vector<MRI *> &ValueVolumes,
                         const char *        TextFile) {
  std::ofstream outfile(TextFile, std::ios::app);
  if (!outfile) {
    std::cout << "ERROR: Could not open " << TextFile << " for writing"
              << std::endl;
    exit(1);
  }

  std::cout << "Writing values along spline to " << TextFile << std::endl;

  for (auto ipt = mAllPoints.begin(); ipt < mAllPoints.end(); ipt += 3) {
    outfile << ipt[0] << " " << ipt[1] << " " << ipt[2];

    for (auto ivol = ValueVolumes.begin(); ivol < ValueVolumes.end(); ivol++) {
      outfile << " " << MRIgetVoxVal(*ivol, ipt[0], ipt[1], ipt[2], 0);
    }

    outfile << std::endl;
  }
}

//
// Compute average intensity of each of a set of input volumes along the spline
//
std::vector<float> Spline::ComputeAvg(std::vector<MRI *> &ValueVolumes) {
  int                          nvox = static_cast<int>(mAllPoints.size()) / 3;
  std::vector<float>           avg(ValueVolumes.size(), 0);
  std::vector<float>::iterator iavg;

  for (auto ipt = mAllPoints.begin(); ipt < mAllPoints.end(); ipt += 3) {
    iavg = avg.begin();

    for (auto ivol = ValueVolumes.begin(); ivol < ValueVolumes.end(); ivol++) {
      *iavg += MRIgetVoxVal(*ivol, ipt[0], ipt[1], ipt[2], 0);
      iavg++;
    }
  }

  for (iavg = avg.begin(); iavg < avg.end(); iavg++) {
    *iavg /= nvox;
  }

  return avg;
}

//
// Print control point coordinates
//
void Spline::PrintControlPoints() {
  for (auto icpt = mControlPoints.begin(); icpt != mControlPoints.end();
       icpt += 3) {
    std::cout << icpt[0] << " " << icpt[1] << " " << icpt[2] << std::endl;
  }
}

//
// Print spline coordinates
//
void Spline::PrintAllPoints() {
  for (auto ipt = mAllPoints.begin(); ipt != mAllPoints.end(); ipt += 3) {
    std::cout << ipt[0] << " " << ipt[1] << " " << ipt[2] << std::endl;
  }
}

//
// Print tangent vectors along spline
//
void Spline::PrintTangent() {
  for (auto itang = mTangent.begin(); itang != mTangent.end(); itang += 3) {
    std::cout << itang[0] << " " << itang[1] << " " << itang[2] << std::endl;
  }
}

//
// Print normal vectors along spline
//
void Spline::PrintNormal() {
  for (auto inorm = mNormal.begin(); inorm != mNormal.end(); inorm += 3) {
    std::cout << inorm[0] << " " << inorm[1] << " " << inorm[2] << std::endl;
  }
}

//
// Print curvatures along spline
//
void Spline::PrintCurvature() {
  for (auto icurv = mCurvature.begin(); icurv != mCurvature.end(); icurv++) {
    std::cout << *icurv << std::endl;
  }
}

//
// Return pointer to start of control points
//
std::vector<int>::const_iterator Spline::GetControlPointsBegin() {
  return mControlPoints.begin();
}

//
// Return pointer to end of control points
//
std::vector<int>::const_iterator Spline::GetControlPointsEnd() {
  return mControlPoints.end();
}

//
// Return pointer to beginning of control points
//
std::vector<int>::const_iterator Spline::GetAllPointsBegin() {
  return mAllPoints.begin();
}

//
// Return pointer to end of control points
//
std::vector<int>::const_iterator Spline::GetAllPointsEnd() {
  return mAllPoints.end();
}

//
// Return pointer to beginning of tangent vectors
//
std::vector<float>::const_iterator Spline::GetTangentBegin() {
  return mTangent.begin();
}

//
// Return pointer to end of tangent vectors
//
std::vector<float>::const_iterator Spline::GetTangentEnd() {
  return mTangent.end();
}

//
// Return pointer to beginning of normal vectors
//
std::vector<float>::const_iterator Spline::GetNormalBegin() {
  return mNormal.begin();
}

//
// Return pointer to end of normal vectors
//
std::vector<float>::const_iterator Spline::GetNormalEnd() {
  return mNormal.end();
}

//
// Return pointer to beginning of curvatures
//
std::vector<float>::const_iterator Spline::GetCurvatureBegin() {
  return mCurvature.begin();
}

//
// Return pointer to end of curvatures
//
std::vector<float>::const_iterator Spline::GetCurvatureEnd() {
  return mCurvature.end();
}

//
// Intepolate Catmull-Rom spline from control points
//
void Spline::CatmullRomInterpolate(
    std::vector<int> &InterpPoint, const float t,
    std::vector<int>::const_iterator ControlPoint1,
    std::vector<int>::const_iterator ControlPoint2,
    std::vector<int>::const_iterator ControlPoint3,
    std::vector<int>::const_iterator ControlPoint4) {
  const float t2 = t * t;
  const float t3 = t2 * t;
  const float a  = -.5 * (t3 + t) + t2;
  const float b  = 1.5 * t3 - 2.5 * t2 + 1;
  const float c  = -1.5 * t3 + 2 * t2 + .5 * t;
  const float d  = .5 * (t3 - t2);

  for (int k = 0; k < 3; k++) {
    InterpPoint[k] =
        static_cast<int>(round(a * ControlPoint1[k] + b * ControlPoint2[k] +
                               c * ControlPoint3[k] + d * ControlPoint4[k]));
  }
}

//
// Compute Catmull-Rom spline first derivative vector from control points
//
void Spline::CatmullRomDerivative1(
    std::vector<float> &InterpDerivative, const float t,
    std::vector<int>::const_iterator ControlPoint1,
    std::vector<int>::const_iterator ControlPoint2,
    std::vector<int>::const_iterator ControlPoint3,
    std::vector<int>::const_iterator ControlPoint4) {
  const float t2 = t * t;
  const float a  = -1.5 * t2 + 2 * t - .5;
  const float b  = 4.5 * t2 - 5 * t;
  const float c  = -4.5 * t2 + 4 * t + .5;
  const float d  = 1.5 * t2 - t;

  for (int k = 0; k < 3; k++) {
    InterpDerivative[k] = a * ControlPoint1[k] + b * ControlPoint2[k] +
                          c * ControlPoint3[k] + d * ControlPoint4[k];
  }
}

//
// Compute Catmull-Rom spline second derivative vector from control points
//
void Spline::CatmullRomDerivative2(
    std::vector<float> &InterpDerivative, const float t,
    std::vector<int>::const_iterator ControlPoint1,
    std::vector<int>::const_iterator ControlPoint2,
    std::vector<int>::const_iterator ControlPoint3,
    std::vector<int>::const_iterator ControlPoint4) {
  const float a = -3 * t + 2;
  const float b = 9 * t - 5;
  const float c = -9 * t + 4;
  const float d = 3 * t - 1;

  for (int k = 0; k < 3; k++) {
    InterpDerivative[k] = a * ControlPoint1[k] + b * ControlPoint2[k] +
                          c * ControlPoint3[k] + d * ControlPoint4[k];
  }
}

//
// Least-squares fit of a Catmull-Rom spline to a set points,
// given the coordinates of the points but no user-specified parameterization
// (default to parameterizing the curve based on equidistant dominant points)
//
void Spline::CatmullRomFit(const std::vector<int> &InputPoints) {
  const int          npts = static_cast<int>(InputPoints.size()) / 3;
  const float        smax = (float)(mNumControl - 1);
  const float        ds   = smax / (npts - 1);
  float              s    = 0.0;
  std::vector<float> arcparam(npts);

  for (auto iarc = arcparam.begin(); iarc < arcparam.end(); iarc++) {
    *iarc = s;
    s += ds;
  }

  CatmullRomFit(InputPoints, arcparam);
}

//
// Least-squares fit of a Catmull-Rom spline to a set of points,
// given the coordinates of the points and the respective arc length
// parameter values, which should be reals in the interval [0, mNumControl-1]
//
void Spline::CatmullRomFit(const std::vector<int> &  InputPoints,
                           const std::vector<float> &ArcLengthParameter) {
  const int                  npts = static_cast<int>(InputPoints.size()) / 3;
  auto                       ipt  = InputPoints.begin() + 3;
  std::vector<int>::iterator icpt;
  auto                       iarc = ArcLengthParameter.begin() + 1;
  MATRIX *                   A    = NULL;
  MATRIX *                   Ap   = NULL;
  MATRIX *                   y    = NULL;
  MATRIX *                   x    = NULL;

  // Fit all but the first and last control points,
  // which will be set equal to the first and last of the input points
  A  = MatrixAlloc(npts - 2, mNumControl - 2, MATRIX_REAL);
  Ap = MatrixAlloc(mNumControl - 2, npts - 2, MATRIX_REAL);
  y  = MatrixAlloc(npts - 2, 3, MATRIX_REAL);
  x  = MatrixAlloc(mNumControl - 2, 3, MATRIX_REAL);

  for (int irow = 1; irow < npts - 1; irow++) {
    const int   idx = static_cast<int>(floor(*iarc));
    const float t   = *iarc - floor(*iarc);
    const float t2  = t * t;
    const float t3  = t2 * t;
    const float a   = -.5 * (t3 + t) + t2;
    const float b   = 1.5 * t3 - 2.5 * t2 + 1;
    const float c   = -1.5 * t3 + 2 * t2 + .5 * t;
    const float d   = .5 * (t3 - t2);

    if (idx == 0) {
      const float ab   = a + b;
      auto        ipt1 = InputPoints.begin();

      for (int k = 1; k < 4; k++) {
        y->rptr[irow][k] = *ipt - ab * (*ipt1);
        ipt++;
        ipt1++;
      }

      A->rptr[irow][1] = c;
      A->rptr[irow][2] = d;
    } else if (idx == 1) {
      auto ipt1 = InputPoints.begin();

      for (int k = 1; k < 4; k++) {
        y->rptr[irow][k] = *ipt - a * (*ipt1);
        ipt++;
        ipt1++;
      }

      A->rptr[irow][1] = b;
      A->rptr[irow][2] = c;
      A->rptr[irow][3] = d;
    } else if (idx == mNumControl - 3) {
      auto iptn = InputPoints.end() - 3;

      for (int k = 1; k < 4; k++) {
        y->rptr[irow][k] = *ipt - d * (*iptn);
        ipt++;
        iptn++;
      }

      A->rptr[irow][mNumControl - 4] = a;
      A->rptr[irow][mNumControl - 3] = b;
      A->rptr[irow][mNumControl - 2] = c;
    } else if (idx == mNumControl - 2) {
      const float cd   = c + d;
      auto        iptn = InputPoints.end() - 3;

      for (int k = 1; k < 4; k++) {
        y->rptr[irow][k] = *ipt - cd * (*iptn);
        ipt++;
        iptn++;
      }

      A->rptr[irow][mNumControl - 3] = a;
      A->rptr[irow][mNumControl - 2] = b;
    } else {
      for (int k = 1; k < 4; k++) {
        y->rptr[irow][k] = *ipt;
        ipt++;
      }

      A->rptr[irow][idx - 1] = a;
      A->rptr[irow][idx]     = b;
      A->rptr[irow][idx + 1] = c;
      A->rptr[irow][idx + 2] = d;
    }

    iarc++;
  }

  // Find least-squares fit of all control points but the first and last
  MatrixPseudoInverse(A, Ap);
  MatrixMultiply(Ap, y, x);

  // Copy fitted control points and append the first and last point
  mControlPoints.resize(mNumControl * 3);

  icpt = mControlPoints.begin();
  ipt  = InputPoints.begin();

  for (ipt = InputPoints.begin(); ipt < InputPoints.begin() + 3; ipt++) {
    *icpt = *ipt;
    *icpt++;
  }

  for (int irow = 1; irow < mNumControl - 1; irow++) {
    for (int k = 1; k < 4; k++) {
      *icpt = static_cast<int>(round(x->rptr[irow][k]));
      icpt++;
    }
  }

  for (ipt = InputPoints.end() - 3; ipt < InputPoints.end(); ipt++) {
    *icpt = *ipt;
    *icpt++;
  }
}

//
// Check if a point is in mask
//
bool Spline::IsInMask(std::vector<int>::const_iterator Point) {
  // Check that point is inside mask and has not already been traversed
  return (Point[0] > -1) && (Point[0] < mMask->width) && (Point[1] > -1) &&
         (Point[1] < mMask->height) && (Point[2] > -1) &&
         (Point[2] < mMask->depth) &&
         (MRIgetVoxVal(mMask, Point[0], Point[1], Point[2], 0) > 0) &&
         (MRIgetVoxVal(mVolume, Point[0], Point[1], Point[2], 0) == 0);
}
