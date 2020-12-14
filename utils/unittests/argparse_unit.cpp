//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

#include "../argparse.cpp"

#pragma GCC diagnostic ignored "-Wglobal-constructors"

namespace gtest = testing::internal;

ArgumentParser       parser;
std::vector<char *>  av{"wow", "yes"};
unsigned char const *wow = reinterpret_cast<const unsigned char *>("help");
int                  ac  = 1;

auto setup() {
  static bool done = false;
  if (!done) {
    done = true;
    parser.addArgument("--atlas_reg", 1, String, true);
    parser.addArgument("--subject_reg", 1, String, true);
    parser.addArgument("--subject_surf", 1, String, true);
    parser.addArgument("--out", 1, String, true);
    // optional
    parser.addArgument("--annot_in", 1, String);
    parser.addArgument("--annot_out", 1, String);
    // help text
    parser.addHelp(wow, 20);
    parser.parse(ac, av.data());

    auto yes = parser.retrieve<std::string>("atlas_reg");
  }
}
TEST(argparse_unit, addArgument) { // NOLINT

  EXPECT_EQ(1, 0);
}

TEST(argparse_unit, addHelp) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(argparse_unit, Argument) { // NOLINT

  EXPECT_EQ(1, 0);
}

TEST(argparse_unit, canonicalName) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(argparse_unit, exists) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(argparse_unit, insertArgument) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(argparse_unit, parse) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(argparse_unit, stob) { // NOLINT
  EXPECT_TRUE(stob("tRuE"));
  EXPECT_FALSE(stob("fAlSe"));
  EXPECT_TRUE(stob("yes"));
  EXPECT_FALSE(stob("NO"));
  EXPECT_TRUE(stob("YES"));
  EXPECT_FALSE(stob("no"));
  EXPECT_FALSE(stob("0"));
  EXPECT_TRUE(stob("1"));
  EXPECT_ANY_THROW(stob("shit")); // NOLINT
}

TEST(argparse_unit, strip) { // NOLINT
  EXPECT_EQ(strip("--three"), "three");
  EXPECT_EQ(strip("-d"), "d");
  EXPECT_EQ(strip("-do"), "do");
}

TEST(argparse_unit, typeName) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(argparse_unit, unstrip) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(argparse_unit, validate) { // NOLINT
  EXPECT_EQ(1, 0);
}

TEST(argparse_unit, verifyOption) {                 // NOLINT
  EXPECT_DEATH(verifyOption(""), "");               // NOLINT
  EXPECT_DEATH(verifyOption("to"), "");             // NOLINT
  EXPECT_DEATH(verifyOption("two"), "");            // NOLINT
  EXPECT_DEATH(verifyOption("-three"), "");         // NOLINT
  EXPECT_DEATH(verifyOption("three"), "");          // NOLINT
  EXPECT_DEATH(verifyOption("--t"), "");            // NOLINT
  EXPECT_NO_FATAL_FAILURE(verifyOption("--three")); // NOLINT
  EXPECT_NO_FATAL_FAILURE(verifyOption("-d"));      // NOLINT
  EXPECT_NO_FATAL_FAILURE(verifyOption("-do"));     // NOLINT
}

auto main(int /*argc*/, char ** /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
