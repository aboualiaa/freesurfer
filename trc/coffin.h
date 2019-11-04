/**
 * @file  coffin.h
 * @brief Main container of tractography data and methods
 *
 * Main container of tractography data and methods
 */
/*
 * Original Author: Anastasia Yendiki
 * CVS Revision Info:
 *
 * Copyright © 2011-2016 The General Hospital Corporation (Boston, MA) "MGH"
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

#ifndef COFFIN_H
#define COFFIN_H

#include "vial.h" // Needs to be included first because of CVS libs

#include "bite.h"
#include "mri.h"
#include "pdf.h"
#include "spline.h"
#include "utils.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <limits.h>
#include <math.h>
#include <sstream>
#include <string>
#include <vector>

#include "TrackIO.h"

class Aeon { // One point in time
public:
  Aeon();
  ~Aeon();
  static void SetBaseMask(MRI *BaseMask);
  static void SavePathPriors(std::vector<float> &Priors);
  static void SaveBasePath(std::vector<int> &PathPoints);
  static void SetPathMap(unsigned int PathIndex);
  void ReadData(const char *RootDir, const char *DwiFile,
                const char *GradientFile, const char *BvalueFile,
                const char *MaskFile, const char *BedpostDir,
                int NumTract, float FminPath,
                const char *BaseXfmFile);
  [[nodiscard]] MRI *GetMask() const;
  [[nodiscard]] MRI *GetBaseMask() const;
  [[nodiscard]] float GetDx() const;
  [[nodiscard]] float GetDy() const;
  [[nodiscard]] float GetDz() const;
  [[nodiscard]] unsigned int GetNumSample() const;
  void FreeMask();
  void SetOutputDir(const char *OutDir);
  [[nodiscard]] const string &GetOutputDir() const;
  void ClearPath();
  bool MapPathFromBase(Spline &BaseSpline);
  void FindDuplicatePathPoints(std::vector<bool> &IsDuplicate);
  void RemovePathPoints(std::vector<bool> &DoRemove, unsigned int NewSize = 0);
  void ProposeDiffusionParameters();
  bool ComputePathDataFit();
  int FindErrorSegment(Spline &BaseSpline);
  void UpdatePath();
  void SavePathDataFit(bool IsPathAccepted);
  void SavePath();
  void WriteOutputs();
  [[nodiscard]] unsigned int GetNumFZerosNew() const;
  [[nodiscard]] unsigned int GetNumFZeros() const;
  [[nodiscard]] bool RejectF() const;
  [[nodiscard]] bool AcceptF() const;
  [[nodiscard]] bool RejectTheta() const;
  [[nodiscard]] bool AcceptTheta() const;
  [[nodiscard]] const string &GetLog() const;
  [[nodiscard]] unsigned int GetPathLengthNew() const;
  [[nodiscard]] unsigned int GetPathLength() const;
  [[nodiscard]] double GetLikelihoodOffPathNew() const;
  [[nodiscard]] double GetLikelihoodOffPath() const;
  [[nodiscard]] double GetLikelihoodOnPathNew() const;
  [[nodiscard]] double GetLikelihoodOnPath() const;
  [[nodiscard]] double GetPriorOffPathNew() const;
  [[nodiscard]] double GetPriorOffPath() const;
  [[nodiscard]] double GetPriorOnPathNew() const;
  [[nodiscard]] double GetPriorOnPath() const;
  [[nodiscard]] double GetPosteriorOffPathNew() const;
  [[nodiscard]] double GetPosteriorOffPath() const;
  [[nodiscard]] double GetPosteriorOnPathNew() const;
  [[nodiscard]] double GetPosteriorOnPath() const;
  [[nodiscard]] double GetDataFitNew() const;
  [[nodiscard]] double GetDataFit() const;

private:
  static const unsigned int mDiffStep;
  static int mMaxAPosterioriPath;
  static unsigned int mMaxAPosterioriPath0;
  static std::vector<float> mPriorSamples;
  static std::vector<std::vector<int>> mBasePathPointSamples;
  static MRI *mBaseMask;

  bool mRejectF, mAcceptF, mRejectTheta, mAcceptTheta;
  int mNx, mNy, mNz, mNxy, mNumVox;
  unsigned int mPathLength, mPathLengthNew;
  double mLikelihoodOnPath, mPriorOnPath, mPosteriorOnPath,
      mLikelihoodOnPathNew, mPriorOnPathNew, mPosteriorOnPathNew,
      mLikelihoodOffPath, mPriorOffPath, mPosteriorOffPath,
      mLikelihoodOffPathNew, mPriorOffPathNew, mPosteriorOffPathNew;
  MRI *mMask;
  string mRootDir, mOutDir, mLog;
  std::vector<int> mPathPoints, mPathPointsNew, mErrorPoint;
  std::vector<float> mPathPhi, mPathPhiNew, mPathTheta, mPathThetaNew,
      mDataFitSamples;
  std::vector<std::vector<int>> mPathPointSamples;
  std::vector<Bite> mData;       // [mNumVox]
  std::vector<Bite *> mDataMask; // [mNx x mNy x mNz]
  AffineReg mBaseReg;

  bool IsInMask(std::vector<int>::const_iterator Point);
  void ComputePathLengths(std::vector<int> &PathLengths,
                          std::vector<std::vector<int>> &PathSamples);
  void ComputePathHisto(MRI *HistoVol,
                        std::vector<std::vector<int>> &PathSamples);
  int FindMaxAPosterioriPath(std::vector<std::vector<int>> &PathSamples,
                             std::vector<int> &PathLengths, MRI *PathHisto);
};

class Coffin { // The main container
public:
  Coffin(const char *OutDir, std::vector<char *> InDirList, const char *DwiFile,
         const char *GradientFile, const char *BvalueFile, const char *MaskFile,
         const char *BedpostDir, int NumTract, float FminPath,
         const char *BaseXfmFile, const char *BaseMaskFile,
         const char *InitFile, const char *RoiFile1, const char *RoiFile2,
         const char *RoiMeshFile1, const char *RoiMeshFile2,
         const char *RoiRefFile1, const char *RoiRefFile2,
         const char *XyzPriorFile0, const char *XyzPriorFile1,
         const char *TangPriorFile, const char *CurvPriorFile,
         const char *NeighPriorFile, const char *NeighIdFile,
         int NeighPriorSet, const char *LocalPriorFile,
         const char *LocalIdFile, int LocalPriorSet,
         std::vector<char *> AsegList, const char *AffineXfmFile,
         const char *NonlinXfmFile, int NumBurnIn, int NumSample,
         int KeepSampleNth, int UpdatePropNth,
         const char *PropStdFile, bool Debug = false);
  ~Coffin();
  void SetOutputDir(const char *OutDir);
  void SetPathway(const char *InitFile, const char *RoiFile1,
                  const char *RoiFile2, const char *RoiMeshFile1,
                  const char *RoiMeshFile2, const char *RoiRefFile1,
                  const char *RoiRefFile2, const char *XyzPriorFile0,
                  const char *XyzPriorFile1, const char *TangPriorFile,
                  const char *CurvPriorFile, const char *NeighPriorFile,
                  const char *NeighIdFile, const char *LocalPriorFile,
                  const char *LocalIdFile);
  void SetMcmcParameters(int NumBurnIn, int NumSample,
                         int KeepSampleNth, int UpdatePropNth,
                         const char *PropStdFile);
  bool RunMcmcFull();
  bool RunMcmcSingle();
  void WriteOutputs();

private:
  static const unsigned int mMaxTryMask, mMaxTryWhite, mDiffStep;
  static const float mTangentBinSize, mCurvatureBinSize;
  bool mRejectSpline, mRejectPosterior, mRejectF, mAcceptF, mRejectTheta,
      mAcceptTheta;
  const bool mDebug;
  int mNx, mNy, mNz, mNxy, mNumControl, mNxAtlas, mNyAtlas, mNzAtlas, mNumArc,
      mPriorSetLocal, mPriorSetNear, mNumBurnIn, mNumSample, mKeepSampleNth,
      mUpdatePropNth;
  double mDataPosteriorOnPath, mDataPosteriorOnPathNew, mDataPosteriorOffPath,
      mDataPosteriorOffPathNew, mXyzPriorOnPath, mXyzPriorOnPathNew,
      mXyzPriorOffPath, mXyzPriorOffPathNew, mAnatomicalPrior,
      mAnatomicalPriorNew, mShapePrior, mShapePriorNew, mPosteriorOnPath,
      mPosteriorOnPathNew, mPosteriorOnPathMap, mPosteriorOffPath,
      mPosteriorOffPathNew;
  std::string mOutDir, mInfoGeneral, mInfoPathway, mInfoMcmc;
  std::vector<bool> mRejectControl;            // [mNumControl]
  std::vector<int> mAcceptCount, mRejectCount, // [mNumControl]
      mControlPoints, mControlPointsNew, mPathPoints, mPathPointsNew, mDirLocal,
      mDirNear;
  std::vector<float> mResolution,     // [3]
      mProposalStdInit, mProposalStd, // [mNumControl x 3]
      mControlPointJumps,             // [mNumControl x 3]
      mAcceptSpan, mRejectSpan;       // [mNumControl x 3]
  std::vector<std::vector<int>> mAtlasCoords;
  std::vector<std::vector<unsigned int>> mIdsLocal, mIdsNear;
  std::vector<std::vector<float>> mPriorTangent, // [mNumArc]
      mPriorCurvature,                           // [mNumArc]
      mPriorLocal,                               // [mNumArc x 7]
      mPriorNear;                                // [mNumArc x 6]
  MRI *mMask, *mRoi1, *mRoi2, *mXyzPrior0, *mXyzPrior1;
  std::ofstream mLog;
  Spline mSpline;
  AffineReg mAffineReg;
#ifndef NO_CVS_UP_IN_HERE
  NonlinReg mNonlinReg;
#endif
  std::vector<MRI *> mAseg;
  std::vector<Aeon> mDwi;

  void ReadControlPoints(const char *ControlPointFile);
  void ReadProposalStds(const char *PropStdFile);
  bool InitializeMcmc();
  bool InitializeFixOffMask(int FailSegment);
  bool InitializeFixOffWhite(int FailSegment);
  int FindErrorSegment();
  bool JumpMcmcFull();
  bool JumpMcmcSingle(int ControlIndex);
  bool ProposePathFull();
  bool ProposePathSingle(int ControlIndex);
  void ProposeDiffusionParameters();
  bool AcceptPath(bool UsePriorOnly = false);
  double ComputeXyzPriorOffPath(std::vector<int> &PathAtlasPoints);
  double ComputeXyzPriorOnPath(std::vector<int> &PathAtlasPoints);
  double ComputeAnatomicalPrior(std::vector<int> &PathAtlasPoints);
  double ComputeShapePrior(std::vector<int> &PathAtlasPoints);
  void UpdatePath();
  void UpdateAcceptanceRateFull();
  void UpdateRejectionRateFull();
  void UpdateAcceptRejectRateSingle();
  void UpdateProposalStd();
  void SavePathPosterior(bool IsPathAccepted);
  void SavePath();
  void RemoveDuplicatePathPoints();
  bool IsInMask(std::vector<int>::const_iterator Point);
  bool IsInRoi(std::vector<int>::const_iterator Point, MRI *Roi);
  bool IsZigZag(std::vector<int> &ControlPoints,
                std::vector<int>::const_iterator FirstPerturbedPoint,
                std::vector<int>::const_iterator LastPerturbedPoint);
  void MapPointToAtlas(std::vector<int>::iterator OutPoint,
                       std::vector<int>::const_iterator InPoint);
  void LogObjective();
  void LogObjectiveNaN(unsigned int NumValiData);
};

#endif
