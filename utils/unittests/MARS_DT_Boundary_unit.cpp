//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(mars_dt_boundary_unit, Boundary) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(mars_dt_boundary_unit, array) { // NOLINT

  EXPECT_EQ(1, 0);
}
auto main(int /*argc*/, char * * /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
