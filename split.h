#ifndef _SPLIT_H_
#define _SPLIT_H_

#include <vector>

using namespace std;

enum Meta_Type{
    TYPE_SPACE,
    TYPE_WORD,
    TYPE_SPECIAL,
};

typedef union{
 char * str;
 char   chr[4];
}Meta_Data;

struct Meta_Struct{
    Meta_Struct(char * _str, int _len, int _type, int _line, int _pos){
        data.str = strdup(_str);
        len      = _len;
        type     = _type;
        line     = _line;
        pos      = _pos;
    }

    Meta_Struct(char chr0, int _type, int _line, int _pos){
        _Meta_Struct(chr0, '\0', '\0', '\0', 1, _type, _line, _pos);
    }
    // Meta_Struct(char chr0, char chr1, int _type, int _line, int _pos){
    //     _Meta_Struct(chr0, chr1, '\0', '\0', 2, _type, _line, _pos);
    // }
    // Meta_Struct(char chr0, char chr1, char chr2,
    //     int _type, int _line, int _pos){
    //     _Meta_Struct(chr0, chr1, chr2, '\0', 3, _type, _line, _pos);
    // }
    void _Meta_Struct(char chr0, char chr1, char chr2, char chr3,
        int _len, int _type, int _line, int _pos){
        data.chr[0] = chr0;
        data.chr[1] = chr1;
        data.chr[2] = chr2;
        data.chr[3] = chr3;
        len         = _len;
        type        = _type;
        line        = _line;
        pos         = _pos;
    }

    Meta_Data data;
    int    len;
    int    type;
    int    line;
    int    pos;
};

typedef vector<Meta_Struct> Meta_Vector;

void split(const char * data, int datalen, int line, Meta_Vector & wordVector);

const char * getTypeName(int type);

#endif // _SPLIT_H_
