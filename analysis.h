
#ifndef _ANALYSIS_H
#define _ANALYSIS_H

#include "split.h" //FIXME: it's a but on "//" pos

enum Header_Type{
    TYPE_SYSTEM,
    TYPE_USER,
};

void getIncludeFiles(Meta_Vector & wordVector, size_t index);

void analysis(Meta_Vector & wordVector);

#endif

