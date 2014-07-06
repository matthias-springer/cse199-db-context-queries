#include "top_k_tf_column_db_query_benchmark.h"
#include <unordered_set>

namespace benchmark
{
    
    void run_docs_in_context_column_db_query()
    {
        int terms_size[11] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
        top_k_tf_column_db_query::generate_random_tuples();
        
        for (int i = 0; i < 11; ++i)
        {
            int r = 0;
            
            for (r = 0; r < 50; ++r)
            {
                show_info("Running top_k_in_documents_tf with number of terms " << terms_size[i] << ".");
                vector<DOMAIN_TYPE> *terms = new vector<DOMAIN_TYPE>;
                
                for (int j = 0; j < terms_size[i]; ++j)
                {
                    terms->push_back(rand() % input::b_MAX_TERM);
                }
                
                unordered_set<DOMAIN_TYPE> *documents = top_k_tf_column_db_query::documents_in_context(terms);
                
                delete documents;
                delete terms;
            }
            
            debug("Statistics for " << r << " runs.");
            output::show_stats();
            output::clear_stats();
        }
    }
    
    void run_top_k_tf_column_db_query()
    {
        int terms_size[11] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
        top_k_tf_column_db_query::generate_random_tuples();
        
        for (int i = 0; i < 11; ++i)
        {
            int r = 0;
            
            for (r = 0; r < 50; ++r)
            {
                show_info("Running top_k_in_documents_tf with number of terms " << terms_size[i] << ".");
                vector<DOMAIN_TYPE> *terms = new vector<DOMAIN_TYPE>;
                
                for (int j = 0; j < terms_size[i]; ++j)
                {
                    terms->push_back(rand() % input::b_MAX_TERM);
                }
                
                vector<DOMAIN_TYPE> *result = top_k_tf_column_db_query::top_k_tf_in_context(terms, input::b_K);
                delete terms;
            }
            
            debug("Statistics for " << r << " runs.");
            output::show_stats();
            output::clear_stats();
        }
    }
    
    void run_top_k_tf_in_docs_column_db_query()
    {
        int docs_size[8] = {1, 10, 100, 1000, 10000, 50000, 100000, 1000000};
        top_k_tf_column_db_query::generate_random_tuples();
        
        for (int i = 0; i < 8; ++i)
        {
            show_info("Running run_top_k_tf_in_docs_column_db_query with number of documents " << docs_size[i] << ".");
            unordered_set<DOMAIN_TYPE> *documents = new unordered_set<DOMAIN_TYPE>();
            
            for (int j = 0; j < docs_size[i]; ++j)
            {
                documents->insert(rand() % input::b_MAX_DOCUMENT);
            }
            
            vector<DOMAIN_TYPE> *result = top_k_tf_column_db_query::top_k_tf_in_documents(documents, input::b_K);
            delete documents;
            
            output::show_stats();
            output::clear_stats();
        }
    }
}