
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "split.h"
#include "analysis.h"

using namespace std;

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

        split(lineData, read, lineNum, wordVector);
#ifdef _DEBUG
        dumpWordVector(lineData, wordVector);
        freeWordVector(wordVector);
        getchar();
#endif
    }

    if (lineData)
        free(lineData);

    // dump
    // dumpWordVector(lineData, wordVector);

    analysis(wordVector);

    freeWordVector(wordVector);

    fclose(fp);
    return 0;
}
