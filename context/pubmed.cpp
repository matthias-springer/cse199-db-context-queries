#include "pubmed.h"

int* documents_per_term;

namespace pubmed
{
    void load_docs_per_term()
    {
        show_info("Loading documents per term from " << STATS_FILE_T << ".");
        
        documents_per_term = new int[T_PM];
        
        std::ifstream infile(STATS_FILE_T);
        for (int i = 0; i < T_PM; ++i)
        {
            infile >> documents_per_term[i];
        }
        
        show_info("Done loading file.");
    }
    
    int get_random_group_by_term_count()
    {
        return documents_per_term[rand() % T_PM];
    }
    
    int get_group_by_term(int term)
    {
        return documents_per_term[term];
    }
}