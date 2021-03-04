//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gsl/gsl>
#include <gtest/gtest.h>

#include "utils/afni.hpp"

#pragma GCC diagnostic ignored "-Wglobal-constructors"

namespace gtest = testing::internal;

char const *const filename =
    "/Users/aboualiaa/Downloads/sample96/anat+orig.BRIK";
char const *const header = "/Users/aboualiaa/Downloads/sample96/anat+orig.HEAD";
char const *const invalid_filename =
    "/Users/aboualiaa/Downloads/sample96/invalid.BRIK";
char const *const output_filename =
    "/Users/aboualiaa/Downloads/sample96/testResult.BRIK";
char const *const output_header =
    "/Users/aboualiaa/Downloads/sample96/testResult.HEAD";
char const *const afni_header_out =
    "AFNI Header Information ============================================\n"
    "DATASET_RANK      : spatial dims 3, sub-bricks 1\n"
    "DATASET_DIMENSIONS: (256, 256, 124)\n"
    "TYPESTRING        : 3DIM_HEAD_ANAT\n"
    "SCENE_DATA        : view type 0, func type 0, verify 0\n"
    "ORIGIN            : (-119.531197, 119.531197, 73.599998)\n"
    "DELTA             : (0.937500, -0.937500, -1.200000)\n"
    "IDCODE_STRING     : MCW_VSHODUEAYTB\n"
    "BYTEORDER_STRING  : MSB_FIRST\n"
    "BRICK_STATS       : min 0.000000\t max 857.000000\n"
    "BRICK_TYPES       : short\n"
    "BRICK_FLOAT_FACS  : 0.000000\n"
    "====================================================================\n";

char const *const afni_read_suffix =
    "BRICK_STATS min = 0.000000 <--> actual min = 0.000000\n"
    "BRICK_STATS max = 857.000000 <--> actual max = 857.000000\n";

TEST(afni_unit, afniRead) { // NOLINT
  gtest::CaptureStderr();
  gtest::CaptureStdout();
  auto *      mri         = fs::utils::afniRead(filename, 1);
  auto *      invalid_mri = fs::utils::afniRead(invalid_filename, 1);
  std::string output{afni_header_out};
  output.append(afni_read_suffix);
  auto wow    = gtest::GetCapturedStderr();
  auto wowstd = gtest::GetCapturedStdout();
  EXPECT_NE(mri, nullptr);
  EXPECT_EQ(wowstd, output);
  EXPECT_EQ(invalid_mri, nullptr);
}

TEST(afni_unit, afniWrite) { // NOLINT
  gtest::CaptureStdout();
  auto *mri = fs::utils::afniRead(filename, 1);
  EXPECT_EQ(fs::utils::afniWrite(mri, output_filename), 0);
  auto out = gtest::GetCapturedStdout();
}

TEST(afni_unit, readAFNIHeader) { // NOLINT
  fs::utils::AF af;
  AFinit(af);
  gsl::owner<FILE *> fp = fopen(header, "re");
  EXPECT_NE(fp, nullptr);
  auto               res = readAFNIHeader(fp, &af);
  std::string        typestring{af.typestring};
  std::string        idcode_string{af.idcode_string};
  std::string        byteorder_string{af.byteorder_string};
  std::vector<float> brick_stats(af.brick_stats, af.brick_stats + af.numstats);
  std::vector<float> brick_types(af.brick_types, af.brick_types + af.numtypes);
  std::vector<float> brick_float_facs(af.brick_float_facs,
                                      af.brick_float_facs + af.numfacs);
  EXPECT_EQ(af.dataset_rank[0], 3);
  EXPECT_EQ(af.dataset_rank[1], 1);
  EXPECT_EQ(af.dataset_dimensions[0], 256);
  EXPECT_EQ(af.dataset_dimensions[1], 256);
  EXPECT_EQ(af.dataset_dimensions[2], 124);
  EXPECT_EQ(typestring, "3DIM_HEAD_ANAT");
  EXPECT_EQ(af.scene_data[0], 0);
  EXPECT_EQ(af.scene_data[1], 0);
  EXPECT_EQ(af.scene_data[2], 0);
  EXPECT_EQ(af.orient_specific[0], 3);
  EXPECT_EQ(af.orient_specific[1], 5);
  EXPECT_EQ(af.orient_specific[2], 1);
  EXPECT_FLOAT_EQ(af.origin[0], -119.5312);
  EXPECT_FLOAT_EQ(af.origin[1], 119.5312);
  EXPECT_FLOAT_EQ(af.origin[2], 73.6);
  EXPECT_FLOAT_EQ(af.delta[0], 0.9375);
  EXPECT_FLOAT_EQ(af.delta[1], -0.9375);
  EXPECT_FLOAT_EQ(af.delta[2], -1.2);
  EXPECT_EQ(idcode_string, "MCW_VSHODUEAYTB");
  EXPECT_EQ(af.numchars, 0);
  EXPECT_EQ(byteorder_string, "MSB_FIRST");
  EXPECT_EQ(brick_stats[0], 0);
  EXPECT_EQ(brick_stats[1], 857);
  EXPECT_EQ(af.numstats, 2);
  EXPECT_EQ(af.numstats, brick_stats.size());
  EXPECT_EQ(brick_types[0], 1);
  EXPECT_EQ(af.numtypes, 1);
  EXPECT_EQ(af.numtypes, brick_types.size());
  EXPECT_EQ(brick_float_facs[0], 0);
  EXPECT_EQ(af.numfacs, 1);
  EXPECT_EQ(af.numfacs, brick_float_facs.size());
  EXPECT_EQ(res, 1); // readAFNIHeader returns 1 on success
}

TEST(afni_unit, AFinit) { // NOLINT
  fs::utils::AF   af;
  gsl::span<char> wow = af.typestring;
  AFinit(af);
  EXPECT_EQ(strcmp(wow.data(), ""), 0);
  EXPECT_EQ(af.dataset_rank[0], 0);
  EXPECT_EQ(af.dataset_rank[1], 0);
  EXPECT_EQ(af.dataset_dimensions[0], 0);
  EXPECT_EQ(af.dataset_dimensions[1], 0);
  EXPECT_EQ(af.dataset_dimensions[2], 0);
  EXPECT_EQ(af.scene_data[0], 0);
  EXPECT_EQ(af.scene_data[1], 0);
  EXPECT_EQ(af.scene_data[2], 0);
  EXPECT_EQ(af.orient_specific[0], 0);
  EXPECT_EQ(af.orient_specific[1], 0);
  EXPECT_EQ(af.orient_specific[2], 0);
  EXPECT_EQ(af.origin[0], 0.);
  EXPECT_EQ(af.origin[1], 0.);
  EXPECT_EQ(af.origin[2], 0.);
  EXPECT_EQ(af.delta[0], 0.);
  EXPECT_EQ(af.delta[1], 0.);
  EXPECT_EQ(af.delta[2], 0.);
  EXPECT_EQ(af.numchars, 0);
  EXPECT_EQ(af.idcode_string, nullptr);
  EXPECT_EQ(af.numstats, 0);
  EXPECT_EQ(af.brick_stats, nullptr);
  EXPECT_EQ(af.numtypes, 0);
  EXPECT_EQ(af.brick_types, nullptr);
  EXPECT_EQ(af.numfacs, 0);
  EXPECT_EQ(af.brick_float_facs, nullptr);
}

TEST(afni_unit, AFclean) { // NOLINT
  fs::utils::AF af;
  AFinit(af);
  gsl::owner<FILE *> fp = fopen(header, "re");
  EXPECT_NE(fp, nullptr);
  [[maybe_unused]] auto res = readAFNIHeader(fp, &af);
  AFclean(&af);
  EXPECT_EQ(af.idcode_string, nullptr);
  EXPECT_EQ(af.brick_stats, nullptr);
  EXPECT_EQ(af.brick_types, nullptr);
  EXPECT_EQ(af.brick_float_facs, nullptr);
}

TEST(afni_unit, printAFNIHeader) { // NOLINT
  fs::utils::AF         af;
  gsl::owner<FILE *>    fp  = fopen(header, "re");
  [[maybe_unused]] auto res = readAFNIHeader(fp, &af);
  gtest::CaptureStdout();
  printAFNIHeader(af);
  auto out = gtest::GetCapturedStdout();
  EXPECT_EQ(afni_header_out, out);
}

auto main(int /*argc*/, char ** /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
