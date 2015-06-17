
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

int handleSlash(Meta_Vector & wordVector, size_t index, int & commentType) {
    int curLine = wordVector[index].line;
    Meta_Struct meta = wordVector[index+1];
    size_t i;

    if (meta.line != curLine) {
        printf("nothing but next line after '/'\n");
        return 0;
    }

    if (meta.type != TYPE_SPECIAL) {
        printf("space or word after '/'\n");
        return 0;
    }

    char nextChr = meta.data.chr[0];
    if (nextChr == '/') {
        commentType = TYPE_SIGLELINE;

        for(i=index+1; i < wordVector.size(); i++) {
            if (wordVector[i].line != curLine)
                break;
        }

        printf("TYPE_SIGLELINE start at %d, end at %d\n", index, i);
        return (int)(i - index);
    }
    else if (nextChr == '*') {
        commentType = TYPE_MULTILINE;

        for(i=index+1; i < wordVector.size(); i++) {
            if (wordVector[i].type == TYPE_SPECIAL && wordVector[i].data.chr[0] == '/' &&
                wordVector[i-1].type == TYPE_SPECIAL && wordVector[i-1].data.chr[0] == '*')
                break;
        }

        printf("TYPE_MULTILINE start at %d, end at %d\n", index, i);
        return (int)(i - index);
    }
    else {
        printf("after '/' is %c\n", nextChr);
        return 0;
    }
}


int handleBackSlash(Meta_Vector & wordVector, size_t index, bool & nextline, char & escapeChar) {
    int curLine = wordVector[index].line;
    Meta_Struct meta = wordVector[index+1];

    if (meta.line != curLine) {
        printf("it's a nextline flag.\n");
        nextline = true;
        return 0;
    }
    else if (meta.type == TYPE_SPACE) {
        if (wordVector[index+2].line != curLine) {
            printf("it's a nextline flag.\n");
            nextline = true;
            return 1;
        }
        else {
            printf("error with next char(%c) after slash.\n", meta.data.str[0]);
            return -1;
        }
    }
    else {
        char c = '\0';
        if (meta.type == TYPE_SPECIAL)
            c = meta.data.chr[0];
        else
            c = meta.data.str[0];
        printf("is a escape  char \\%c\n", c);

        switch(c){
            case 'a':
                escapeChar = '\a';
                break;
            case 'b':
                escapeChar = '\b';
                break;
            case 'f':
                escapeChar = '\f';
                break;
            case 'n':
                escapeChar = '\n';
                break;
            case 'r':
                escapeChar = '\r';
                break;
            case 't':
                escapeChar = '\t';
                break;
            case 'v':
                escapeChar = '\v';
                break;
            case '\\':
                escapeChar = '\\';
                break;
            case '\'':
                escapeChar = '\'';
                break;
            case '\"':
                escapeChar = '\"';
                break;
            case '\0':
                escapeChar = '\0';
                break;
            default:
                printf("error with next char(%c) after slash.\n", c);
                return -1;
        }
        return 0;
    }
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

int handlDefine(Meta_Vector & wordVector, size_t index)
{
    int step = CHECK_SPACE;
    int curLine = wordVector[index].line;
    int defineType;

    for(size_t i=index+2; i < wordVector.size(); i++) {
        Meta_Struct meta = wordVector[i];
        if (meta.line != curLine)
                break;

        if (meta.type == TYPE_SPECIAL && meta.data.chr[0] == '/') {
            int commentType;
            int ret = handleSlash(wordVector, i, commentType);
            if (ret > 0) {
                if (wordVector[i-1].line == meta.line && wordVector[i-1].type == TYPE_SPACE &&
                    wordVector[i+ret+1].line == meta.line && wordVector[i+ret+1].type == TYPE_SPACE)
                    ret++;
                i += ret;
                // printf("skip %d after comment\n", ret);
                continue;
            }
        }

#define CHECK_TYPE(_type) \
        if (meta.type != _type) { \
            printf("[%d]expect type %s after \"#define\", but actually type is %d(%s)\n", \
                meta.pos, getTypeName(_type), meta.type, getTypeName(meta.type)); \
            return -DEFINE_FMT_ERROR; \
        }

        switch (step) {
            case CHECK_SPACE:
                CHECK_TYPE(TYPE_SPACE);
                step = CHECK_WORD;
                break;
            case CHECK_WORD:
                CHECK_TYPE(TYPE_WORD);
                printf("define name is: %s\n", meta.data.str);
                step = CHECK_TYPE;
                break;
            case CHECK_TYPE:
                if (meta.type == TYPE_SPACE) {
                    printf("define type is constant\n");
                    defineType = TYPE_CONSTANT;
                }
                else if (meta.type == TYPE_SPECIAL && meta.data.chr[0] == '(') {
                    printf("define type is function\n");
                    defineType = TYPE_FUNCTION;
                }
                else {
                    printf("define type error\n");
                    return -1;
                }
                step = CHECK_NONE;
                break;
        }
    }
    return 0;

    char defineStr[1024] = {'\0'};
    int defineStrLen = 0;
    bool addSpace = false;

    // #define A
    if (index + 3 >= wordVector.size() || wordVector[index+3].line != curLine){
        restoreLine(wordVector, index, defineStr);
        printf("Line %d is not a full define statement:%s\n", curLine, defineStr);
        return 0;
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

        return 0;
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
            return 0;
        }
    }

    if (defineType == TYPE_CONSTANT){
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

            if (meta.data.chr[0] == '\\'){
                if (wordVector[i-1].type == TYPE_SPECIAL && wordVector[i-1].data.chr[0] == '\\'){
                    // printf("it's already escaped with this slash.\n");
                }
                else {
                    bool nextline = false;
                    char escapeChar = '\0';
                    handleBackSlash(wordVector, i, nextline, escapeChar);
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
        // restoreLine(wordVector, index, defineStr);
        // printf("%s\n", defineStr);
        bool needNextLine = false;

        // skip '#', "define", space, function define and ()
        for(size_t i=index+3; i < wordVector.size(); i++) {
            Meta_Struct meta = wordVector[i];
            if (meta.line != curLine){
                if (needNextLine){
                    curLine = meta.line;
                    needNextLine = false;
                }
                else
                    break;
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
                else{
                    if (meta.data.chr[0] == '\\'){
                        strcat(defineStr, "\n");
                        needNextLine = true;
                    }
                    else
                        strcat(defineStr, meta.data.chr);
                }
                defineStrLen += meta.len;
            }
        }
        printf("Line %d is a function define:\n%s\n", curLine, defineStr);
    }

    return 0;
}

void analysis(Meta_Vector & wordVector)
{
    int curLine = 0;
    bool newLineFlag = true;
    size_t vectorSize = wordVector.size();

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
                    handlDefine(wordVector, i);
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
