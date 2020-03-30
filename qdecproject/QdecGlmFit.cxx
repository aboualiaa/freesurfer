/**
 * @brief Wrapper for mri_glmfit.
 *
 * Run mri_glmfit, given its input data, and puts the output in the specified
 * working directory.
 */
/*
 * Original Authors: Nick Schmansky and Kevin Teich
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

#include <cstring>
#include <fstream>
#include <sstream>

#include "QdecGlmFit.h"

// Constructors/Destructors
//

QdecGlmFit::QdecGlmFit() { this->mGlmFitResults = nullptr; }

QdecGlmFit::~QdecGlmFit() {
  if (this->mGlmFitResults)
    delete this->mGlmFitResults;
}

//
// Methods
//

/**
 *
 * Run mri_glmfit, using the design parameters specified in the given
 * QdecGlmDesign object.  Results are written to disk, and the
 * GetGlmFitResults method is called to get pointers to these results.
 *
 * @return int
 * @param  iGlmDesign
 */
int QdecGlmFit::Run(QdecGlmDesign *iGlmDesign) {
  if (!iGlmDesign->IsValid()) {
    fprintf(stderr, "\nERROR: QdecGlmFit::Run: QdecGlmDesign is invalid!\n");
    return -1;
  }

  if (iGlmDesign->GetProgressUpdateGUI()) {
    iGlmDesign->GetProgressUpdateGUI()->BeginActionWithProgress(
        "Running GLM...");
  }

  // check if the ?h.aparc.label file exists, if so, use it to mask-out
  // non-cortical medial wall region. if not, check for existence of
  // ?h.cortex.label
  std::stringstream ssAparcLabelFile;
  ssAparcLabelFile << iGlmDesign->GetSubjectsDir() << "/"
                   << iGlmDesign->GetAverageSubject() << "/label/"
                   << iGlmDesign->GetHemi() << ".aparc.label";
  std::ifstream     fInput(ssAparcLabelFile.str().c_str(), std::ios::in);
  std::stringstream ssCortexLabel;
  if (fInput && !fInput.bad()) {
    ssCortexLabel << " --label " << ssAparcLabelFile.str();
  } else {
    std::stringstream ssCortexLabelFile;
    ssCortexLabelFile << iGlmDesign->GetSubjectsDir() << "/"
                      << iGlmDesign->GetAverageSubject() << "/label/"
                      << iGlmDesign->GetHemi() << ".cortex.label";
    std::ifstream     fInput(ssCortexLabelFile.str().c_str(), std::ios::in);
    std::stringstream ssCortexLabel;
    if (fInput && !fInput.bad()) {
      ssCortexLabel << " --label " << ssCortexLabelFile.str();
    } else {
      ssCortexLabel << " --no-mask ";
    }
  }

  // We need to build a command line.
  std::stringstream ssCommand;
  ssCommand << "mri_glmfit --y " << iGlmDesign->GetYdataFileName() << " --fsgd "
            << iGlmDesign->GetFsgdFileName() << " "
            << iGlmDesign->GetDesignMatrixType() << " --glmdir "
            << iGlmDesign->GetWorkingDir() << " --surf "
            << iGlmDesign->GetAverageSubject() << " " << iGlmDesign->GetHemi()
            << ssCortexLabel.str();

  // Append the contrast option to the command for each contrast we
  // have.
  std::vector<std::string> contrastFileNames =
      iGlmDesign->GetContrastFileNames();
  for (unsigned int i = 0; i < iGlmDesign->GetContrastFileNames().size(); i++) {
    ssCommand << " --C " << contrastFileNames[i];
  }

  // Now run the command.
  if (iGlmDesign->GetProgressUpdateGUI()) {
    iGlmDesign->GetProgressUpdateGUI()->UpdateProgressMessage("Running GLM...");
    iGlmDesign->GetProgressUpdateGUI()->UpdateProgressPercent(70);
  }
  char *sCommand = strdup(ssCommand.str().c_str());
  fflush(stdout);
  fflush(stderr);
  int rRun = system(sCommand);
  if (-1 == rRun)
    throw std::runtime_error("system call failed: " + ssCommand.str());
  if (rRun > 0)
    throw std::runtime_error("command failed: " + ssCommand.str());
  free(sCommand);

  //
  // Concatenate the output.
  //
  // Now we have files in our working directory for each contrast we
  // have. We want to concatenate the sig.mgh file in those
  // directories into a final concatenated output file.

  // First check if the output files exist. Add the filenames to a vector.
  if (iGlmDesign->GetProgressUpdateGUI()) {
    iGlmDesign->GetProgressUpdateGUI()->UpdateProgressMessage(
        "Finding output...");
    iGlmDesign->GetProgressUpdateGUI()->UpdateProgressPercent(80);
  }
  std::vector<std::string> contrastNames = iGlmDesign->GetContrastNames();
  std::vector<std::string> lfnSigFiles;
  for (unsigned int i = 0; i < iGlmDesign->GetContrastNames().size(); i++) {
    std::string sigFile = iGlmDesign->GetWorkingDir();
    sigFile += "/";
    sigFile += contrastNames[i];
    sigFile += "/sig.mgh";

    // Check if it exists and is readable.
    std::ifstream fInput(sigFile.c_str(), std::ios::in);
    if (!fInput || fInput.bad())
      throw std::runtime_error(std::string("Couldn't open file ") + sigFile);

    lfnSigFiles.push_back(sigFile);
  }

  // Go through and concatenate copy all the volumes.
  if (iGlmDesign->GetProgressUpdateGUI()) {
    iGlmDesign->GetProgressUpdateGUI()->UpdateProgressMessage(
        "Concatenating output scalars...");
    iGlmDesign->GetProgressUpdateGUI()->UpdateProgressPercent(90);
  }

  // form output filename
  std::string fnContrastsOutput;
  fnContrastsOutput = iGlmDesign->GetWorkingDir();
  fnContrastsOutput += "/contrasts.sig.mgh";

  std::stringstream ssCommand2;
  ssCommand2 << "mri_concat ";
  // subject inputs...
  for (unsigned int i = 0; i < lfnSigFiles.size(); i++) {
    ssCommand2 << lfnSigFiles[i] << " ";
  }
  // and the output filename...
  ssCommand2 << "--o " << fnContrastsOutput;

  // Now run the command.
  sCommand = strdup(ssCommand2.str().c_str());
  rRun     = system(sCommand);
  if (-1 == rRun)
    throw std::runtime_error("system call failed: " + ssCommand2.str());
  if (rRun > 0)
    throw std::runtime_error("command failed: " + ssCommand2.str());
  free(sCommand);

  std::string fnResidualErrorStdDevFile = iGlmDesign->GetWorkingDir();
  fnResidualErrorStdDevFile += "/rstd.mgh";
  std::string fnRegressionCoefficientsFile = iGlmDesign->GetWorkingDir();
  fnRegressionCoefficientsFile += "/beta.mgh";
  std::string fnFsgdFile = iGlmDesign->GetWorkingDir();
  fnFsgdFile += "/y.fsgd";

  // Now write the result information to a GlmFitResults object
  QdecGlmFitResults *glmFitResults = new QdecGlmFitResults(
      iGlmDesign, lfnSigFiles, fnContrastsOutput, fnResidualErrorStdDevFile,
      fnRegressionCoefficientsFile, fnFsgdFile);
  assert(glmFitResults);
  this->mGlmFitResults = glmFitResults;

  if (iGlmDesign->GetProgressUpdateGUI()) {
    iGlmDesign->GetProgressUpdateGUI()->EndActionWithProgress();
  }

  return 0;
}

int QdecGlmFit::CreateResultsFromCachedData(QdecGlmDesign *iGlmDesign) {

  // Build contrast filenames from our contrast names.
  std::vector<std::string> lContrastNames = iGlmDesign->GetContrastNames();
  std::vector<std::string> lfnSigFiles;
  std::vector<std::string>::iterator tContrastName;
  for (tContrastName = lContrastNames.begin();
       tContrastName != lContrastNames.end(); ++tContrastName) {

    // Build the name.
    std::string fnSigFile = iGlmDesign->GetWorkingDir();
    fnSigFile += "/";
    fnSigFile += *tContrastName;
    fnSigFile += "/sig.mgh";

    // Check if it exists and is readable.
    std::ifstream fInput(fnSigFile.c_str(), std::ios::in);
    if (!fInput || fInput.bad())
      throw std::runtime_error(std::string("Couldn't open file ") + fnSigFile);

    lfnSigFiles.push_back(fnSigFile);
  }

  // Make our other filenames.
  std::string fnContrastsOutput;
  fnContrastsOutput = iGlmDesign->GetWorkingDir();
  fnContrastsOutput += "/contrasts.sig.mgh";

  std::string fnResidualErrorStdDevFile = iGlmDesign->GetWorkingDir();
  fnResidualErrorStdDevFile += "/rstd.mgh";

  std::string fnRegressionCoefficientsFile = iGlmDesign->GetWorkingDir();
  fnRegressionCoefficientsFile += "/beta.mgh";

  std::string fnFsgdFile = iGlmDesign->GetWorkingDir();
  fnFsgdFile += "/y.fsgd";

  // Now write the result information to a GlmFitResults object
  QdecGlmFitResults *glmFitResults = new QdecGlmFitResults(
      iGlmDesign, lfnSigFiles, fnContrastsOutput, fnResidualErrorStdDevFile,
      fnRegressionCoefficientsFile, fnFsgdFile);
  assert(glmFitResults);

  delete this->mGlmFitResults;
  this->mGlmFitResults = glmFitResults;

  return 0;
}

/**
 * @return QdecGlmFitResults
 */
QdecGlmFitResults *QdecGlmFit::GetResults() { return this->mGlmFitResults; }
