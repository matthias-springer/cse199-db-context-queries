#ifndef context_pubmed_h
#define context_pubmed_h

#include <fstream>
#include "output.h"

namespace pubmed
{
    extern long tuples;
    
    void load_terms_per_doc();
    void load_docs_per_term();
    void load_docs_per_author();
    void load_authors_per_doc();
    int get_random_group_by_term_count();
    int get_group_by_term(int term);
    short get_group_by_doc(long doc);
    short get_DA_group_by_doc(int doc);
    short get_DA_group_by_author(int author);
}

#endif
