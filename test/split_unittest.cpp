
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include "gtest/gtest.h"
#include "split.h"

int getMetaType(char c) {
  if (isalnum(c) || c == '_')
    return TYPE_WORD;
  else if (isspace(c))
    return TYPE_SPACE;
  else
    return TYPE_SPECIAL;
}

void runTest(const char * data, int itemSize, ...) {
  Meta_Vector wordVector;
  split(data, strlen(data), 0, wordVector);

  EXPECT_EQ(itemSize, (int)wordVector.size());

  const char * value = NULL;
  int len;
  int type;
  int pos = 0;

  va_list ap;
  va_start(ap, itemSize);
  for (int i=0; i < itemSize; i++) {
    value = va_arg(ap, char*);
    EXPECT_STREQ(wordVector[i].data, value);

    len = strlen(value);
    EXPECT_EQ(len, wordVector[i].len);

    type = getMetaType(value[0]);
    EXPECT_EQ(type, wordVector[i].type);

    EXPECT_EQ(pos, wordVector[i].pos);
    pos += len;
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
  runTest(" ",     1, " ");
  runTest("  ",    1, "  ");
  runTest("\t",    1, "\t");
  runTest("\t\t",  1, "\t\t");
  runTest("\t \t", 1, "\t \t");
  runTest(" \t ",  1, " \t ");
}

TEST(SplitTest, CRLF) {
  runTest(" \r",   1, " ");
  runTest(" \n",   1, " ");
  runTest(" \r\n", 1, " ");
  runTest(";\r",   1, ";");
  runTest(";\n",   1, ";");
  runTest(";\r\n", 1, ";");
}

TEST(SplitTest, Include) {
  runTest("#include <stdio.h>", 8,
    "#",  "include", " ",  "<", "stdio", ".", "h", ">");

  runTest("#include \"stdio.h\"", 8,
    "#",  "include", " ",  "\"", "stdio", ".", "h", "\"");

  runTest("#include <iostream>", 6,
    "#",  "include", " ",  "<", "iostream", ">");

  runTest("#include <sys/types.h>", 10,
    "#",  "include", " ",  "<", "sys", "/", "types", ".", "h", ">");
}

TEST(SplitTest, DefineConstant) {
  runTest("#define SPLIT_H", 4,
    "#",  "define", " ",  "SPLIT_H");

  runTest("#define PROJNAME     \"PROJNAME\"", 8,
    "#",  "define", " ",  "PROJNAME", "     ", "\"", "PROJNAME", "\"");

  runTest("#define MAXSIZE     (100)", 8,
    "#",  "define", " ",  "MAXSIZE", "     ", "(", "100", ")");
}

TEST(SplitTest, Viriable) {
  runTest("int a;", 4,
    "int", " ", "a", ";");

  runTest("int a = 10;", 8,
    "int", " ", "a", " ", "=", " ", "10", ";");

  runTest("char word[256] = {'\\0'};", 16,
    "char",  " ", "word",  "[", "256", "]", " ", "=",
    " ", "{", "'", "\\", "0", "'", "}", ";");

  runTest("char word[256] = {'\\0'};", 16,
    "char", " ", "word", "[",  "256", "]",  " ", "=",
    " ",    "{", "'",   "\\", "0",   "'", "}", ";");
}

TEST(SplitTest, Special) {
  // check /=
  runTest("max/=10;", 4, "max", "/=", "10", ";");

  // check ::
  runTest("QTextEdit::ExtraSelection selection;", 6,
    "QTextEdit", "::", "ExtraSelection", " ", "selection", ";");

  // check (), )), ), );
  runTest("func(aa(), bb());", 12,
    "func", "(", "aa", "(", ")", ",", " ", "bb", "(", ")", ")", ";");

  // check ++ and ++)
  runTest("func(i++)", 5, "func", "(", "i", "++", ")");

  // check <<, )<, <" and ";
  runTest("qWarning()<<\"hello world\";", 10, "qWarning", "(", ")", "<<",
    "\"", "hello", " ", "world", "\"", ";");

  // check ):, ){;
  runTest("LineNumberArea(CodeEditor*editor):QWidget(editor){", 12,
    "LineNumberArea", "(", "CodeEditor", "*", "editor", ")", ":",
    "QWidget", "(", "editor", ")", "{");

  // check ]= and =-
  runTest("mBookMarkList[i]=-1;", 8, "mBookMarkList", "[", "i", "]", "=",
    "-", "1", ";");
}
