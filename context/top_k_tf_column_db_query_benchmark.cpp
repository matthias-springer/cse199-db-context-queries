#include "top_k_tf_column_db_query_benchmark.h"

namespace benchmark
{
    void run_top_k_tf_column_db_query()
    {
        int terms_size[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};
        top_k_tf_column_db_query::generate_random_tuples();
        
        for (int i = 0; i < 7; ++i)
        {
            show_info("Running top_k_in_documents_tf with number of terms " << terms_size[i] << ".");
            vector<DOMAIN_TYPE> *terms = new vector<DOMAIN_TYPE>;
            
            for (int j = 0; j < terms_size[i]; ++j)
            {
                terms->push_back(rand() % input::b_MAX_TERM);
            }
            
            vector<DOMAIN_TYPE> *result = top_k_tf_column_db_query::top_k_tf_in_context(terms, input::b_K);
            delete terms;
            
            output::show_stats();
            output::clear_stats();
        }
    }
}