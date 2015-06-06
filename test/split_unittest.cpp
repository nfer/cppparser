
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

TEST(SplitTest, Special_Slash) {
  // check /=
  runTest("max/=10;", 4, "max", "/=", "10", ";");
  // check /*
  runTest("/*multiline comments start", 6,
    "/*", "multiline", " ", "comments", " ", "start");
  // check //
  runTest("//comment", 2, "//", "comment");
  // check ///
  runTest("///comment", 2, "///", "comment");
  // check /(
  runTest("a/(b)", 5, "a", "/", "(", "b", ")");
}

TEST(SplitTest, Special_Colon) {
  // check ::
  runTest("a::b", 3, "a", "::", "b");
}

TEST(SplitTest, Special_OpenParen) {
  // check ()
  runTest("func();", 4, "func", "(", ")", ";");
}

TEST(SplitTest, Special_CloseParen) {
  // check ))
  runTest("func(aa())", 6, "func", "(", "aa", "(", ")", ")");
  // check ),
  runTest("func(aa(),bb)", 8, "func", "(", "aa", "(", ")", ",", "bb", ")");
  // check );
  runTest("func(aa);", 5, "func", "(", "aa", ")", ";");
  // check )<
  runTest("(a+b)<c", 7, "(", "a", "+", "b", ")", "<", "c");
  // check ):
  runTest("child():Parent()", 7, "child", "(", ")", ":", "Parent", "(", ")");
  // check ){
  runTest("func(){", 4, "func", "(", ")", "{");
  // check )*
  runTest("(a+b)+c", 7, "(", "a", "+", "b", ")", "+", "c");
  // check )/
  runTest("(a+b)-c", 7, "(", "a", "+", "b", ")", "-", "c");
  // check )*
  runTest("(a+b)*c", 7, "(", "a", "+", "b", ")", "*", "c");
  // check )/
  runTest("(a+b)/c", 7, "(", "a", "+", "b", ")", "/", "c");
  // check ).
  runTest("data().value", 5, "data", "(", ")", ".", "value");
}

TEST(SplitTest, Special) {
  // check ++ and ++)
  runTest("func(i++)", 5, "func", "(", "i", "++", ")");

  // check <<, <" and ";
  runTest("qWarning()<<\"hello world\";", 10, "qWarning", "(", ")", "<<",
    "\"", "hello", " ", "world", "\"", ";");

  // check ]= and =-
  runTest("mBookMarkList[i]=-1;", 8, "mBookMarkList", "[", "i", "]", "=",
    "-", "1", ";");

  // check ')
  runTest("intspace=width(QLatin1Char('9'))*digits;", 14,
    "intspace", "=", "width", "(", "QLatin1Char",
    "(", "'", "9", "'", ")", ")", "*", "digits", ";");

  // check <" and "<
  runTest("cout<<\"hello\"<<endl;", 8,
    "cout", "<<", "\"", "hello", "\"", "<<", "endl", ";");

  // check ==
  runTest("if(a==b)", 6, "if", "(", "a", "==", "b", ")");

  // check */
  runTest("multiline comments end*/", 6,
    "multiline", " ", "comments", " ", "end", "*/");

  // check ->
  runTest("event->rect().bottom()", 9,
    "event", "->", "rect", "(", ")", ".", "bottom", "(", ")");

  // check &&
  runTest("if(x&&y)", 6, "if", "(", "x", "&&", "y", ")");

  // check ;/
  runTest("int a;//comment", 6, "int", " ", "a", ";", "//", "comment");

  // check &,
  runTest("void updateLineNumberArea(const QRect &, int);", 14,
    "void", " ", "updateLineNumberArea", "(", "const", " ", "QRect", " ",
    "&", ",", " ", "int", ")", ";");

  // check ];
  runTest("int      mBookMarkList[BOOKMARKMAXCOUNT];", 7,
    "int", "      ", "mBookMarkList", "[", "BOOKMARKMAXCOUNT", "]", ";");
}
