#ifndef __context__input__
#define __context__input__

#include <iostream>
#include "context.h"

namespace input
{
    extern FILE *input_file;
    extern int storage_type;
    extern int debug;
    extern bool stats_visible;
    
    void store(string name);
    void store(string name, DOMAIN_TYPE id);
    DOMAIN_TYPE read_value();
}

#endif