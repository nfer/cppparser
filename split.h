#ifndef _SPLIT_H_
#define _SPLIT_H_

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

void analysis(const char * data, int datalen, int line, Meta_Vector & wordVector);

#endif // _SPLIT_H_
