#include "pubmed.h"

int* documents_per_term;
short* terms_per_doc;

namespace pubmed
{
    long tuples;
    
    void load_docs_per_term()
    {
        show_info("Loading documents per term from " << STATS_FILE_T << ".");
        
        documents_per_term = new int[T_PM];
        tuples = 0;
        
        std::ifstream infile(STATS_FILE_T);
        for (int i = 0; i < T_PM; ++i)
        {
            infile >> documents_per_term[i];
            tuples += documents_per_term[i];
        }
        
        show_info("Done loading file.");
    }
    
    void load_terms_per_doc()
    {
        show_info("Loading terms per document from " << STATS_FILE_D << ".");
        
        terms_per_doc = new short[D_PM];
        
        std::ifstream infile(STATS_FILE_D);
        for (int i = 0; i < D_PM; ++i)
        {
            infile >> terms_per_doc[i];
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
    
    short get_group_by_doc(long doc)
    {
        return terms_per_doc[doc];
    }
}