#ifndef context_pubmed_h
#define context_pubmed_h

#include <fstream>
#include "output.h"

namespace pubmed
{
    extern long tuples;
    
    void load_terms_per_doc();
    void load_docs_per_term();
    int get_random_group_by_term_count();
    int get_group_by_term(int term);
    short get_group_by_doc(long doc);
}

#endif
