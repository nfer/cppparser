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
    Meta_Data(char * _data, int _len, int _type, int _line, int _pos){
        data = _data;
        len  = _len;
        type = _type;
        line = _line;
        pos  = _pos;
    }
    char * data;
    int    len;
    int    type;
    int    line;
    int    pos;
};

typedef vector<Meta_Data> Meta_Vector;

void split(const char * data, int datalen, int line, Meta_Vector & wordVector);

#endif // _SPLIT_H_
