#include "top_k_tf_column_db_query.h"
#include <map>

#define MIN(a,b) (((a)<(b))?(a):(b))

using namespace std;

DOMAIN_TYPE *c_term;
DOMAIN_TYPE *c_doc;
DOMAIN_TYPE *c_freq;

#define S_CLUSTERED 1
#define S_UNOPTIMIZED 0
#define S_BINARY_SEARCH 2

int sorted_by_term = S_CLUSTERED;
bool sorted_by_doc = false;

long *term_offsets;

namespace top_k_tf_column_db_query
{
    void generate_random_tuples()
    {
        debug("Generating " << input::b_NUM_TUPLES << " random tuples.");
        
        output::start_timer("run/top_k_column_db_tf_in_documents_generate_random");
        
        c_term = new DOMAIN_TYPE[input::b_NUM_TUPLES];
        c_doc = new DOMAIN_TYPE[input::b_NUM_TUPLES];
        c_freq = new DOMAIN_TYPE[input::b_NUM_TUPLES];
        
        if (sorted_by_term)
        {
            term_offsets = new long[input::b_MAX_TERM + 1];
            for (DOMAIN_TYPE i = 0; i < input::b_MAX_TERM + 1; ++i) term_offsets[i] = 20000000000L;
        }
        
        for (long i = 0; i < input::b_NUM_TUPLES; ++i)
        {
            if (sorted_by_term == S_UNOPTIMIZED)
            {
                // no optimization
                c_term[i] = rand() % input::b_MAX_TERM;
            }
            else
            {
                // clustered or binary search
                c_term[i] = (int) ((i * 1.0) / input::b_NUM_TUPLES * input::b_MAX_TERM);
                term_offsets[c_term[i]] = MIN(term_offsets[c_term[i]], i);
            }
            
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
        unordered_map<DOMAIN_TYPE, DOMAIN_TYPE> term_to_temp_index;
        
        for (int i = 0; i < terms->size(); ++i)
        {
            term_to_temp_index[terms->at(i)] = i;
        }
        
        long read_tuples = 0;
        
        if (sorted_by_term == S_CLUSTERED)
        {
            for (int term_index = 0; term_index < terms->size(); ++term_index)
            {
                DOMAIN_TYPE term = terms->at(term_index);
                long index = term_offsets[term];
                
                while (c_term[index] == term && index < input::b_NUM_TUPLES)
                {
                    temp_sets[term_index].insert(c_doc[index]);
                    index++;
                    read_tuples++;
                }
            }
            
            debug("Read " << read_tuples << " (clustered).");
        }
        else if (sorted_by_term == S_BINARY_SEARCH)
        {
            for (int term_index = 0; term_index < terms->size(); ++term_index)
            {
                DOMAIN_TYPE term = terms->at(term_index);
                
                // search for term - 1
                long index = 0;
                long range = input::b_NUM_TUPLES;
                
                while (range > 0)
                {
                    read_tuples++;
                    
                    if (c_term[index + range / 2] < term - 1)
                    {
                        index += range / 2;
                        range /= 2;
                    }
                    else if (c_term[index + range / 2] == term - 1)
                    {
                        index += range / 2;
                        break;
                    }
                    else
                    {
                        range /= 2;
                    }
                }

                // found starting point for binary search
                // debug("Starting row for " << term << " is " << index << ".");
                
                while (c_term[index] <= term && index < input::b_NUM_TUPLES)
                {
                    if (c_term[index] == term)
                    {
                        temp_sets[term_index].insert(c_doc[index]);
                    }
                    
                    index++;
                    read_tuples++;
                }
            }
            
            debug("Read " << read_tuples << " (binary search).");
        }
        else
        {
            for (int row = 0; row < input::b_NUM_TUPLES; ++row)
            {
                if (term_to_temp_index.end() != term_to_temp_index.find(c_term[row]))
                {
                    // found key
                    temp_sets[term_to_temp_index[c_term[row]]].insert(c_doc[row]);
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
        
        output::start_timer("run/documents_in_context_column_db_intersect");
        
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
        
        output::stop_timer("run/documents_in_context_column_db_intersect");
        
        //delete temp_sets;
        output::stop_timer("run/documents_in_context_column_db");
        
        return result;
    }
    
    vector<DOMAIN_TYPE> *top_k_tf_in_documents(unordered_set<DOMAIN_TYPE> *documents, int k)
    {
        debug("Running top_k_tf_in_documents.");
        debug("Context size is " << documents->size() << ".");
        
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