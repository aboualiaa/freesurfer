#define export
#include <vnl/algo/vnl_matrix_inverse.h>
#include <vnl/vnl_matrix.h>

#include "error.h"
#include "macros.h"
#include "version.h"

static void printUsage();
static bool parseCommandLine(int argc, char *argv[]);
static int  parseNextCommand(int argc, char *argv[]);

static char vcid[] =
    "$Id: lta_convert.cpp,v 1.10 2016/08/09 02:11:11 zkaufman Exp $";
const char *Progname = nullptr;
float       myrand(float f);

int main(int argc, char *argv[]) {
  std::cout << vcid << std::endl << std::endl;

  // Default initialization
  int nargs = handleVersionOption(argc, argv, "cpptester");
  if (nargs && argc - nargs == 1) {
    exit(0);
  }
  argc -= nargs;
  Progname = argv[0];
  argc--;
  argv++;
  ErrorInit(NULL, NULL, NULL);

  while (true) { // FIXME: infinite loop
    vnl_matrix<float> M(3000, 3000);
    vnl_matrix<float> Q = M.apply(&myrand);
  }
  vnl_matrix<float> N(3, 2);
  // vnl_matrix<float> Q =  M.apply(&myrand);
  // Q.print(cout);
  // vnl_matrix<float> Qinv = vnl_inverse(Q);
  // vnl_matrix<float> Qinv = vnl_matrix_inverse<float>(Q);
  // Qinv.print(cout);
  // N.fill(3);
  // M.fill_diagonal(2);
  // vnl_matrix<float> S = M*N;
  // N.print(cout);
  // printf("is_zero %d\n",M.is_zero());
  // printf("is_ident %d\n",M.is_identity());

  // Parse command line
  if (!parseCommandLine(argc, argv)) {
    // printUsage();
    exit(1);
  }
}

// TODO: implement or delete
static void printUsage() { printf("usage\n"); }

// TODO: use boost
static int parseNextCommand(int argc, char *argv[]) {
  int   nargs = 0;
  char *option;

  option = argv[0] + 1; // remove '-'
  if (option[0] == '-') {
    option = option + 1; // remove second '-'
  }
  fflush(stdout);

  return (nargs);
}

static bool parseCommandLine(int argc, char *argv[]) {
  int nargs;
  int inputargs = argc;
  for (; argc > 0 && ISOPTION(*argv[0]); argc--, argv++) {
    nargs = parseNextCommand(argc, argv);
    argc -= nargs;
    argv += nargs;
  }

  if (inputargs == 0) {
    printUsage();
    exit(1);
  }

  return true;
}

float myrand(float f) { return ((float)drand48()); }
