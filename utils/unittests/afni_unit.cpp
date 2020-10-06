//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(afni_unit, afniRead) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(afni_unit, afniWrite) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(afni_unit, readAFNIHeader) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(afni_unit, AFinit) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(afni_unit, AFclean) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(afni_unit, printAFNIHeader) { // NOLINT
  EXPECT_EQ(1, 0);
}

auto main(int /*argc*/, char * * /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
