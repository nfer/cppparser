
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "split.h"

void runTest(const char * data, int itemSize, ...) {
  Meta_Vector wordVector;
  split(data, strlen(data), 0, wordVector);

  EXPECT_EQ(itemSize, (int)wordVector.size());

  va_list ap;
  va_start(ap, itemSize); // data, len, type, pos
  for (int i=0; i < itemSize; i++) {
    EXPECT_EQ(0, strcmp(wordVector[i].data, va_arg(ap, char*)));
    EXPECT_EQ(va_arg(ap, int), wordVector[i].len);
    EXPECT_EQ(va_arg(ap, int), wordVector[i].type);
    EXPECT_EQ(va_arg(ap, int), wordVector[i].pos);
  }
  va_end(ap);

  // free wordVector data
  for(size_t i=0; i<wordVector.size(); i++)
  {
      free(wordVector[i].data);
  }
  wordVector.clear();
}

TEST(SplitTest, Blank) {
  runTest("", 0);
}

TEST(SplitTest, Space) {
  runTest(" ",     1, " ",     1, TYPE_SPACE, 0);
  runTest("  ",    1, "  ",    2, TYPE_SPACE, 0);
  runTest("\t",    1, "\t",    1, TYPE_SPACE, 0);
  runTest("\t\t",  1, "\t\t",  2, TYPE_SPACE, 0);
  runTest("\t \t", 1, "\t \t", 3, TYPE_SPACE, 0);
  runTest(" \t ",  1, " \t ",  3, TYPE_SPACE, 0);
}

TEST(SplitTest, CRLF) {
  runTest(" \r",   1, " ", 1, TYPE_SPACE,   0);
  runTest(" \n",   1, " ", 1, TYPE_SPACE,   0);
  runTest(" \r\n", 1, " ", 1, TYPE_SPACE,   0);
  runTest(";\r",   1, ";", 1, TYPE_SPECIAL, 0);
  runTest(";\n",   1, ";", 1, TYPE_SPECIAL, 0);
  runTest(";\r\n", 1, ";", 1, TYPE_SPECIAL, 0);
}

TEST(SplitTest, Include) {
  runTest("#include <stdio.h>", 8,
    "#",     1, TYPE_SPECIAL, 0,  "include", 7, TYPE_WORD,    1,
    " ",     1, TYPE_SPACE,   8,  "<",       1, TYPE_SPECIAL, 9,
    "stdio", 5, TYPE_WORD,    10, ".",       1, TYPE_SPECIAL, 15,
    "h",     1, TYPE_WORD,    16, ">",       1, TYPE_SPECIAL, 17);

  runTest("#include \"stdio.h\"", 8,
    "#",     1, TYPE_SPECIAL, 0,  "include",  7, TYPE_WORD,    1,
    " ",     1, TYPE_SPACE,   8,  "\"",       1, TYPE_SPECIAL, 9,
    "stdio", 5, TYPE_WORD,    10, ".",        1, TYPE_SPECIAL, 15,
    "h",     1, TYPE_WORD,    16, "\"",       1, TYPE_SPECIAL, 17);

  runTest("#include <iostream>", 6,
    "#",        1, TYPE_SPECIAL, 0,  "include", 7, TYPE_WORD,    1,
    " ",        1, TYPE_SPACE,   8,  "<",       1, TYPE_SPECIAL, 9,
    "iostream", 8, TYPE_WORD,    10, ">",       1, TYPE_SPECIAL, 18);

  runTest("#include <sys/types.h>", 10,
    "#",     1, TYPE_SPECIAL, 0,  "include", 7, TYPE_WORD,    1,
    " ",     1, TYPE_SPACE,   8,  "<",       1, TYPE_SPECIAL, 9,
    "sys",   3, TYPE_WORD,    10, "/",       1, TYPE_SPECIAL, 13,
    "types", 5, TYPE_WORD,    14, ".",       1, TYPE_SPECIAL, 19,
    "h",     1, TYPE_WORD,    20, ">",       1, TYPE_SPECIAL, 21);
}
