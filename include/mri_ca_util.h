/*
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

#ifndef util_h
#define util_h

#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <set>
#include <stdio.h>
#include <string>
#include <vector>

#include "mri.h"

typedef std::vector<double>           TypeVectorDouble;
typedef std::vector<TypeVectorDouble> TypeMatrixDouble;

typedef std::vector<float>           TypeVectorFloat;
typedef std::vector<TypeVectorFloat> TypeMatrixFloat;

typedef std::vector<int>           TypeVectorInt;
typedef std::vector<TypeVectorInt> TypeMatrixInt;

typedef std::vector<std::string>      TypeVectorString;
typedef std::vector<TypeVectorString> TypeMatrixString;

void printType(TypeVectorDouble vectorDouble, ostream &ofs = cout) {
  char strMsg[500];

  int nMaxIndex = vectorDouble.size() - 1;
  int nWhichIndex;

  if (nMaxIndex == -1) {
    ofs << "\n";
  }

  for (nWhichIndex = 0; nWhichIndex <= nMaxIndex; nWhichIndex++) {
    if (nWhichIndex == nMaxIndex) {
      sprintf(strMsg, "%8.3f", vectorDouble[nWhichIndex]);
      ofs << strMsg << std::endl;
    } else {
      sprintf(strMsg, "%8.3f, ", vectorDouble[nWhichIndex]);
      ofs << strMsg;
    }
  }
}

void printType(TypeMatrixDouble matrixDouble, ostream &ofs = cout) {
  char strMsg[500];

  int nMaxRowIndex = matrixDouble.size() - 1;

  if (nMaxRowIndex == -1) {
    ofs << "\n";
  }
  for (int nWhichRowIndex = 0; nWhichRowIndex <= nMaxRowIndex;
       nWhichRowIndex++) {
    int nMaxColIndex = matrixDouble[nWhichRowIndex].size() - 1;
    int nWhichColIndex;
    for (nWhichColIndex = 0; nWhichColIndex <= nMaxColIndex; nWhichColIndex++) {
      if (nWhichColIndex == nMaxColIndex) {
        sprintf(strMsg, "%8.3f", matrixDouble[nWhichRowIndex][nWhichColIndex]);
        ofs << strMsg << std::endl;
      } else {
        sprintf(strMsg, "%8.3f, ",
                matrixDouble[nWhichRowIndex][nWhichColIndex]);
        ofs << strMsg;
      }
    };
  }
}

void printType(TypeVectorFloat vectorFloat, ostream &ofs = cout) {
  char strMsg[500];

  int nMaxIndex = vectorFloat.size() - 1;
  int nWhichIndex;

  if (nMaxIndex == -1) {
    ofs << "\n";
  }

  for (nWhichIndex = 0; nWhichIndex <= nMaxIndex; nWhichIndex++) {
    if (nWhichIndex == nMaxIndex) {
      sprintf(strMsg, "%8.3f", vectorFloat[nWhichIndex]);
      ofs << strMsg << std::endl;
    } else {
      sprintf(strMsg, "%8.3f, ", vectorFloat[nWhichIndex]);
      ofs << strMsg;
    }
  }
}

void printType(TypeMatrixFloat matrixFloat, ostream &ofs = cout) {
  char strMsg[500];

  int nMaxRowIndex = matrixFloat.size() - 1;

  if (nMaxRowIndex == -1) {
    ofs << "\n";
  }
  for (int nWhichRowIndex = 0; nWhichRowIndex <= nMaxRowIndex;
       nWhichRowIndex++) {
    int nMaxColIndex = matrixFloat[nWhichRowIndex].size() - 1;
    int nWhichColIndex;
    for (nWhichColIndex = 0; nWhichColIndex <= nMaxColIndex; nWhichColIndex++) {
      if (nWhichColIndex == nMaxColIndex) {
        sprintf(strMsg, "%8.3f", matrixFloat[nWhichRowIndex][nWhichColIndex]);
        ofs << strMsg << std::endl;
      } else {
        sprintf(strMsg, "%8.3f, ", matrixFloat[nWhichRowIndex][nWhichColIndex]);
        ofs << strMsg;
      }
    };
  }
}

void printType(TypeVectorInt vectorInt, ostream &ofs = cout) {
  char strMsg[500];

  int nMaxIndex = vectorInt.size() - 1;
  if (nMaxIndex == -1) {
    ofs << "\n";
  }

  int nWhichIndex;
  for (nWhichIndex = 0; nWhichIndex <= nMaxIndex; nWhichIndex++) {

    if (nWhichIndex == nMaxIndex) {
      sprintf(strMsg, "%8d", vectorInt[nWhichIndex]);
      ofs << strMsg << std::endl;
    } else {
      sprintf(strMsg, "%8d, ", vectorInt[nWhichIndex]);
      ofs << strMsg;
    }
  }
}

void printType(TypeMatrixInt matrixInt, ostream &ofs = cout) {
  char strMsg[500];

  int nMaxRowIndex = matrixInt.size() - 1;
  if (nMaxRowIndex == -1) {
    ofs << "\n";
  }

  for (int nWhichRowIndex = 0; nWhichRowIndex <= nMaxRowIndex;
       nWhichRowIndex++) {
    int nMaxColIndex = matrixInt[nWhichRowIndex].size() - 1;
    if (nMaxColIndex == -1) {
      ofs << "\n";
    }
    int nWhichColIndex;
    ofs << "\n";
    for (nWhichColIndex = 0; nWhichColIndex <= nMaxColIndex; nWhichColIndex++) {

      if (nWhichColIndex == nMaxColIndex) {
        sprintf(strMsg, "%8d", matrixInt[nWhichRowIndex][nWhichColIndex]);
        ofs << strMsg << std::endl;
      } else {
        sprintf(strMsg, "%8d, ", matrixInt[nWhichRowIndex][nWhichColIndex]);
        ofs << strMsg;
      }
    };
  }
}

void printType(std::string strString, ostream &ofs = cout) {
  char strMsg[500];

  sprintf(strMsg, "%s", strString.c_str());
  ofs << strMsg << std::endl;
}

void printType(TypeVectorString vectorString, ostream &ofs = cout) {
  char strMsg[500];

  int nMaxIndex = vectorString.size() - 1;
  if (nMaxIndex == -1) {
    ofs << "\n";
  }

  int nWhichIndex;
  for (nWhichIndex = 0; nWhichIndex <= nMaxIndex; nWhichIndex++) {
    if (nWhichIndex == nMaxIndex) {
      sprintf(strMsg, "%s", vectorString[nWhichIndex].c_str());
      ofs << strMsg << std::endl;
    } else {
      sprintf(strMsg, "%s, ", vectorString[nWhichIndex].c_str());
      ofs << strMsg;
    }
  }
}

void printType(TypeMatrixString matrixString, ostream &ofs = cout) {
  char strMsg[500];

  int nMaxRowIndex = matrixString.size() - 1;
  if (nMaxRowIndex == -1) {
    ofs << "\n";
  }

  for (int nWhichRowIndex = 0; nWhichRowIndex <= nMaxRowIndex;
       nWhichRowIndex++) {
    int nMaxColIndex = matrixString[nWhichRowIndex].size() - 1;
    if (nMaxColIndex == -1) {
      ofs << "\n";
    }
    int nWhichColIndex;
    for (nWhichColIndex = 0; nWhichColIndex <= nMaxColIndex; nWhichColIndex++) {

      if (nWhichColIndex == nMaxColIndex) {
        sprintf(strMsg, "%s",
                matrixString[nWhichRowIndex][nWhichColIndex].c_str());
        ofs << strMsg << std::endl;
      } else {
        sprintf(strMsg, "%s, ",
                matrixString[nWhichRowIndex][nWhichColIndex].c_str());
        ofs << strMsg;
      }
    };
  }
}

#endif
