//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(MRIio_old_unit, lmalloc) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(MRIio_old_unit, lcalloc) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(MRIio_old_unit, buffer_to_image) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(MRIio_old_unit, image_to_buffer) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(MRIio_old_unit, file_name) { // NOLINT

  EXPECT_EQ(1, 0);
}

auto main(int /*argc*/, char * * /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
