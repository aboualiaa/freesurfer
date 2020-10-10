//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include "AFNI.h"
#include "gtest/gtest.h"

#pragma GCC diagnostic ignored "-Wglobal-constructors"

namespace gtest = testing::internal;

char *filename         = "/Users/aboualiaa/Downloads/sample96/anat+orig.BRIK";
char *header           = "/Users/aboualiaa/Downloads/sample96/anat+orig.HEAD";
char *invalid_filename = "/Users/aboualiaa/Downloads/sample96/invalid.BRIK";
char *output_filename  = "/Users/aboualiaa/Downloads/sample96/testResult.BRIK";
char *afni_header_out =
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

AF af;

TEST(afni_unit, afniRead) { // NOLINT

  gtest::CaptureStderr();
  gtest::CaptureStdout();
  auto *mri         = afniRead(filename, 1);
  auto *invalid_mri = afniRead(invalid_filename, 1);
  auto  wow         = gtest::GetCapturedStderr();
  auto  wowstd      = gtest::GetCapturedStdout();
  EXPECT_NE(mri, nullptr);
  EXPECT_EQ(invalid_mri, nullptr);
}

TEST(afni_unit, afniWrite) { // NOLINT
  auto *mri = afniRead(filename, 1);
  EXPECT_EQ(afniWrite(mri, output_filename), 0);
}

TEST(afni_unit, readAFNIHeader) { // NOLINT
  auto *fp = fopen(header, "r");
  EXPECT_NE(fp, nullptr);
  auto res = readAFNIHeader(fp, &af);
  EXPECT_EQ(res, 1);
}

TEST(afni_unit, AFinit) { // NOLINT
  AFinit(&af);
  EXPECT_EQ(strcmp(af.typestring, ""), 0);
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
  AFclean(&af);
  EXPECT_EQ(af.idcode_string, nullptr);
  EXPECT_EQ(af.brick_stats, nullptr);
  EXPECT_EQ(af.brick_types, nullptr);
  EXPECT_EQ(af.brick_float_facs, nullptr);
}

TEST(afni_unit, printAFNIHeader) { // NOLINT
  auto *fp  = fopen(header, "r");
  auto  res = readAFNIHeader(fp, &af);
  gtest::CaptureStdout();
  printAFNIHeader(&af);
  auto out = gtest::GetCapturedStdout();
  EXPECT_EQ(strcmp(afni_header_out, out.c_str()), 0);
}

auto main(int /*argc*/, char * * /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
