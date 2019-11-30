//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#include <gtest/gtest.h>
#include "mri_convert_lib.hpp"

using namespace fs::utils::cli;

TEST(test_usage_message, nullptr_test) {
  FILE *stream{};
  std::string program_name{"boosty_boost"};
  ASSERT_FALSE(usage_message(nullptr, nullptr));
  ASSERT_FALSE(usage_message(stream, nullptr));
  ASSERT_FALSE(usage_message(nullptr, nullptr));
  ASSERT_FALSE(usage_message(nullptr, program_name));
  ASSERT_FALSE(usage_message(nullptr, program_name.c_str()));
  ASSERT_FALSE(usage_message(stdout, nullptr));
  ASSERT_FALSE(usage_message(std::cout, nullptr));
  ASSERT_FALSE(usage_message(stdin, program_name));
}

TEST(test_usage_message, valid_input_test) {
  FILE *stream{};
  std::string program_name{"boosty_boost"};
  ASSERT_TRUE(usage_message(std::cout, program_name.c_str()));
  ASSERT_TRUE(usage_message(std::cout, program_name));
  ASSERT_TRUE(usage_message(stdout, program_name.c_str()));
  ASSERT_TRUE(usage_message(stdout, program_name));
}

int main() {

  testing::InitGoogleTest();

  return RUN_ALL_TESTS();
}
