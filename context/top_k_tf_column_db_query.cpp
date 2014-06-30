#include "top_k_tf_column_db_query.h"

using namespace std;

DOMAIN_TYPE *c_term;
DOMAIN_TYPE *c_doc;
DOMAIN_TYPE *c_freq;

namespace top_k_tf_column_db_query
{
    void generate_random_tuples()
    {
        debug("Generating " << input::b_NUM_TUPLES << " random tuples.");
        
        output::start_timer("run/top_k_column_db_tf_in_documents_generate_random");
        
        c_term = new DOMAIN_TYPE[input::b_NUM_TUPLES];
        c_doc = new DOMAIN_TYPE[input::b_NUM_TUPLES];
        c_freq = new DOMAIN_TYPE[input::b_NUM_TUPLES];
        
        for (long i = 0; i < input::b_NUM_TUPLES; ++i)
        {
            c_term[i] = rand() % input::b_MAX_TERM;
            c_doc[i] = rand() % input::b_MAX_DOCUMENT;
            c_freq[i] = rand() % input::b_MAX_FREQUENCY;
        }
        
        output::stop_timer("run/top_k_column_db_tf_in_documents_generate_random");
    }
    
    unordered_set<DOMAIN_TYPE> *documents_in_context(vector<DOMAIN_TYPE> *terms)
    {
        debug("Running documents_in_context. Context size is " << terms->size() << ".");
        
        output::start_timer("run/documents_in_context_column_db");
        
        // fill temp sets with documents
        unordered_set<DOMAIN_TYPE> *temp_sets = new unordered_set<DOMAIN_TYPE>[terms->size()]();
        for (int row = 0; row < input::b_NUM_TUPLES; ++row)
        {
            for (int term_index = 0; term_index < terms->size(); ++term_index)
            {
                DOMAIN_TYPE term = terms->at(term_index);
                
                if (c_term[row] == term)
                {
                    temp_sets[term_index].insert(c_doc[row]);
                    break;
                }
            }
        }
        
        int smallest_set = 0;
        long smallest_set_size = temp_sets[0].size();
        
        // retrieve smallest set
        for (int s = 1; s < terms->size(); ++s)
        {
            if (temp_sets[s].size() < smallest_set_size)
            {
                smallest_set_size = temp_sets[s].size();
                smallest_set = s;
            }
        }
        
        // intersect sets
        auto it = temp_sets[smallest_set].begin();
        while (temp_sets[smallest_set].end() != it)
        {
            auto current = it++;
            for (int s = 0; s < terms->size(); ++s)
            {
                if (s != smallest_set)
                {
                    if (temp_sets[s].end() == temp_sets[s].find(*current))
                    {
                        // element not found in at least one set
                        temp_sets[smallest_set].erase(current);
                        break;
                    }
                }
            }
        }
        
        unordered_set<DOMAIN_TYPE> *result = new unordered_set<DOMAIN_TYPE>();
        for (auto it = temp_sets[smallest_set].begin(); temp_sets[smallest_set].end() != it; ++it)
        {
            result->insert(*it);
        }
        
        //delete temp_sets;
        output::stop_timer("run/documents_in_context_column_db");
        
        return result;
    }
    
    vector<DOMAIN_TYPE> *top_k_tf_in_documents(unordered_set<DOMAIN_TYPE> *documents, int k)
    {
        debug("Running top_k_tf_in_documents.");
        
        output::start_timer("run/top_k_column_db_tf_in_documents");
        
        aggregation *aggr = new map_aggregation();
        
        for (int row = 0; row < input::b_NUM_TUPLES; ++row)
        {
            if (documents->end() != documents->find(c_doc[row]))
            {
                // this document is in the list
                aggr->add(c_term[row], c_freq[row]);
            }
        }
        
        show_info("Printing top-" << k << " documents with term frequency:");
        
        output::start_timer("run/top_k_column_db_tf_in_documents_extract_top_k");
        vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *top_k = aggr->top_k(k);
        output::stop_timer("run/top_k_column_db_tf_in_documents_extract_top_k");
        
        for (vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>>::iterator iter = top_k->begin(); iter != top_k->end(); ++iter)
        {
            show_info("  [" << (*iter).first << "]  " << (*iter).second);
        }
        
        output::stop_timer("run/top_k_tf_in_documents_complete");
        output::stop_timer("run/top_k_column_db_tf_in_documents");
        
        delete top_k;
        
        return NULL;
    }
    
    vector<DOMAIN_TYPE> *top_k_tf_in_context(vector<DOMAIN_TYPE> *context, int k)
    {
        debug("Running top_k_tf_in_context.");
        
        output::start_timer("run/top_k_column_db_tf_in_context");
        
        unordered_set<DOMAIN_TYPE> *documents = documents_in_context(context);
        top_k_tf_in_documents(documents, k);
        
        delete documents;
        
        output::stop_timer("run/top_k_column_db_tf_in_context");
        
        return NULL;
    }
}