
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
	if (index + 5 >= wordVector.size() || wordVector[index+5].line != curLine){
		printf("not a full header include statement [%d, %d].\n", (int)index, (int)wordVector.size());
		return ;
	}

	if (wordVector[index+3].data[0] == '\"'){
		headerType = TYPE_USER;
	}
	else if (wordVector[index+3].data[0] == '<'){
		headerType = TYPE_SYSTEM;
	}
	else{
		printf("not a valid header include statement [%s].\n", wordVector[index+3].data);
		return;
	}

	// skip '#', "include", space and header type char '"' or '<'
    for(size_t i=index+4; i < wordVector.size(); i++)
    {
    	if (wordVector[i].line != wordVector[index].line)
    		break;

    	if (strncmp(wordVector[i].data, "//", 2) == 0){
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

    // #define A
    if (index + 3 >= wordVector.size() || wordVector[index+3].line != curLine){
        printf("not a full define statement [%d, %d].\n", (int)index, (int)wordVector.size());
        return ;
    }

    if (wordVector[index+4].line != curLine) {
        if (wordVector[index+3].type == TYPE_WORD){
            printf("Line %d is a NULL constant define: %s\n", curLine, wordVector[index+3].data);
            return ;
        }
        else
            printf("not a valid define statement [%s].\n", wordVector[index+2].data);
    }
    else{
        if (wordVector[index+4].data[0] == '('){
            defineType = TYPE_FUNCTION;
        }
        else if (wordVector[index+4].type == TYPE_SPACE &&
                 wordVector[index+5].type == TYPE_WORD){
            defineType = TYPE_CONSTANT;
        }
        else{
            printf("not a valid define statement [%s].\n", wordVector[index+3].data);
            return;
        }
    }

    if (defineType == TYPE_CONSTANT){
        int defineStrLen = 0;
        bool addSpace = false;

        // skip '#', "define", space, constant define and space
        for(size_t i=index+5; i < wordVector.size(); i++)
        {
            if (wordVector[i].line != wordVector[index].line)
                break;

            if (strncmp(wordVector[i].data, "//", 2) == 0){
                printf("comment occurs at pos %d\n", wordVector[i].pos);
                break;
            }

            if (wordVector[i].type == TYPE_SPACE){
                if (addSpace == false) {
                    addSpace = true;
                }
            }
            else{
                if (addSpace == true) {
                    addSpace = false;
                    strcat(defineStr, wordVector[i-1].data);
                    defineStrLen += wordVector[i-1].len;
                }

                strcat(defineStr, wordVector[i].data);
                defineStrLen += wordVector[i].len;
            }
        }

        printf("Line %d is a constant define: %s, value is %s, len is %d\n", curLine,
            wordVector[index+3].data, defineStr, defineStrLen);
    }
    else{
        printf("Line %d is a function define: %s\n", curLine, wordVector[index+3].data);
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
