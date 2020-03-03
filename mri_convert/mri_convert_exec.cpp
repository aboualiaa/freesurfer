/**
 * @file  mri_convert.c
 * @brief performs all kinds of conversion and reformatting of MRI volume files
 *
 */
/*
 * Original Author: Bruce Fischl (Apr 16, 1997)
 * CVS Revision Info:
 *    $Author: greve $
 *    $Date: 2017/02/16 19:15:42 $
 *    $Revision: 1.227 $
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

#include "mri_convert.hpp"

#include "DICOMRead.h"
#include "cma.h"
#include "diag.h"
#include "fio.h"
#include "fmriutils.h"
#include "gcamorph.h"
#include "mri2.h"
#include "mri2020.hpp"
#include "mri_conform.h"
#include "mri_identify.h"
#include "mriio.hpp"

#include <vector>

#include <boost/algorithm/string.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

auto mri_convert(std::vector<char const *> args) -> int;

auto main(int argc, char const *argv[]) -> int {
  std::vector<char const *> args(argv, argv + argc);
  return mri_convert(args);
}
