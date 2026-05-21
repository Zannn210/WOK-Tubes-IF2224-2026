#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP

#include <string>

enum ObjKind {
    OBJ_CONST   = 0,
    OBJ_VAR     = 1,
    OBJ_TYPE    = 2,
    OBJ_PROC    = 3,
    OBJ_FUNC    = 4,
    OBJ_PROGRAM = 5
};

struct TabEntry {
    std::string id; 
    int link; 
    int obj; 
    int type;
    int ref; 
    int nrm; 
    int lev; 
    int adr;
};

struct BtabEntry {
    int last; 
    int lpar; 
    int psze;  
    int vsze;  
};

struct AtabEntry {
    int xtyp;  
    int etyp;
    int eref; 
    int low;  
    int high;
    int elsz;  
    int size; 
};

#endif 
