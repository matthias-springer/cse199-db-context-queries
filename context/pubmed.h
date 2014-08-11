#ifndef context_pubmed_h
#define context_pubmed_h

#include <fstream>
#include "output.h"

#define STATS_FILE_T "/Users/matthias/Documents/context/pubmed/stats_terms.csv"

// number of documents
#define D_PM 13930233

// number of terms
#define T_PM 26842

namespace pubmed
{
    void load_docs_per_term();
    int get_random_group_by_term_count();
    int get_group_by_term(int term);
}

#endif
