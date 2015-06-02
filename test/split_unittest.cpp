
#include <limits.h>
#include "gtest/gtest.h"
#include "split.h"

#define INIT_TEST() \
  Meta_Vector wordVector;

#define RUN_TEST(data) \
  split(data, strlen(data), 0, wordVector);

#define CLEAN_TEST() \
  for(size_t i=0; i<wordVector.size(); i++) \
    { \
        free(wordVector[i].data); \
    } \
    wordVector.clear();

TEST(SplitTest, Blank) {
  INIT_TEST();

  RUN_TEST("");
  EXPECT_EQ(0, (int)wordVector.size());
  CLEAN_TEST();
}

TEST(SplitTest, Space) {
  INIT_TEST();

  RUN_TEST(" ");
  EXPECT_EQ(1, (int)wordVector.size());
  EXPECT_EQ(1, wordVector[0].len);
  EXPECT_EQ(TYPE_SPACE, wordVector[0].type);
  CLEAN_TEST();

  RUN_TEST("  ");
  EXPECT_EQ(1, (int)wordVector.size());
  EXPECT_EQ(2, wordVector[0].len);
  EXPECT_EQ(TYPE_SPACE, wordVector[0].type);
  CLEAN_TEST();

  RUN_TEST("\t");
  EXPECT_EQ(1, (int)wordVector.size());
  EXPECT_EQ(1, wordVector[0].len);
  EXPECT_EQ(TYPE_SPACE, wordVector[0].type);
  CLEAN_TEST();

  RUN_TEST("\t\t");
  EXPECT_EQ(1, (int)wordVector.size());
  EXPECT_EQ(2, wordVector[0].len);
  EXPECT_EQ(TYPE_SPACE, wordVector[0].type);
  CLEAN_TEST();

  RUN_TEST("\t \t");
  EXPECT_EQ(1, (int)wordVector.size());
  EXPECT_EQ(3, wordVector[0].len);
  EXPECT_EQ(TYPE_SPACE, wordVector[0].type);
  CLEAN_TEST();

  RUN_TEST(" \t ");
  EXPECT_EQ(1, (int)wordVector.size());
  EXPECT_EQ(3, wordVector[0].len);
  EXPECT_EQ(TYPE_SPACE, wordVector[0].type);
  CLEAN_TEST();
}

TEST(SplitTest, CRLF) {
  INIT_TEST();

  RUN_TEST(" \r");
  EXPECT_EQ(1, (int)wordVector.size());
  EXPECT_EQ(1, wordVector[0].len);
  EXPECT_EQ(TYPE_SPACE, wordVector[0].type);
  CLEAN_TEST();

  RUN_TEST(" \n");
  EXPECT_EQ(1, (int)wordVector.size());
  EXPECT_EQ(1, wordVector[0].len);
  EXPECT_EQ(TYPE_SPACE, wordVector[0].type);
  CLEAN_TEST();

  RUN_TEST(" \r\n");
  EXPECT_EQ(1, (int)wordVector.size());
  EXPECT_EQ(1, wordVector[0].len);
  EXPECT_EQ(TYPE_SPACE, wordVector[0].type);
  CLEAN_TEST();
}
