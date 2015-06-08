
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "split.h"
#include "analysis.h"

using namespace std;

void dumpWordVector(Meta_Vector & wordVector)
{
    // dump wordVector
    for(size_t i=0; i<wordVector.size(); i++)
    {
        Meta_Struct meta = wordVector[i];
        switch (meta.type) {
        case TYPE_WORD:
            printf("[%3d:%2d] type %d, word[%d]:%s\n", meta.line, meta.pos,
                meta.type, meta.len, meta.data.str);
            break;

        case TYPE_SPECIAL:
            printf("[%3d:%2d] type %d, word[%d]:%s\n", meta.line, meta.pos,
                meta.type, meta.len, meta.data.chr);
            break;

        case TYPE_SPACE:
            break;

        default:
            break;
        }
    }
}

void freeWordVector(Meta_Vector & wordVector)
{
    // free wordVector data
    for(size_t i=0; i<wordVector.size(); i++)
    {
        if (wordVector[i].type != TYPE_SPECIAL)
            free(wordVector[i].data.str);
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
    // const char * str = "\thello word";
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

        split(lineData, read, lineNum, wordVector);
#ifdef _DEBUG
        dumpWordVector(wordVector);
        freeWordVector(wordVector);
        getchar();
#endif
    }

    if (lineData)
        free(lineData);

    // dump
    // dumpWordVector(wordVector);

    analysis(wordVector);

    freeWordVector(wordVector);

    fclose(fp);
    return 0;
}
