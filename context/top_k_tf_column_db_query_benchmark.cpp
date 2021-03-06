#include "top_k_tf_column_db_query_benchmark.h"
#include <unordered_set>

namespace benchmark
{
    
    void run_docs_in_context_column_db_query()
    {
        int terms_size[9] = {5, 10, 50, 100, 250, 500, 750, 1000, 1500};
        top_k_tf_column_db_query::generate_random_tuples();
        
        for (int i = 0; i < 9; ++i)
        {
            int r = 0;
            
            for (r = 0; r < 15; ++r)
            {
                show_info("Running top_k_in_documents_tf with number of terms " << terms_size[i] << ".");
                vector<DOMAIN_TYPE> *terms = new vector<DOMAIN_TYPE>;
                
                for (int j = 0; j < terms_size[i]; ++j)
                {
                    terms->push_back(rand() % input::T_PM);
                }
                
                unordered_set<DOMAIN_TYPE> *documents = top_k_tf_column_db_query::documents_in_context(terms);
                
                delete documents;
                delete terms;
            }
            
            show_info("Statistics for " << r << " runs.");
            output::show_stats();
            output::clear_stats();
        }
    }
    
    void run_top_k_tf_column_db_query()
    {
        int terms_size[9] = {5, 10, 50, 100, 250, 500, 750, 1000, 1500};
        top_k_tf_column_db_query::generate_random_tuples();
        
        for (int i = 0; i < 9; ++i)
        {
            int r = 0;
            
            show_info("Running top_k_in_documents_tf with number of terms " << terms_size[i] << ".");
            for (r = 0; r < 15; ++r)
            {
                vector<DOMAIN_TYPE> *terms = new vector<DOMAIN_TYPE>;
                
                for (int j = 0; j < terms_size[i]; ++j)
                {
                    terms->push_back(rand() % input::T_PM);
                }
                
                vector<DOMAIN_TYPE> *result = top_k_tf_column_db_query::top_k_tf_in_context(terms, input::b_K);
                delete terms;
            }
            
            show_info("Statistics for " << r << " runs.");
            output::show_stats();
            output::clear_stats();
        }
    }
    
    void run_top_k_tf_in_docs_column_db_query()
    {
        int docs_size[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};
        top_k_tf_column_db_query::generate_random_tuples();
        
        for (int i = 0; i < 7; ++i)
        {
            show_info("Running run_top_k_tf_in_docs_column_db_query with number of documents " << docs_size[i] << ".");
            int r = 0;
            for (r = 0; r < 15; ++r)
            {
                unordered_set<DOMAIN_TYPE> *documents = new unordered_set<DOMAIN_TYPE>();
                
                for (int j = 0; j < docs_size[i]; ++j)
                {
                    documents->insert(rand() % input::D_PM);
                }
                
                vector<DOMAIN_TYPE> *result = top_k_tf_column_db_query::top_k_tf_in_documents(documents, input::b_K);
                delete documents;
            }
            
            show_info("Statistics for " << r << " runs.");
            output::show_stats();
            output::clear_stats();
        }
    }
}
