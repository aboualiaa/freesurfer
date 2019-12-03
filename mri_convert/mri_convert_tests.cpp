//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#include <gtest/gtest.h>
#include "mri_convert_lib.hpp"
#include <armadillo>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <Eigen/Dense>

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
  ASSERT_FALSE(usage_message(nullptr));
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
  FILE *stream = fopen("test.txt", "w");
  if (stream) {
    ASSERT_TRUE(usage_message(stream));
    fclose(stream);
  }
  //  system("chmod 000 test.txt");
  stream = fopen("test.txt", "r");
  if (stream) {
    ASSERT_FALSE(usage_message(stream));
    fclose(stream);
  }

  std::string program_name{"boosty_boost"};
  testing::internal::CaptureStdout();
  ASSERT_TRUE(usage_message(std::cout, program_name.c_str()));
  auto output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ(output.c_str(), "\ntype boosty_boost -u for usage\n\n");

  testing::internal::CaptureStdout();
  ASSERT_TRUE(usage_message(std::cout, program_name));
  output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ(output.c_str(), "\ntype boosty_boost -u for usage\n\n");
  testing::internal::CaptureStdout();
  ASSERT_TRUE(usage_message(stdout, program_name.c_str()));
  output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ(output.c_str(), "\ntype boosty_boost -u for usage\n\n");
  testing::internal::CaptureStdout();
  ASSERT_TRUE(usage_message(stdout, program_name));
  output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ(output.c_str(), "\ntype boosty_boost -u for usage\n\n");
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

TEST(test_check_vector_range, test_vectors) {
  std::vector<std::string> strvec{20};
  ASSERT_TRUE(fs::util::cli::check_vector_range(strvec, 20));
  ASSERT_TRUE(fs::util::cli::check_vector_range(strvec, 20, 30));
  ASSERT_FALSE(fs::util::cli::check_vector_range(strvec, 19));
  ASSERT_FALSE(fs::util::cli::check_vector_range(strvec, 0));
  ASSERT_FALSE(fs::util::cli::check_vector_range(strvec, 30));
  ASSERT_TRUE(fs::util::cli::check_vector_range(strvec, 15, 25));
  ASSERT_TRUE(fs::util::cli::check_vector_range(strvec, 15, 20));
  ASSERT_TRUE(fs::util::cli::check_vector_range(strvec, 19, 20));
  ASSERT_TRUE(fs::util::cli::check_vector_range(strvec, 20, 21));
  ASSERT_DEATH(fs::util::cli::check_vector_range(strvec, 30, 20), "");
}

TEST(test_create_gdb_file, test_input) {
  char *myargv[4] = {"mri_convert_tests", "--debug", "--hello", "2"};
  gsl::multi_span<char *> args(myargv, 4);
  fs::dbg::create_gdb_file(args);
  std::ifstream file("debug.gdb");
  std::string str((std::istreambuf_iterator<char>(file)),
                  std::istreambuf_iterator<char>());
  ASSERT_STREQ(str.c_str(), "# source this file in gdb to debug\nfile "
                            "mri_convert_tests\nrun --hello 2 \n");
  char *myargv2[4] = {"mri_convert_tests", "-debug", "--hello", "2"};
  gsl::multi_span<char *> args2(myargv2, 4);
  fs::dbg::create_gdb_file(args2);
  std::ifstream file2("debug.gdb");
  std::string str2((std::istreambuf_iterator<char>(file2)),
                   std::istreambuf_iterator<char>());
  ASSERT_STREQ(str2.c_str(), "# source this file in gdb to debug\nfile "
                             "mri_convert_tests\nrun --hello 2 \n");
}

TEST(test_print_parsed_tokens, example_input) {
  char *myargv[4] = {"mri_convert_tests", "--debug", "--hello", "baby"};
  po::options_description mydesc("whazzup");
  std::string temp{};
  mydesc.add_options()                /**/
      ("debug",                       /**/
       "print debug information")     /**/
      ("hello",                       /**/
       po::value<std::string>(&temp), /**/
       "takes a string")              /**/
      ;

  auto parsed_opts = po::command_line_parser(4, myargv)
                         .options(mydesc)
                         .style(fs::util::cli::po_style)
                         .run();

  testing::internal::CaptureStdout();
  fs::util::cli::print_parsed_tokens(parsed_opts);
  auto output = testing::internal::GetCapturedStdout();

  ASSERT_STREQ(output.c_str(), "--debug\n--hello baby\n");
}

int main(int argc, char *argv[]) {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
