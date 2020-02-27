//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#include "mri_convert_lib.hpp"
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <eigen3/Eigen/Dense>
#include <gtest/gtest.h>
#include <random>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wglobal-constructors"

namespace cli   = fs::util::cli;
namespace po    = boost::program_options;
namespace gtest = testing::internal;

auto get_random_number() -> uint64_t;

/// Generates a random number that persists throughout the program lifetime
/// \return a random uint64_t between 0 and 40 000
auto get_random_number() -> uint64_t {
  static std::random_device &rd =
      *new std::random_device;     // obtain a random number from hardware
  static std::mt19937   eng(rd()); // seed the generator
  static const uint64_t upper_limit = 40'000;
  static std::uniform_int_distribution<uint64_t> distr(
      0, upper_limit); // define the range
  static auto fsrand = distr(eng);
  return fsrand;
}

TEST(test_usage_message, nullptr_test) { // NOLINT
  FILE *      stream{};
  std::string program_name{"boosty_boost"};
  ASSERT_FALSE(cli::usage_message(nullptr, nullptr));
  ASSERT_FALSE(cli::usage_message(stream, nullptr));
  ASSERT_FALSE(cli::usage_message(nullptr, nullptr));
  ASSERT_FALSE(cli::usage_message(nullptr));
  ASSERT_FALSE(cli::usage_message(nullptr, program_name));
  ASSERT_FALSE(cli::usage_message(nullptr, program_name.c_str()));
  ASSERT_FALSE(cli::usage_message(stdout, nullptr));
  ASSERT_FALSE(cli::usage_message(std::cout, nullptr));
  ASSERT_FALSE(cli::usage_message(stdin, program_name));
}

TEST(test_usage_message, returns_string) { // NOLINT
  ASSERT_STREQ(cli::usage_message().c_str(),
               "\ntype mri_convert_unit -u for usage\n\n");
}

TEST(test_usage_message, valid_input_test) { // NOLINT
  FILE *stream = fopen("test.txt", "we");
  if (stream != nullptr) {
    ASSERT_TRUE(cli::usage_message(stream));
    fclose(stream);
  }
  //  system("chmod 000 test.txt");
  stream = fopen("test.txt", "re");
  if (stream != nullptr) {
    ASSERT_FALSE(cli::usage_message(stream));
    fclose(stream);
  }

  std::string program_name{"boosty_boost"};
  gtest::CaptureStdout();
  ASSERT_TRUE(cli::usage_message(std::cout, program_name.c_str()));
  auto output = gtest::GetCapturedStdout();
  ASSERT_STREQ(output.c_str(), "\ntype boosty_boost -u for usage\n\n");

  gtest::CaptureStdout();
  ASSERT_TRUE(cli::usage_message(std::cout, program_name));
  output = gtest::GetCapturedStdout();
  ASSERT_STREQ(output.c_str(), "\ntype boosty_boost -u for usage\n\n");
  gtest::CaptureStdout();
  ASSERT_TRUE(cli::usage_message(stdout, program_name.c_str()));
  output = gtest::GetCapturedStdout();
  ASSERT_STREQ(output.c_str(), "\ntype boosty_boost -u for usage\n\n");
  gtest::CaptureStdout();
  ASSERT_TRUE(cli::usage_message(stdout, program_name));
  output = gtest::GetCapturedStdout();
  ASSERT_STREQ(output.c_str(), "\ntype boosty_boost -u for usage\n\n");
}

// TODO(aboualiaa): make this work with armadillo 4 then remove condition
// Info: on trusty armadillo version is 4
// This only compiles with version 9.850.1 on macOS Catalina
#ifndef __linux__

#include <armadillo>

TEST(test_frobenius_norm, vector_test) { // NOLINT
  auto const          thresh = 1e-011;
  std::vector<double> arr{};
  arma::mat           matrix(1, get_random_number(), arma::fill::randn);
  arr.reserve(get_random_number());
  for (uint64_t i = 0; i < get_random_number(); ++i) {
    arr.push_back(matrix(0, i));
  }

  auto armanorm = arma::norm(matrix, "fro");
  auto norm     = fs::math::frobenius_norm(&arr);
  ASSERT_NEAR(norm, armanorm, thresh);
}

TEST(test_frobenius_normalize, vector_version) { // NOLINT
  auto const          thresh = 1e-11;
  std::vector<double> fsMatrix{};
  fsMatrix.reserve(get_random_number());
  arma::mat       armaMatrix(1, get_random_number(), arma::fill::randn);
  Eigen::MatrixXd eigenMatrix(1, get_random_number());
  for (size_t i = 0; i < get_random_number(); ++i) {
    fsMatrix.push_back(armaMatrix(0, i));
    eigenMatrix(0, i) = armaMatrix(0, i);
  }
  auto armaNorm  = arma::norm(armaMatrix, "fro");
  auto eigenNorm = eigenMatrix.norm();
  auto fsNorm    = fs::math::frobenius_norm(&fsMatrix);
  ASSERT_NEAR(fsNorm, armaNorm, thresh);
  ASSERT_NEAR(fsNorm, eigenNorm, thresh);
  for (auto &x : fsMatrix) {
    auto fsResult    = x / fsNorm;
    auto armaResult  = x / armaNorm;
    auto eigenResult = x / eigenNorm;
    ASSERT_NEAR(fsResult, armaResult, thresh);
    ASSERT_NEAR(fsResult, eigenResult, thresh);
  }
  auto copy = fsMatrix;
  fs::math::frobenius_normalize(&fsMatrix);
  eigenMatrix.normalize();
  for (size_t i = 0; i < copy.size(); ++i) {
    auto fsResult    = fsMatrix[i];
    auto armaResult  = copy[i] / armaNorm;
    auto eigenResult = eigenMatrix(0, i);
    ASSERT_NEAR(fsResult, armaResult, thresh);
    ASSERT_NEAR(fsResult, eigenResult, thresh);
  }
}

#endif

TEST(test_check_vector_range, test_vectors) { // NOLINT
  std::vector<std::string> strvec{20};
  ASSERT_TRUE(cli::check_vector_range(strvec, "1", 20));
  ASSERT_TRUE(cli::check_vector_range(strvec, "2", 20, 30));
  ASSERT_ANY_THROW(cli::check_vector_range(strvec, "3", 19)); // NOLINT (goto)
  ASSERT_ANY_THROW(cli::check_vector_range(strvec, "4", 0));  // NOLINT (goto)
  ASSERT_ANY_THROW(cli::check_vector_range(strvec, "5", 30)); // NOLINT (goto)
  ASSERT_TRUE(cli::check_vector_range(strvec, "6", 15, 25));
  ASSERT_TRUE(cli::check_vector_range(strvec, "7", 15, 20));
  ASSERT_TRUE(cli::check_vector_range(strvec, "8", 19, 20));
  ASSERT_TRUE(cli::check_vector_range(strvec, "9", 20, 21));
  ASSERT_ANY_THROW(
      cli::check_vector_range(strvec, "10", 30, 20)); // NOLINT (goto)
}

TEST(test_create_gdb_file, test_input) { // NOLINT
  std::vector<char const *> args{"mri_convert_tests", "--debug", "--hello",
                                 "2"};
  fs::dbg::create_gdb_file(args);
  std::ifstream file("debug.gdb");
  std::string   str((std::istreambuf_iterator<char>(file)),
                  std::istreambuf_iterator<char>());
  ASSERT_STREQ(str.c_str(), "# source this file in gdb to debug\nfile "
                            "mri_convert_tests\nrun --hello 2 \n");
  std::vector<char const *> args2{"mri_convert_tests", "--debug", "--hello",
                                  "2"};
  fs::dbg::create_gdb_file(args2);
  std::ifstream file2("debug.gdb");
  std::string   str2((std::istreambuf_iterator<char>(file2)),
                   std::istreambuf_iterator<char>());
  ASSERT_STREQ(str2.c_str(), "# source this file in gdb to debug\nfile "
                             "mri_convert_tests\nrun --hello 2 \n");
}

TEST(test_print_parsed_tokens, example_input) { // NOLINT
  std::vector<char const *> myargv{"mri_convert_tests", "--debug", "--hello",
                                   "baby"};
  po::options_description   mydesc("whazzup");
  std::string               temp{};
  mydesc.add_options()                /**/
      ("debug",                       /**/
       "print debug information")     /**/
      ("hello",                       /**/
       po::value<std::string>(&temp), /**/
       "takes a string")              /**/
      ;

  auto parsed_opts = po::command_line_parser(4, myargv.data())
                         .options(mydesc)
                         .style(cli::po_style)
                         .run();

  gtest::CaptureStdout();
  cli::print_parsed_tokens(parsed_opts);
  auto output = gtest::GetCapturedStdout();

  ASSERT_STREQ(output.c_str(), "--debug\n--hello baby\n");
}

auto main(int /*argc*/, char * * /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}

#pragma GCC diagnostic pop
