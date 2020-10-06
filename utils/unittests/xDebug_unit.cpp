//
// Created by Ahmed Abou-Aliaa on 05.10.20.
//

#include <gtest/gtest.h>

TEST(xDebug_unit, xDbg_Init) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_ShutDown) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_PrintStatus) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_RegisterSegfaultHandler) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_PushStack) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_PopStack) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_GetCurrentFunction) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_PrintStack) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_SegfaultHandler) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_Segfault) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_Printf) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_SetStackDesc) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_SetCurrentNote) { // NOLINT

  EXPECT_EQ(1, 0);
}
TEST(xDebug_unit, xDbg_GetCurrentNote) { // NOLINT

  EXPECT_EQ(1, 0);
}
auto main(int /*argc*/, char * * /*argv*/) -> int {

  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
