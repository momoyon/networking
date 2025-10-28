#ifndef _PC_H_
#define _PC_H_

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct Entity Entity;
typedef struct Nic Nic;

typedef struct Pc Pc;


struct Pc {
    const char *hostname;
    Nic *nic;
    // #TODO: cmd
};

#endif // _PC_H_

void make_pc(Pc *pc_out, const char *hostname, Arena *arena);
