//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#include <gtest/gtest.h>
#include "mri_convert_lib.hpp"
#include <armadillo>

using namespace fs::util::cli;

std::random_device rd;  // obtain a random number from hardware
std::mt19937 eng(rd()); // seed the generator
std::uniform_int_distribution<> distr(0, 40'000); // define the range
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

TEST(test_usage_message, returns_string) {
  ASSERT_STREQ(usage_message().c_str(),
               "\ntype mri_convert_tests -u for usage\n\n");
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
  std::vector<double> fsMatrix{};
  fsMatrix.reserve(fsrand);
  arma::mat armaMatrix(1, fsrand, arma::fill::randu);
  Eigen::MatrixXd eigenMatrix(1, fsrand);
  for (uint64_t i = 0; i < fsrand; ++i) {
    fsMatrix.push_back(armaMatrix(0, i));
    eigenMatrix(0, i) = armaMatrix(0, i);
  }
  auto armaNorm = arma::norm(armaMatrix, "fro");
  auto eigenNorm = eigenMatrix.norm();
  auto fsNorm = fs::math::frobenius_norm(&fsMatrix);
  ASSERT_NEAR(fsNorm, armaNorm, thresh);
  ASSERT_NEAR(fsNorm, eigenNorm, thresh);
  for (auto &x : fsMatrix) {
    auto fsResult = x / fsNorm;
    auto armaResult = x / armaNorm;
    auto eigenResult = x / eigenNorm;
    ASSERT_NEAR(fsResult, armaResult, thresh);
    ASSERT_NEAR(fsResult, eigenResult, thresh);
  }
  auto copy = fsMatrix;
  fs::math::frobenius_normalize(&fsMatrix);
  eigenMatrix.normalize();
  for (int i = 0; i < copy.size(); ++i) {
    auto fsResult = fsMatrix[i];
    auto armaResult = copy[i] / armaNorm;
    auto eigenResult = eigenMatrix(0, i);
    ASSERT_NEAR(fsResult, armaResult, thresh);
    ASSERT_NEAR(fsResult, eigenResult, thresh);
  }
}

int main() {

  testing::InitGoogleTest();

  return RUN_ALL_TESTS();
}
