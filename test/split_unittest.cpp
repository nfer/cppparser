
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

TEST(SplitTest, Slash) {
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

TEST(SplitTest, Colon) {
  // check ::
  runTest("a::b", 3, "a", "::", "b");
}

TEST(SplitTest, OpenParenthesis) {
  // check ()
  runTest("func();", 4, "func", "(", ")", ";");
}

TEST(SplitTest, CloseParenthesis) {
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

TEST(SplitTest, SingleQuotation) {
  // check ')
  runTest("if(a=='9')", 8, "if", "(", "a", "==", "'", "9", "'", ")");
}

TEST(SplitTest, DoubleQuotation) {
  // check ";
  runTest("data=\"hello\";", 6, "data", "=", "\"", "hello", "\"", ";");
  // check ",
  runTest("data=\"hello\",b;", 8,
    "data", "=", "\"", "hello", "\"", ",", "b", ";");
  // check "<
  runTest("cout<<\"hello\"<<endl;", 8,
    "cout", "<<", "\"", "hello", "\"", "<<", "endl", ";");
}

TEST(SplitTest, Plus) {
  // check ++
  runTest("i++", 2, "i", "++");
  // check +=
  runTest("i+=1;", 4, "i", "+=", "1", ";");
  // check +.
  runTest("1+.2;", 5, "1", "+", ".", "2", ";");
  // check +!
  runTest("a+!b;", 5, "a", "+", "!", "b", ";");

  // check +*
  runTest("i+*pointer", 4, "i", "+", "*", "pointer");
  // check +&
  runTest("i+&data", 4, "i", "+", "&", "data");

  // check +,
  runTest("i++,j++", 5, "i", "++", ",", "j", "++");
  // check +;
  runTest("i++;", 3, "i", "++", ";");
  // check +(
  runTest("a+(b)", 5, "a", "+", "(", "b", ")");
  // check +)
  runTest("func(i++)", 5, "func", "(", "i", "++", ")");
}

TEST(SplitTest, Minus) {
  // check --
  runTest("i--", 2, "i", "--");
  // check -;
  runTest("i--;", 3, "i", "--", ";");
  // check -)
  runTest("func(i--)", 5, "func", "(", "i", "--", ")");
  // check ->
  runTest("data->value", 3, "data", "->", "value");
}

TEST(SplitTest, LessThan) {
  // check <<
  runTest("cout<<data;", 4, "cout", "<<", "data", ";");
  // check <"
  runTest("cout<<\"data\";", 6, "cout", "<<", "\"", "data", "\"", ";");
}

TEST(SplitTest, Equal) {
  // check =-
  runTest("a=-1;", 5, "a", "=", "-", "1", ";");
  // check ==
  runTest("if(a==b)", 6, "if", "(", "a", "==", "b", ")");
}

TEST(SplitTest, And) {
  // check &&
  runTest("if(x&&y)", 6, "if", "(", "x", "&&", "y", ")");
  // check &)
  runTest("void func(QRect &);", 9,
    "void", " ", "func", "(", "QRect", " ", "&", ")", ";");
  // check &,
  runTest("void func(QRect &, int);", 12,
    "void", " ", "func", "(", "QRect", " ", "&", ",", " ", "int", ")", ";");
}

TEST(SplitTest, CloseBracket) {
  // check ]=
  runTest("data[i]=0;", 7,  "data", "[", "i", "]", "=", "0", ";");
  // check ];
  runTest("int data[4];", 7, "int", " ", "data", "[", "4", "]", ";");
}

TEST(SplitTest, Semicolon) {
  // check ;/
  runTest("int a;//comment", 6, "int", " ", "a", ";", "//", "comment");
}

TEST(SplitTest, Star) {
  // check */
  runTest("multiline comments end*/", 6,
    "multiline", " ", "comments", " ", "end", "*/");
}
