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
    int len = 0;
    int type = TYPE_SPACE;
    bool isEscapeChar = false;
    bool isSplitAfter = false;

#define PRINT_LAST_TYPE() \
    if (len > 0) { \
        if (type == TYPE_SPECIAL) \
            wordVector.push_back(Meta_Struct(word[0], type, line, i-len)); \
        else \
            wordVector.push_back(Meta_Struct(word, len, type, line, i-len)); \
        memset(word, 0x00, sizeof(word)); \
        len = 0; \
    }

#define CHECK_LAST_TYPE(_type) \
    if(type != _type){ \
        PRINT_LAST_TYPE(); \
        type = _type; \
    }

    int i;
    for (i = 0; i < datalen; i++){
        char c = data[i];

        if (isalnum(c) || c == '_'){
            if (isSplitAfter){
                isSplitAfter = false;
                PRINT_LAST_TYPE();
            }

            if (isEscapeChar){
                isEscapeChar = false;
                isSplitAfter = true;
            }

            CHECK_LAST_TYPE(TYPE_WORD);
            word[len++] = c;
        }
        else if (isspace(c)){
            CHECK_LAST_TYPE(TYPE_SPACE);
            if (c == ' ' || c == '\t')
                word[len++] = c;
            else
                break;
        }
        else{
            PRINT_LAST_TYPE();
            type = TYPE_SPECIAL;
            word[len++] = c;

            if (c == '\\') {
                isEscapeChar = !isEscapeChar;
            }
            else{
                if (isEscapeChar){
                    isEscapeChar = false;
                }
            }
        }
    }

    PRINT_LAST_TYPE();
}

const char * getTypeName(int type) {
    switch (type) {
        case TYPE_SPACE:
            return "space";
        case TYPE_WORD:
            return "word";
        case TYPE_SPECIAL:
            return "special";
        default:
            return "error type";
    }
}
