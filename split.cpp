#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "split.h"

using namespace std;

void analysis(const char * data, int datalen, int line, Meta_Vector & wordVector)
{
#ifdef _DEBUG
    if (data[0] == '\r' || data[0] == '\n')
        cout << "[" << line << "] a blank line." << endl;
    else
        cout << "[" << line << "] data:" << data << endl;
#endif

    int len = strlen(data);
    char word[256] = {'\0'};
    int wordlen = 0;
    int lastType = TYPE_SPACE;
    bool isLineComment = false;
    bool isEscapeChar = false;
    bool isStringMode = false;

#define PRINT_LAST_TYPE() \
    if (wordlen > 0) { \
        wordVector.push_back(Meta_Data(strdup(word), lastType, line, i-wordlen)); \
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
    for (i = 0; i < len; i++){
        char c = data[i];
        if (isalnum(c) || c == '_'){
            CHECK_LAST_TYPE(TYPE_WORD);

            CHECK_ESCAPE_CHAR();

            word[wordlen++] = c;
        }
        
        else if (isspace(c)){
            CHECK_LAST_TYPE(TYPE_SPACE);
        }
        else{
            CHECK_LAST_TYPE(TYPE_SPECIAL);

            CHECK_ESCAPE_CHAR();

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
                case ':':
                case '*':
                case '&':
                case '!':
                    PRINT_LAST_TYPE();
                    word[wordlen++] = c;
                    break;

                case '\"':
                    PRINT_LAST_TYPE();
                    word[wordlen++] = c;
                    isStringMode = !isStringMode;
                    break;

                case '/':
                    if (wordlen > 0 && word[wordlen - 1] == '/'){
                        word[wordlen++] = c;
                        if ( !isLineComment ){
                            isLineComment = true;
                            PRINT_LAST_TYPE();
                        }
                    }
                    else{
                        PRINT_LAST_TYPE();
                        word[wordlen++] = c;
                    }
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

    CHECK_LAST_TYPE(TYPE_SPACE);
}
