#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vector>

using namespace std;

enum ANALYSIS_TYPE{
    TYPE_SPACE,
    TYPE_WORD,
    TYPE_SPECIAL,
};

typedef vector< pair<char *,int> > WORD_Vector;

void analysis(const char * data, WORD_Vector & wordVector)
{
    int len = strlen(data);
    char word[256] = {'\0'};
    int wordlen = 0;
    int lastType = TYPE_SPACE;
    bool isLineComment = false;

#define PRINT_LAST_TYPE() \
    if (wordlen > 0) { \
        wordVector.push_back(make_pair(strdup(word), lastType)); \
        memset(word, 0x00, sizeof(word)); \
        wordlen = 0; \
    } \

#define CHECK_LAST_TYPE(type) \
    if(lastType != type){ \
        PRINT_LAST_TYPE(); \
        lastType = type; \
    }

    for (int i=0; i<len; i++){
        char c = data[i];
        if (isLineComment || isalnum(c) || c == '_'){
            CHECK_LAST_TYPE(TYPE_WORD);

            if (isLineComment && c == '/')
                continue;

            word[wordlen++] = c;
        }
        
        else if (isspace(c)){
            CHECK_LAST_TYPE(TYPE_SPACE);
        }
        else{
            CHECK_LAST_TYPE(TYPE_SPECIAL);
            switch(c){
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case '\'':
                case '\"':
                    PRINT_LAST_TYPE();
                    word[wordlen++] = c;
                    PRINT_LAST_TYPE();
                    break;

                case '/':
                    if (wordlen > 0 && word[wordlen - 1] == '/')
                        isLineComment = true;
                    else
                        PRINT_LAST_TYPE();

                    word[wordlen++] = c;
                    break;

                case '\\':
                case ',':
                case ';':
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

void dumpWordVector(const char * str, WORD_Vector & wordVector)
{
    cout << "str(len:" << strlen(str) << "):" << str << endl;
    // dump wordVector
    for(size_t i=0; i<wordVector.size(); i++)
    {
        cout << "type :" << wordVector[i].second << "\tword : " << wordVector[i].first << endl;
    }
}

void freeWordVector(WORD_Vector & wordVector)
{
    // free wordVector data
    for(size_t i=0; i<wordVector.size(); i++)
    {
        free(wordVector[i].first);
    }
    wordVector.clear();
}

int main(int argc, char * argv[])
{
    WORD_Vector wordVector;
    wordVector.clear();
    // const char * str = "        if ( (parser->flags & SKIP_SP) && (*c == ' ' || *c == '\\t'))";
    // const char * str = "    HTTP_TOKEN1 = 0,// must be 0";
    // const char * str = "    a /= 3; // must be 0";
    // const char * str = "    a /= 3; ////// must be 0";
    // const char * str = "typedef vector< pair<char *,int> > WORD_Vector;";

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int lineStart = -1, lineIdx = 0;
    if (argc > 2){
        lineStart = atoi(argv[2]);
    }

    FILE * fp = fopen(argv[1], "r");
    if(!fp)
    {
        printf("open file failed!\n");
        return -1;
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        lineIdx++;
        if (lineIdx < lineStart)
            continue;

        printf("line %d\t", lineIdx);
        analysis(line, wordVector);
        dumpWordVector(line, wordVector);
        freeWordVector(wordVector);
        getchar();
    }

    if (line)
        free(line);

    fclose(fp);
    return 0;
}