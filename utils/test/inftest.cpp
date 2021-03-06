/**
 * @brief infinity tests
 *
 */
/*
 * Original Author: Y. Tosa
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

#include <iostream>
#include <math.h>

#include "mghendian.h"
#include "utils.h"
const char *Progname = "inftest";

int main(int argc, char *argv[]) {
  int   fails = 0;
  float v     = 0.f;
  int   res   = devIsnan(0.f / v);
  if (res != 1) {
    std::cerr << res << " should be nan when 0.f/0.f" << std::endl;
    fails++;
  }
  res = devIsnan(1.f);
  if (res != 0) {
    std::cerr << res << " should not be nan when 0.f/0.f" << std::endl;
    fails++;
  }
  res = devIsinf(1.f);
  if (res != 0) {
    std::cerr << res << " should not be inf when 1.f" << std::endl;
    fails++;
  }
#ifndef SunOS
  res = devIsinf(HUGE_VALF);
  if (res != 1) {
    std::cerr << res << " should be +inf for HUGE_VALF" << std::endl;
    fails++;
  }
  res = devIsinf(-HUGE_VALF);
  if (res != -1) {
    std::cerr << res << " should be -inf for -HUGE_VALF" << std::endl;
    fails++;
  }
#endif
  if (fails)
    return 77; // don't indicate out-right failure
}
