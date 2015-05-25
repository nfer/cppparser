#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vector>

using namespace std;

enum Meta_Type{
    TYPE_SPACE,
    TYPE_WORD,
    TYPE_SPECIAL,
};

struct Meta_Data{
    Meta_Data(char * _data, int _type, int _line, int _pos){
        data = _data;
        type = _type;
        line = _line;
        pos  = _pos;
    }
    char * data;
    int    type;
    int    line;
    int    pos;
};

typedef vector<Meta_Data> Meta_Vector;

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
        if (isLineComment || isalnum(c) || c == '_'){
            CHECK_LAST_TYPE(TYPE_WORD);

            CHECK_ESCAPE_CHAR();

            if (isLineComment && c == '/')
                continue;

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
                    PRINT_LAST_TYPE();
                    word[wordlen++] = c;
                    break;

                case '\"':
                    PRINT_LAST_TYPE();
                    word[wordlen++] = c;
                    isStringMode = !isStringMode;
                    break;

                case '/':
                    if (wordlen > 0 && word[wordlen - 1] == '/')
                        isLineComment = true;
                    else
                        PRINT_LAST_TYPE();

                    word[wordlen++] = c;
                    break;

                case '\\':
                    PRINT_LAST_TYPE();
                    word[wordlen++] = c;
                    if (isStringMode)
                        isEscapeChar = true;
                    break;

                case ',':
                case ';':
                case ':':
                    PRINT_LAST_TYPE();
                    word[wordlen++] = c;
                    break;

                default:
                    word[wordlen++] = c;
                    break;
            }
        }
    }

    CHECK_LAST_TYPE(TYPE_SPACE);
}

void dumpWordVector(const char * str, Meta_Vector & wordVector)
{
    // dump wordVector
    for(size_t i=0; i<wordVector.size(); i++)
    {
        printf("[%3d:%2d] type %d, word:%s\n", wordVector[i].line, wordVector[i].pos,
            wordVector[i].type, wordVector[i].data);
    }
}

void freeWordVector(Meta_Vector & wordVector)
{
    // free wordVector data
    for(size_t i=0; i<wordVector.size(); i++)
    {
        free(wordVector[i].data);
    }
    wordVector.clear();
}

int main(int argc, char * argv[])
{
    Meta_Vector wordVector;
    wordVector.clear();
    // const char * str = "        if ( (parser->flags & SKIP_SP) && (*c == ' ' || *c == '\\t'))";
    // const char * str = "    HTTP_TOKEN1 = 0,// must be 0";
    // const char * str = "    a /= 3; // must be 0";
    // const char * str = "    a /= 3; ////// must be 0";
    // const char * str = "typedef vector< pair<char *,int> > Meta_Vector;";

    char * lineData = NULL;
    size_t lineLen = 0;
    ssize_t read;

    int lineOffset = -1, lineNum = 0;
    if (argc > 2){
        lineOffset = atoi(argv[2]);
    }

    FILE * fp = fopen(argv[1], "r");
    if(!fp)
    {
        printf("open file failed!\n");
        return -1;
    }

    while ((read = getline(&lineData, &lineLen, fp)) != -1)
    {
        lineNum++;
        if (lineNum < lineOffset){
            continue;
        }

        analysis(lineData, read, lineNum, wordVector);
#ifdef _DEBUG
        dumpWordVector(lineData, wordVector);
        freeWordVector(wordVector);
        getchar();
#endif
    }

    if (lineData)
        free(lineData);

    // dump
    dumpWordVector(lineData, wordVector);
    freeWordVector(wordVector);

    fclose(fp);
    return 0;
}
