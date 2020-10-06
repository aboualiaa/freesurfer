//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(corio_unit, free_cor) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(corio_unit, alloc_cor) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(corio_unit, ld_cor) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(corio_unit, sv_cor) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(corio_unit, setcorval) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(corio_unit, getcorval) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(corio_unit, cordir_iswritable) { // NOLINT
  EXPECT_EQ(1, 0);
}
auto main(int /*argc*/, char * * /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
