#include "top_k_tf_dual_list_query.h"
#include "input.h"
#include "map_aggregation.h"
#include "array_aggregration.h"
#include "count_context_query.h"
#include "list_storage.h"

namespace top_k_tf_dual_list_query
{
    vector<DOMAIN_TYPE> *top_k_tf_in_context(vector<DOMAIN_TYPE> *context, int k)
    {
        vector<DOMAIN_TYPE> *documents = count_context_query::documents_in_context(context);
        return top_k_tf_in_documents(documents, k);
    }
    
    vector<DOMAIN_TYPE> *top_k_tf_in_documents(vector<DOMAIN_TYPE> *documents, int k)
    {
        show_info("Number of documents is " << documents->size() << ".");
        
        output::start_timer("run/top_k_tf_dual_list_in_documents_complete");
        output::start_timer("run/ttop_k_tf_dual_list_in_documents_aggregation_and_load");
        
        //aggregation *aggr = new map_aggregation();
        aggregation *aggr = new array_aggregation(input::b_MAX_TERM + 1);
        
        for (vector<DOMAIN_TYPE>::iterator iter = documents->begin(); iter != documents->end(); ++iter)
        {
            storage *s, *s_counts;
            
            output::start_timer("run/top_k_tf_dual_list_in_documents_load");
            
            if (input::omit_io)
            {
                s = new list_storage();
                s->generate_randomly(input::b_TERMS_PER_DOCUMENT, input::b_MAX_TERM);
            }
            else
            {
                s = storage::load("document_tf_list1", *iter, STORAGE_TYPE_LIST);
            }
            
            if (input::omit_io)
            {
                s_counts = new list_storage();
                s_counts->generate_randomly(input::b_TERMS_PER_DOCUMENT, input::b_MAX_FREQUENCY);
            }
            else
            {
                s = storage::load("document_tf_list2", *iter, STORAGE_TYPE_LIST);
            }
            
            output::stop_timer("run/top_k_tf_dual_list_in_documents_load");
            
            vector<DOMAIN_TYPE> *terms = s->elements();
            vector<DOMAIN_TYPE> *freqs = s_counts->elements();
            
            auto freq_iter = freqs->begin();
            
            for (auto term_iter = terms->begin(); term_iter != terms->end(); ++term_iter)
            {
                aggr->add(*term_iter, *(freq_iter++));
            }
        }
        
        output::stop_timer("run/top_k_tf_dual_list_in_documents_aggregation_and_load");
        
        show_info("Printing top-" << k << " documents:");
        
        output::start_timer("run/top_k_tf_dual_list_in_documents_extract_top_k");
        vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *top_k = aggr->top_k(k);
        output::stop_timer("run/top_k_tf_dual_list_in_documents_extract_top_k");
        
        for (vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>>::iterator iter = top_k->begin(); iter != top_k->end(); ++iter)
        {
            show_info("  [" << (*iter).first << "]  " << (*iter).second);
        }
        
        output::stop_timer("run/top_k_tf_dual_list_in_documents_complete");
        
        return NULL;
    }
}
