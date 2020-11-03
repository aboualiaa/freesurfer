//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include "autoencoder.h"
#include <gtest/gtest.h>

TEST(autoencoder_unit, SAEalloc) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEfree) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEaddLayer) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEtrainLayer) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEtrainFromVoxlist) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEactivateLastHiddenLayer) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEtrainFromMRI) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEvectorToMRI) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEactivateNetwork) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEcomputeRMS) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEcomputeTotalRMS) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEwrite) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEread) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEinputWeightsToMRI) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAElayerWeightsToMRI) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAElayerWeightsToMRI) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEdump) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, AEdump) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEfillInputVector) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, SAEfindLastLayer) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAEfillInputs) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAEalloc) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAEaddLayer) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAEfillInputse) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAEtrainLayerFromVoxlist) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAEwrite) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAEcreateOutputs) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAEcomputeTotalRMS) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAEcomputeVoxlistRMS) { // NOLINT
  EXPECT_EQ(1, 0);
}
TEST(autoencoder_unit, CSAEread) { // NOLINT
  EXPECT_EQ(1, 0);
}

auto main(int /*argc*/, char ** /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
