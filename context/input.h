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
    extern bool no_generate_benchmark_data;
    extern bool omit_io;
    
    extern DOMAIN_TYPE b_NUM_TERMS;
    extern DOMAIN_TYPE b_DOCUMENTS_PER_TERM;
    extern DOMAIN_TYPE b_MAX_DOCUMENT;
    extern DOMAIN_TYPE b_NUM_DOCUMENTS;
    extern DOMAIN_TYPE b_TERMS_PER_DOCUMENT;
    extern DOMAIN_TYPE b_MAX_TERM;
    extern DOMAIN_TYPE b_MAX_FREQUENCY;
    extern DOMAIN_TYPE b_K;
    extern long b_NUM_TUPLES;

    extern char* STATS_FILE_T;
    extern char* STATS_FILE_D;
    extern long T_PM;
    extern long D_PM;
    extern long NUM_TUPLES;

    void print_stats();               
    void store(string name);
    void store(string name, DOMAIN_TYPE id);
    DOMAIN_TYPE read_value();
    
    short** terms_bench_items();
    void dealloc_terms_bench_items(short** ptr);
    
    int** docs_bench_items();
    void dealloc_docs_bench_items(int** ptr);
}

#endif
