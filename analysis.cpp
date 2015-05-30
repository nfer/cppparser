
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analysis.h"

void getIncludeFiles(Meta_Vector & wordVector, size_t index)
{
	int curLine = wordVector[index].line;
	int headerType = TYPE_SYSTEM;
	char headerStr[256] = {'\0'};

	// #include <string.h> or #include <vector>
	if (index + 4 >= wordVector.size() || wordVector[index+4].line != curLine){
		printf("not a full header include statement [%d, %d].\n", (int)index, (int)wordVector.size());
		return ;
	}

	if (wordVector[index+2].data[0] == '\"'){
		headerType = TYPE_USER;
	}
	else if (wordVector[index+2].data[0] == '<'){
		headerType = TYPE_SYSTEM;
	}
	else{
		printf("not a valid header include statement [%s].\n", wordVector[index+2].data);
		return;
	}

	// skip '#' and "include" and header type char '"' or '<'
    for(size_t i=index+3; i < wordVector.size(); i++)
    {
    	if (wordVector[i].line != wordVector[index].line)
    		break;

    	if (strcmp(wordVector[i].data, "//") == 0){
    		printf("comment occurs at pos %d\n", wordVector[i].pos);
    		break;
    	}

    	if (headerType == TYPE_SYSTEM && wordVector[i].data[0] == '>'){
    		printf("Line %d is a system header include: %s\n", curLine, headerStr);
    		break;
    	}
    	else if (headerType == TYPE_USER && wordVector[i].data[0] == '\"'){
    		printf("Line %d is a user   header include: %s\n", curLine, headerStr);
    		break;
    	}
    	else if (wordVector[i].type == TYPE_WORD ||
    		strcmp(wordVector[i].data, "/") == 0 ||
    		strcmp(wordVector[i].data, ".") == 0){
    		strcat(headerStr, wordVector[i].data);
    	}
    	else{
			printf("not a valid header include statement [%s].\n", wordVector[i].data);
			break;
    	}
    }
}

void getDefine(Meta_Vector & wordVector, size_t index)
{
    int curLine = wordVector[index].line;
    int defineType = TYPE_CONSTANT;
    char defineStr[256] = {'\0'};

    // #define A a or just #define A
    if (index + 2 >= wordVector.size() || wordVector[index+2].line != curLine){
        printf("not a full define statement [%d, %d].\n", (int)index, (int)wordVector.size());
        return ;
    }

    if (wordVector[index+3].line != curLine) {
        if (wordVector[index+2].type == TYPE_WORD){
            printf("Line %d is a NULL constant define: %s\n", curLine, wordVector[index+2].data);
            return ;
        }
        else
            printf("not a valid header include statement [%s].\n", wordVector[index+2].data);
    }
    else{
        if (wordVector[index+2].data[0] == '('){
            defineType = TYPE_FUNCTION;
        }
        else if (wordVector[index+2].type == TYPE_WORD){
            defineType = TYPE_CONSTANT;
        }
        else{
            printf("not a valid header include statement [%s].\n", wordVector[index+2].data);
            return;
        }
    }

    if (defineType == TYPE_CONSTANT){

        int lastPos = -1, defineStrLen = 0, dataLen = 0;
        // skip '#' and "define" and constant define
        for(size_t i=index+3; i < wordVector.size(); i++)
        {
            if (wordVector[i].line != wordVector[index].line)
                break;

            if (strcmp(wordVector[i].data, "//") == 0){
                printf("comment occurs at pos %d\n", wordVector[i].pos);
                break;
            }

            if (lastPos == -1){
                lastPos = wordVector[i].pos;
            }

            // FIXME: should save space string on wordVector
            if (lastPos != wordVector[i].pos) {
                memset(defineStr + defineStrLen, 0x20, wordVector[i].pos - lastPos);
            }

            strcat(defineStr, wordVector[i].data);

            // FIXME: should add dataLen on Meta_Data struct
            dataLen = strlen(wordVector[i].data);
            lastPos = wordVector[i].pos + dataLen;
            defineStrLen += dataLen;
        }

        printf("Line %d is a constant define: %s, value is %s\n", curLine,
            wordVector[index+2].data, defineStr);
    }
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
            if (wordVector[i].data[0] == '#' && i < vectorSize-1){
                if (strcmp(wordVector[i+1].data, "include") == 0) {
                    getIncludeFiles(wordVector, i);
                }
                else if (strcmp(wordVector[i+1].data, "define") == 0) {
                    getDefine(wordVector, i);
                }
                else if (strcmp(wordVector[i+1].data, "if") == 0) {
                    printf("Line %d is a if compile condition.\n", wordVector[i].line);
                }
                else if (strcmp(wordVector[i+1].data, "ifdef") == 0) {
                    printf("Line %d is a ifdef compile condition.\n", wordVector[i].line);
                }
                else if (strcmp(wordVector[i+1].data, "ifndef") == 0) {
                    printf("Line %d is a ifndef compile condition.\n", wordVector[i].line);
                }
                else if (strcmp(wordVector[i+1].data, "else") == 0) {
                    printf("Line %d is a else compile condition.\n", wordVector[i].line);
                }
                else if (strcmp(wordVector[i+1].data, "endif") == 0) {
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
