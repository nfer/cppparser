
#ifndef _ANALYSIS_H
#define _ANALYSIS_H

#include "split.h" //FIXME: it's a but on "//" pos

enum Header_Type{
    TYPE_SYSTEM,
    TYPE_USER,
};

enum Comment_Type{
    TYPE_SIGLELINE,
    TYPE_MULTILINE,
};

enum Define_Type{
    TYPE_CONSTANT,
    TYPE_FUNCTION,
};

enum Analysis_Error{
	ANALYSIS_OK = 0,

    DEFINE_FMT_ERROR = 0x1000,
};

enum Analysis_Step{
    CHECK_SPACE,
    CHECK_WORD,
    CHECK_TYPE,

    CHECK_NONE,
};

void getIncludeFiles(Meta_Vector & wordVector, size_t index);
void getDefine(Meta_Vector & wordVector, size_t index);

void analysis(Meta_Vector & wordVector);

#endif

