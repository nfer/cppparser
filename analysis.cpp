
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analysis.h"

int restoreLine(Meta_Vector & wordVector, size_t index, char * line) {
    int curLine = wordVector[index].line;
    int lineLen = 0;

    for(size_t i=index; i < wordVector.size(); i++) {
        Meta_Struct meta = wordVector[i];
        if (meta.line != curLine)
            break;

        if (meta.type == TYPE_SPECIAL) {
            strcat(line, meta.data.chr);
            lineLen += meta.len;
        }
        else{
            strcat(line, meta.data.str);
            lineLen += meta.len;
        }
    }
    return 0;
}

void getIncludeFiles(Meta_Vector & wordVector, size_t index)
{
    int curLine = wordVector[index].line;
    int headerType = TYPE_SYSTEM;
    char headerStr[256] = {'\0'};
    int  headerStrLen = 0;

    // #include <string.h> or #include <vector>
    if (index + 5 >= wordVector.size() || wordVector[index+5].line != curLine){
        restoreLine(wordVector, index, headerStr);
        printf("Line %d is not a full header include statement: %s\n",
            curLine, headerStr);
        return ;
    }

    if (wordVector[index+3].data.chr[0] == '\"'){
        headerType = TYPE_USER;
    }
    else if (wordVector[index+3].data.chr[0] == '<'){
        headerType = TYPE_SYSTEM;
    }
    else{
        restoreLine(wordVector, index, headerStr);
        printf("Line %d is not a valid header include statement: %s.\n",
            curLine, headerStr);
        return;
    }

    bool isLineComment = false;

    // skip '#', "include", space and header type char '"' or '<'
    for(size_t i=index+4; i < wordVector.size(); i++)
    {
        Meta_Struct meta = wordVector[i];
        if (meta.line != curLine)
            break;

        if (isLineComment) {
            isLineComment = false;
            if (meta.data.chr[0] == '/'){
                // printf("comment occurs at pos %d\n", meta.pos);
                headerStrLen--;
                headerStr[headerStrLen] = '\0';
                break;
            }
        }

        if (meta.data.chr[0] == '/'){
            isLineComment = true;
        }

        if (headerType == TYPE_SYSTEM && meta.data.chr[0] == '>'){
            printf("Line %d is a system header include: %s\n", curLine, headerStr);
            break;
        }
        else if (headerType == TYPE_USER && meta.data.chr[0] == '\"'){
            printf("Line %d is a user   header include: %s\n", curLine, headerStr);
            break;
        }
        else if (meta.type == TYPE_WORD){
            strcat(headerStr, wordVector[i].data.str);
            headerStrLen += meta.len;
        }
        else if (meta.type == TYPE_SPECIAL &&
            (meta.data.chr[0] == '/' || meta.data.chr[0] == '.') ) {
            strcat(headerStr, wordVector[i].data.chr);
            headerStrLen += meta.len;
        }
        else{
            restoreLine(wordVector, index, headerStr);
            printf("Line %d is not a valid header include statement: %s.\n",
                curLine, headerStr);
            break;
        }
    }
}

void getDefine(Meta_Vector & wordVector, size_t index, bool & needNextLine)
{
    int curLine = wordVector[index].line;
    int defineType = TYPE_CONSTANT;
    char defineStr[256] = {'\0'};

    // #define A
    if (index + 3 >= wordVector.size() || wordVector[index+3].line != curLine){
        restoreLine(wordVector, index, defineStr);
        printf("Line %d is not a full define statement:%s\n", curLine, defineStr);
        return ;
    }

    if (wordVector[index+4].line != curLine) {
        if (wordVector[index+3].type == TYPE_WORD)
            printf("Line %d is a NULL constant define: %s\n", curLine,
                    wordVector[index+3].data.str);
        else {
            restoreLine(wordVector, index, defineStr);
            printf("Line %d is not a valid NULL constant define statement:%s\n",
                curLine, defineStr);
        }

        return ;
    }
    else{
        if (wordVector[index+4].data.chr[0] == '('){
            defineType = TYPE_FUNCTION;
        }
        else if (wordVector[index+4].type == TYPE_SPACE){
            defineType = TYPE_CONSTANT;
        }
        else{
            restoreLine(wordVector, index, defineStr);
            printf("Line %d is not a valid define statement:%s\n", curLine, defineStr);
            return;
        }
    }

    if (defineType == TYPE_CONSTANT){
        int defineStrLen = 0;
        bool addSpace = false;
        bool isLineComment = false;

        // skip '#', "define", space, constant define and space
        for(size_t i=index+5; i < wordVector.size(); i++)
        {
            Meta_Struct meta = wordVector[i];
            if (meta.line != curLine)
                break;

            if (isLineComment) {
                isLineComment = false;
                if (meta.data.chr[0] == '/'){
                    // printf("comment occurs at pos %d\n", meta.pos);
                    defineStrLen--;
                    defineStr[defineStrLen] = '\0';
                    break;
                }
            }

            if (meta.data.chr[0] == '/'){
                isLineComment = true;
            }

            if (meta.type == TYPE_SPACE){
                if (addSpace == false) {
                    addSpace = true;
                }
            }
            else{
                if (addSpace == true) {
                    addSpace = false;
                    strcat(defineStr, wordVector[i-1].data.str);
                    defineStrLen += wordVector[i-1].len;
                }

                if (meta.type == TYPE_WORD)
                    strcat(defineStr, meta.data.str);
                else
                    strcat(defineStr, meta.data.chr);
                defineStrLen += meta.len;
            }
        }

        if (defineStrLen == 0)
            printf("Line %d is a NULL constant define: %s\n", curLine,
                wordVector[index+3].data.str);
        else
            printf("Line %d is a constant define: %s, value is %s, len is %d\n", curLine,
                wordVector[index+3].data.str, defineStr, defineStrLen);

    }
    else{
        printf("Line %d is a function define: %s\n", curLine, wordVector[index+3].data.str);
        restoreLine(wordVector, index, defineStr);
        printf("%s\n", defineStr);
    }
}

void analysis(Meta_Vector & wordVector)
{
    int curLine = 0;
    bool newLineFlag = true;
    size_t vectorSize = wordVector.size();
    bool needNextLine = false;

    for(size_t i=0; i < vectorSize; i++)
    {
        if (curLine == wordVector[i].line)
            newLineFlag = false;
        else
            newLineFlag = true;

        if (newLineFlag)
        {
            if (wordVector[i].data.chr[0] == '#' && i < vectorSize-1){
                if (strcmp(wordVector[i+1].data.str, "include") == 0) {
                    getIncludeFiles(wordVector, i);
                }
                else if (strcmp(wordVector[i+1].data.str, "define") == 0) {
                    getDefine(wordVector, i, needNextLine);
                }
                else if (strcmp(wordVector[i+1].data.str, "if") == 0) {
                    printf("Line %d is a if compile condition.\n", wordVector[i].line);
                }
                else if (strcmp(wordVector[i+1].data.str, "ifdef") == 0) {
                    printf("Line %d is a ifdef compile condition.\n", wordVector[i].line);
                }
                else if (strcmp(wordVector[i+1].data.str, "ifndef") == 0) {
                    printf("Line %d is a ifndef compile condition.\n", wordVector[i].line);
                }
                else if (strcmp(wordVector[i+1].data.str, "else") == 0) {
                    printf("Line %d is a else compile condition.\n", wordVector[i].line);
                }
                else if (strcmp(wordVector[i+1].data.str, "endif") == 0) {
                    printf("Line %d is a endif compile condition.\n", wordVector[i].line);
                }
                else {
                    printf("Line %d is not support now.\n", wordVector[i].line);
                }
            }

            newLineFlag = false;
            curLine = wordVector[i].line;
        }
        else{
            continue;
        }
    }
}
