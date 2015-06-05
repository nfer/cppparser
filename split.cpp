#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "split.h"

using namespace std;

void split(const char * data, int datalen, int line, Meta_Vector & wordVector)
{
#ifdef _DEBUG
    if (data[0] == '\r' || data[0] == '\n')
        cout << "[" << line << "] a blank line." << endl;
    else
        cout << "[" << line << "] data:" << data << endl;
#endif

    char word[256] = {'\0'};
    int wordlen = 0;
    int lastType = TYPE_SPACE;
    bool isEscapeChar = false;
    bool isStringMode = false;
    bool isSplitAfter = false;

#define PRINT_LAST_TYPE() \
    if (wordlen > 0) { \
        wordVector.push_back(Meta_Data(strdup(word), wordlen, lastType, line, i-wordlen)); \
        memset(word, 0x00, sizeof(word)); \
        wordlen = 0; \
    } \

#define CHECK_LAST_TYPE(type) \
    if(lastType != type){ \
        PRINT_LAST_TYPE(); \
        lastType = type; \
    }

#define CHECK_ESCAPE_CHAR() \
    if (isEscapeChar){ \
        isEscapeChar = false; \
        word[wordlen++] = c; \
        PRINT_LAST_TYPE(); \
        continue; \
    }

    int i;
    for (i = 0; i < datalen; i++){
        char c = data[i];
        if (isalnum(c) || c == '_'){
            CHECK_LAST_TYPE(TYPE_WORD);

            CHECK_ESCAPE_CHAR();

            word[wordlen++] = c;
        }
        
        else if (isspace(c)){
            CHECK_LAST_TYPE(TYPE_SPACE);
            if (c == ' ' || c == '\t')
                word[wordlen++] = c;
            else
                break;
        }
        else{
            CHECK_LAST_TYPE(TYPE_SPECIAL);

            CHECK_ESCAPE_CHAR();

            if (isSplitAfter) {
                PRINT_LAST_TYPE();
                isSplitAfter = false;
            }

            switch(c){
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case '\'':
                case ',':
                case ';':
                case '!':
                    PRINT_LAST_TYPE();
                    word[wordlen++] = c;
                    isSplitAfter = true;
                    break;

                case '\"':
                    PRINT_LAST_TYPE();
                    word[wordlen++] = c;
                    isStringMode = !isStringMode;
                    break;

                case ':':
                case '<':
                case '-':
                case '+':
                case '&':
                    if (wordlen == 0){
                        PRINT_LAST_TYPE();
                    }
                    else if (wordlen == 1 && word[0] != c) {
                        PRINT_LAST_TYPE();
                    }
                    else {
                        // do nothing
                    }

                    word[wordlen++] = c;
                    break;

                case '/':
                    if (wordlen == 0){
                        PRINT_LAST_TYPE();
                    }
                    else if (wordlen == 1 && word[0] != c) {
                        if (word[0] == '*') {
                            // do nothing
                        }
                        else{
                            PRINT_LAST_TYPE();
                        }
                    }
                    else {
                        // do nothing
                    }

                    word[wordlen++] = c;
                    break;

                case '=':
                    if (wordlen == 0){
                        PRINT_LAST_TYPE();
                    }
                    else if (wordlen == 1 && word[0] != c) {
                        if (word[0] == '+' || word[0] == '-' || word[0] == '*' || word[0] == '/' ||
                            word[0] == '%' || word[0] == '&' || word[0] == '|' || word[0] == '!') {
                            // do nothing
                        }
                        else{
                            PRINT_LAST_TYPE();
                        }
                    }
                    else {
                        // do nothing
                    }

                    word[wordlen++] = c;
                    break;

                case '\\':
                    PRINT_LAST_TYPE();
                    word[wordlen++] = c;
                    if (isStringMode)
                        isEscapeChar = true;
                    break;

                default:
                    word[wordlen++] = c;
                    break;
            }
        }
    }

    PRINT_LAST_TYPE();
}
