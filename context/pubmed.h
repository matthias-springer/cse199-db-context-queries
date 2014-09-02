#ifndef context_pubmed_h
#define context_pubmed_h

#include <fstream>
#include "output.h"

#define STATS_FILE_T "/Users/matthias/Documents/context/pubmed/stats_terms.csv"
#define STATS_FILE_D "/Users/matthias/Documents/context/pubmed/stats_docs.csv"

// number of documents
#define D_PM 13930233

// number of terms
#define T_PM 26842

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
