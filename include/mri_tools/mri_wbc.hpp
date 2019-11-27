//
// Created by Ahmed Abou-Aliaa on 04.11.19.
//

#ifndef MRI_TOOLS_MRI_WBC_HPP
#define MRI_TOOLS_MRI_WBC_HPP

#include "absl/strings/str_join.h"
#include "benchmark/benchmark.h"
#include "fmt/printf.h"
#include "gsl/gsl"
#include "gsl/multi_span"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <hpx/hpx_start.hpp>
#include <hpx/include/iostreams.hpp> // hpx headers have to be included before boost headers

#include <boost/program_options/parsers.hpp>

#include "diag.h"
#include "error.h"
#include "fmriutils.h"
#include "fsenv.h"
#include "macros.h"
#include "matfile.h"
#include "mri.h"
#include "mri2.h"
#include "mrimorph.h"
#include "mrisurf.h"
#include "romp_support.h"
#include "timer.h"
#include "version.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <utility>

//#include "banned.h" // always include last, otherwise you'll break the stl :D

namespace po = boost::program_options;
namespace fsys = std::filesystem; // sadly fs is already defined: freesurfer
using podesc = po::options_description;
using povm = po::variables_map;
namespace pocl = boost::program_options::command_line_style;

auto cl_style =
    pocl::allow_short | pocl::short_allow_adjacent | pocl::short_allow_next |
    pocl::allow_long | pocl::long_allow_adjacent | pocl::long_allow_next |
    pocl::allow_sticky | pocl::allow_dash_for_short | pocl::allow_long_disguise;

// TODO(aboualiaa): Find out function and rename accordingly
constexpr float half = 0.5;
constexpr float fifth = 0.2;
constexpr int five = 5;
constexpr int max_rows = 2000;
constexpr float two = 2.0;
constexpr float defaultRes = 3.5;
constexpr float zeroPointNine = 0.9;
constexpr float onePointOne = 1.1;
constexpr int defaultDim = 32;
constexpr int defaultnshorttarg = 10;
constexpr int defaultnlongtarg = 20;
constexpr int defaultnshorttarg1 = 15;
constexpr int defaultnlongtarg1 = 25;
constexpr int defaultdistthresh = 10;
constexpr int expectedErrs = 9;
constexpr float tenToEighth = 1e8;
constexpr float eightPointZero = 8.0;

namespace wbc {

// FIXME: not packed
struct WBCSYNTH {
  double distthresh;
  float volres;
  std::vector<double> wf;
  int nframes;
  int voldim;
  int nshortvol;
  int nlongvol;
  int v0;
  int c0;
  int r0;
  int s0;
  int c2;
  std::array<int, 2> nshorttarg;
  std::array<int, 2> nlongtarg;
  std::array<int, 2> nshort;
  std::array<int, 2> nlong;
  int ForceFail;
};

// FIXME: not packed
struct WBC {
  MRI *frh;                    // 8
  MRI *fvol;                   // 8
  MRI *flh;                    // 8
  MRI *volmask;                // 8
  MRI *lhmask;                 // 8
  MRI *rhmask;                 // 8
  MRI *lhvtxvol;               // 8
  MRI *rhvtxvol;               // 8
  MRI *volcon;                 // 8
  MRI *lhcon;                  // 8
  MRI *rhcon;                  // 8
  MRI *volconS;                // 8
  MRI *lhconS;                 // 8
  MRI *rhconS;                 // 8
  MRI *volconL;                // 8
  MRI *lhconL;                 // 8
  MRI *rhconL;                 // 8
  MRI *volrhomean;             // 8
  MRI *lhrhomean;              // 8
  MRI *rhrhomean;              // 8
  MRI *coordtype;              // 8
  MRI *vertexno;               // 8
  MRI *xyz;                    // 8
  MRI *xyz2;                   // 8
  MRI *vvol;                   // 8
  MRI *f;                      // 8
  MRI *fnorm;                  // 8
  MRI *rhomean;                // 8
  MRI *con;                    // 8
  MRI *conS;                   // 8
  MRI *conL;                   // 8
  LABEL *lhlabel;              // 8
  LABEL *rhlabel;              // 8
  MRIS *lh;                    // 8
  MRIS *rh2;                   // 8
  MRIS *rh;                    // 8
  MRIS *lh2;                   // 8
  MATRIX *M;                   // 8
  WBCSYNTH wbcsynth;           // how many bytes?
  double distthresh;           // 8
  int nvolmask;                // 4
  int nlhmask;                 // 4
  int nrhmask;                 // 4
  int ntot;                    // 4
  int nframes;                 // 4
  bool DoDist;                 // 1
  bool DoMat;                  // 1
  bool DoTest;                 // 1
  std::vector<double> rholist; // 24
};

// FIXME: not packed
struct CMDARGS {
  CMDARGS(int argc, char *argv[]) : raw(argv, argc){}; // NOLINT

public:
  gsl::multi_span<char *> raw;
  std::string fvol;
  std::string volmask;
  std::string flh;
  std::string lhsurface;
  std::string lhsurface2;
  std::string lhlabel;
  std::string lhmask;
  std::string frh;
  std::string rhsurface;
  std::string rhsurface2;
  std::string rhlabel;
  std::string rhmask;
  std::string outdir;
  std::string volcon;
  std::string lhcon;
  std::string rhcon;
  std::string volconS;
  std::string lhconS;
  std::string rhconS;
  std::string volconL;
  std::string lhconL;
  std::string rhconL;
  std::string volrhomean;
  std::string lhrhomean;
  std::string rhrhomean;
  std::string matfile; // read: matrix file
  std::vector<double> rholist;
  double distthresh;
  int nthreads;
  bool DoMat; // do matrix??
  bool ForceFail;
  bool SaveTest;
  bool DoTest;
  bool debug = false;
  bool DoDist;
};

struct ENV {
  std::string vcid = "$Id: mri_wbc.c,v 1.8 2015/07/22 21:51:35 zkaufman Exp $";
};

///
/// \param wbc
/// \return
MRI *WholeBrainCon(WBC *wbc);

///
/// \param wbc
/// \return
int WBCfinish(WBC *wbc);

///
/// \param wbc
/// \return
int WBCprep(WBC *wbc);

/// \brief get the number of frames and checks that all are
/// consistent.
/// \param wbc
/// \return 0 if ok, 1 otherwise
int WBCnframes(WBC *wbc);

///
/// \param wbc
/// \return
int WBCsynth(WBC *wbc);

///
/// \param wbc
/// \return
WBC *WBCtestSynth(WBC *wbc);

///
/// \param wbc
/// \return
int WBCtestCheck(WBC *wbc);

/// \brief Computes the row and col of the ith index in an upper
/// triangular N-x-N matrix ignoring the diagonal and lower triangular
/// components. The application for this involves evaluating pairs
/// of items in a list excluding self-comparisons (diag) and
/// reverse comparisons (lower). See also Index2UpperSubscriptTest().
/// Row, col, and index are all 0-based.
/// \param N
/// \param i
/// \param r
/// \param c
/// \return
int Index2UpperSubscript(int N, int64_t i, int64_t *r, int64_t *c);

/// \brief Test for int Index2UpperSubscript(int N, int64_t i, int *r, int *c)
/// \param seed
/// \return
int Index2UpperSubscriptTest(int seed);

/// \brief does the housekeeping, use this to parse command lines,
///  do sanity and inconsistency checks, read files etc. After that
///  just start directly with your porgram logic
/// \param cmdargs struct to hold values of parsed args
/// \param env
/// \param wbc
/// \return true if all logic is ok, false otherwise
static bool good_cmdline_args(CMDARGS *cmdargs, ENV *env, WBC *wbc);

/// \brief initialize options description and save values in cmdargs
/// \param desc
/// \param cmdargs
static void initArgDesc(podesc *desc, CMDARGS *cmdargs);

///
/// \param desc
/// \param env
inline static void print_usage(podesc const &desc, ENV *env) {
  std::cout << desc << "\n" << env->vcid << std::endl;
}

///
/// \param desc
/// \param env
inline static void print_help(podesc const &desc, ENV *env) {
  print_usage(desc, env);
  // TODO(aboualiaa): add tests and remove
  spdlog::get("stderr")->critical("this program is not yet tested!");
}

///
/// \param cmdargs
/// \param vm
/// \return
inline static bool shouldSave(CMDARGS const *cmdargs,
                              po::variables_map const &vm) {
  return cmdargs->SaveTest || (vm.count("fvol") != 0U) ||
         (vm.count("lh") != 0U) || (vm.count("rh") != 0U);
}
} // namespace wbc

namespace boost::program_options {

///
/// \param vm
/// \param for_what
/// \param required_option
inline void option_dependency(povm const &vm, char const *for_what,
                              char const *required_option) {
  if ((vm.count(for_what) != 0U) && !vm[for_what].defaulted()) {
    if (vm.count(required_option) == 0 || vm[required_option].defaulted()) {
      throw std::logic_error(std::string("Option '") + for_what +
                             "' requires option '" + required_option + "'.");
    }
  }
}

///
/// \param vm
/// \param opt1
/// \param opt2
inline void conflicting_options(povm const &vm, std::string const &opt1,
                                std::string const &opt2) {
  if ((vm.count(opt1) != 0U) && !vm[opt1].defaulted() &&
      (vm.count(opt2) != 0U) && !vm[opt2].defaulted()) {
    throw std::logic_error(std::string("Conflicting options '") + opt1 +
                           "' and '" + opt2 + "'.");
  }
}
} // namespace boost::program_options
#endif // MRI_TOOLS_MRI_WBC_HPP
