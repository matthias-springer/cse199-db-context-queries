#include "pubmed.h"
#include "input.h"
#include <sys/stat.h>

int* documents_per_term;
short* terms_per_doc;

namespace pubmed
{
    void load_docs_per_term()
    {
        debug("[HISTOGRAM] Loading documents per term from " << input::STATS_FILE_T << ".");
    
        struct stat buffer;
        if (stat(input::STATS_FILE_T, &buffer) != 0)
        {
            show_info("[HISTOGRAM] WARNING: STATS_FILE_T does not exist. Cannot run PubMed queries! Run with -h for more options.");
            return;
        }

        std::ifstream infile(input::STATS_FILE_T);
        infile >> input::T_PM;
        debug("[HISTOGRAM] Reading " << input::T_PM << " terms.");

        documents_per_term = new int[input::T_PM];
        input::NUM_TUPLES = 0;

        for (int i = 0; i < input::T_PM; ++i)
        {
            infile >> documents_per_term[i];
            input::NUM_TUPLES += documents_per_term[i];
        }
        
        debug("[HISTOGRAM] Done loading file.");
        debug("[HISTOGRAM] Column DB has " << input::NUM_TUPLES << " tuples.");
    }
    
    void load_terms_per_doc()
    {
        debug("[HISTOGRAM] Loading terms per document from " << input::STATS_FILE_D << ".");
       
        struct stat buffer;
        if (stat(input::STATS_FILE_D, &buffer) != 0)
        {
            show_info("[HISTOGRAM] WARNING: STATS_FILE_D does not exist. Cannot run PubMed queries! Run with -h for more options.");
            return;
        }
 
        std::ifstream infile(input::STATS_FILE_D);
        infile >> input::D_PM;
        debug("[HISTOGRAM] Reading " << input::D_PM << " documents.");

        terms_per_doc = new short[input::D_PM];

        for (int i = 0; i < input::D_PM; ++i)
        {
            infile >> terms_per_doc[i];
        }
        
        debug("[HISTOGRAM] Done loading file.");
    }
    
    int get_random_group_by_term_count()
    {
        return documents_per_term[rand() % input::T_PM];
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
