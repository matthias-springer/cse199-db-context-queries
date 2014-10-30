#include "top_k_tf_column_db_query.h"
#include "pubmed.h"
#include "output.h"
#include "input.h"
#include <algorithm>
#include <random>
#include <unordered_map>
#include <stdio.h>
#include <string.h>
#include "map_aggregation.h"
#include "huffman.h"
#include <bitvector.h>
#include <ibis.h>
#include <map>
#include <pthread.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

using namespace std;

unsigned short *c_term;
unsigned int *c_doc;
unsigned char *c_freq;

#define S_CLUSTERED 1
#define S_UNOPTIMIZED 0
#define S_BINARY_SEARCH 2

#define TUPLES_DIVIDER 1

int sorted_by_term = S_BINARY_SEARCH;
int sorted_by_doc = S_UNOPTIMIZED;

long *term_offsets;
long *doc_offsets;

namespace top_k_tf_column_db_query
{
    void generate_random_tuples()
    {
        debug("Generating " << input::NUM_TUPLES / TUPLES_DIVIDER << " random tuples.");
        
        output::start_timer("run/top_k_column_db_tf_in_documents_generate_random");
        
        if (TUPLES_DIVIDER > 1)
        {
            show_info("Running benchmark with " << input::NUM_TUPLES/TUPLES_DIVIDER << " instead of " << input::NUM_TUPLES << " tuples.");
        }

        c_term = new unsigned short[input::NUM_TUPLES/TUPLES_DIVIDER];
        debug("c_term alloc success");
        c_doc = new unsigned int[input::NUM_TUPLES/TUPLES_DIVIDER];
        debug("c_doc alloc success");
        c_freq = new unsigned char[input::NUM_TUPLES/TUPLES_DIVIDER];
        debug("c_freq alloc success");
        
        // offsets define where cluster of same items begins 
        term_offsets = new long[input::T_PM + 1];
        for (DOMAIN_TYPE i = 0; i < input::T_PM + 1; ++i) term_offsets[i] = 20000000000L;
        
        doc_offsets = new long[input::D_PM + 1];
        for (DOMAIN_TYPE i = 0; i < input::D_PM + 1; ++i) doc_offsets[i] = 200000000000L;

        int next_index = 0;
        for (long term = 0; term < input::T_PM; ++term)
        {
            long times = MAX(1, pubmed::get_group_by_term(term) / TUPLES_DIVIDER);
            term_offsets[term] = next_index;
            for (int i = 0; i < times; ++i)
            {
                if (next_index < input::NUM_TUPLES / TUPLES_DIVIDER)
                {
                    c_freq[next_index] = rand() % input::b_MAX_FREQUENCY;
                    c_term[next_index++] = term;
                }
            }
            
        }

        if (sorted_by_term == S_UNOPTIMIZED)
        {
            shuffle(c_term, c_term + input::NUM_TUPLES / TUPLES_DIVIDER, default_random_engine(42));
        }
        debug("Done generating terms and frequencies.");

        next_index = 0;
        for (long doc = 0; doc < input::D_PM; ++doc)
        {
            long times = MAX(1, pubmed::get_group_by_doc(doc) / TUPLES_DIVIDER);
            doc_offsets[doc] = next_index;
            for (int i = 0; i < times; ++i)
            {
                if (next_index < input::NUM_TUPLES / TUPLES_DIVIDER)
                {
                    c_doc[next_index++] = doc;
                }
            }
        }

        if (sorted_by_doc == S_UNOPTIMIZED)
        {
            shuffle(c_doc, c_doc + input::NUM_TUPLES / TUPLES_DIVIDER, default_random_engine(42));
        }
        debug("Done generating documents.");

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
                
                while (c_term[index] == term && index < input::NUM_TUPLES / TUPLES_DIVIDER)
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
                long range = input::NUM_TUPLES / TUPLES_DIVIDER;
                
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
                
                while (c_term[index] <= term && index < input::NUM_TUPLES / TUPLES_DIVIDER)
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
            for (int row = 0; row < input::NUM_TUPLES / TUPLES_DIVIDER; ++row)
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
        
        delete temp_sets;
        output::stop_timer("run/documents_in_context_column_db");
        
        return result;
    }
    
    vector<DOMAIN_TYPE> *top_k_tf_in_documents(unordered_set<DOMAIN_TYPE> *documents, int k)
    {
        debug("Running top_k_tf_in_documents.");
        debug("Context size is " << documents->size() << ".");
        
        output::start_timer("run/top_k_column_db_tf_in_documents");
        
        aggregation *aggr = new map_aggregation();
        long read_tuples = 0;
        
        if (sorted_by_doc == S_CLUSTERED)
        {
            for (auto doc_iter = documents->begin(); documents->end() != doc_iter; ++doc_iter)
            {
                long index = doc_offsets[*doc_iter];
                
                while (c_doc[index] == *doc_iter && index < input::NUM_TUPLES / TUPLES_DIVIDER)
                {
                    aggr->add(c_term[index], c_freq[index]);
                    index++;
                    read_tuples++;
                }
            }
            
            debug("Read " << read_tuples << " (clustered).");
        }
        else if (sorted_by_doc == S_BINARY_SEARCH)
        {
            for (auto doc_iter = documents->begin(); documents->end() != doc_iter; ++doc_iter)
            {
                // search for doc - 1
                long index = 0;
                long range = input::NUM_TUPLES / TUPLES_DIVIDER;
                
                while (range > 0)
                {
                    read_tuples++;
                    
                    if (c_doc[index + range / 2] < *doc_iter - 1)
                    {
                        index += range / 2;
                        range /= 2;
                    }
                    else if (c_doc[index + range / 2] == *doc_iter - 1)
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
                
                while (c_doc[index] <= *doc_iter && index < input::NUM_TUPLES / TUPLES_DIVIDER)
                {
                    if (c_doc[index] == *doc_iter)
                    {
                        aggr->add(c_term[index], c_freq[index]);
                    }
                    
                    index++;
                    read_tuples++;
                }
            }
            
            debug("Read " << read_tuples << " (binary search).");
        }
        else
        {
            for (int row = 0; row < input::NUM_TUPLES / TUPLES_DIVIDER; ++row)
            {
                if (documents->end() != documents->find(c_doc[row]))
                {
                    // this document is in the list
                    aggr->add(c_term[row], c_freq[row]);
                }
            }
        }
        
        debug("Printing top-" << k << " documents with term frequency:");
        
        output::start_timer("run/top_k_column_db_tf_in_documents_extract_top_k");
        vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *top_k = aggr->top_k(k);
        output::stop_timer("run/top_k_column_db_tf_in_documents_extract_top_k");
        
        for (vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>>::iterator iter = top_k->begin(); iter != top_k->end(); ++iter)
        {
            debug("  [" << (*iter).first << "]  " << (*iter).second);
        }
        
        output::stop_timer("run/top_k_tf_in_documents_complete");
        output::stop_timer("run/top_k_column_db_tf_in_documents");
        
        delete top_k;
        delete aggr;
        
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
