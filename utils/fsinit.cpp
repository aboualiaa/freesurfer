#include <clocale>
#include <cstdlib>
#include "error.h"

#ifdef HAVE_OPENMP
#include "romp_support.h"
#endif

static void setLocale() {
  if (getenv("FS_DISABLE_LANG") != nullptr)
    return;
  if (setlocale(LC_NUMERIC, "en_US"))
    return;
  if (setlocale(LC_NUMERIC, "en_US.utf8"))
    return;
  ErrorPrintf(NO_ERROR, "Could not set locale");
}

int FSinit() {
  setLocale();
#ifdef HAVE_OPENMP
  if (getenv("OMP_NUM_THREADS") == NULL)
    omp_set_num_threads(1);
#endif
  return (NO_ERROR);
}
