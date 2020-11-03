//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(gifti_unit, mrisReadGIFTIfile) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(gifti_unit, mrisReadGIFTIdanum) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(gifti_unit, MRISreadGiftiAsMRI) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(gifti_unit, MRISwriteGIFTI) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(gifti_unit, mriWriteGifti) { // NOLINT

  EXPECT_EQ(1, 0);
}
auto main(int /*argc*/, char ** /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
