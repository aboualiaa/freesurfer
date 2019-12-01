//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#include <gtest/gtest.h>
#include "mri_convert_lib.hpp"
#include <armadillo>

using namespace fs::utils::cli;

std::random_device rd;  // obtain a random number from hardware
std::mt19937 eng(rd()); // seed the generator
std::uniform_int_distribution<> distr(0, 13'000); // define the range
auto fsrand = distr(eng);

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

TEST(test_frobenius_norm, vector_test) {
  auto thresh = 1e-013;
  std::vector<double> arr{};
  arma::mat matrix(1, fsrand, arma::fill::randu);
  arr.reserve(fsrand);
  for (uint64_t i = 0; i < fsrand; ++i) {
    arr.push_back(matrix(0, i));
  }

  auto armanorm = arma::norm(matrix, "fro");
  auto norm = fs::math::frobenius_norm(&arr);
  ASSERT_NEAR(norm, armanorm, thresh);
}

TEST(test_frobenius_normalize, vector_version) {
  auto thresh = 1e-13;
  std::vector<double> mat{};
  mat.reserve(fsrand);
  arma::mat dirs(1, fsrand, arma::fill::randu);
  Eigen::MatrixXd eigenMat(1, fsrand);
  for (uint64_t i = 0; i < fsrand; ++i) {
    mat.push_back(dirs(0, i));
    eigenMat(0, i) = dirs(0, i);
  }
  auto armanorm = arma::norm(dirs, "fro");
  auto norm = fs::math::frobenius_norm(&mat);
  ASSERT_NEAR(norm, armanorm, thresh);
  for (auto &x : mat) {
    auto res = x / norm;
    auto armares = x / armanorm;
    ASSERT_NEAR(res, armares, thresh);
  }
  auto copy = mat;
  fs::math::frobenius_normalize(&mat);
  for (int i = 0; i < copy.size(); ++i) {
    auto res = mat[i];
    auto armares = copy[i] / armanorm;
    ASSERT_NEAR(res, armares, thresh);
  }
}

int main() {

  testing::InitGoogleTest();

  return RUN_ALL_TESTS();
}
