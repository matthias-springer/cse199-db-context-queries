#ifndef __context__offset__
#define __context__offset__

#include "context.h"
#include <iostream>
#include <string.h>
#include <map>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "compile_options.h"

#define OFFSETS_FILE_SIZE sizeof(long) * 2 * 200000005 //1400000

using namespace std;

namespace offset
{
    long offset_begin(string name, DOMAIN_TYPE id);
    long offset_length(string name, DOMAIN_TYPE id);
    static long* offsets_for(string name);
    static void open_offset_file(string name);
    long next_offset_for(string name);
    void set_offsets_for(string name, DOMAIN_TYPE id, long begin, long length);
    void handle_bytes_written(string name, long length);
    
    void close_offset_files();
    
    static map<string, long*> offsets;
    static map<string, int> offset_files;
}

#endif
