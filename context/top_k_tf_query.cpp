#include "top_k_tf_query.h"
#include "input.h"
#include "map_aggregation.h"
#include "count_context_query.h"
#include "map_column_storage.h"

namespace top_k_tf_query
{
    vector<DOMAIN_TYPE> *top_k_tf_in_context(vector<DOMAIN_TYPE> *context, int k)
    {
        vector<DOMAIN_TYPE> *documents = count_context_query::documents_in_context(context);
        return top_k_tf_in_documents(documents, k);
    }
    
    vector<DOMAIN_TYPE> *top_k_tf_in_documents(vector<DOMAIN_TYPE> *documents, int k)
    {
        show_info("Number of documents is " << documents->size() << ".");
        
        output::start_timer("run/top_k_tf_in_documents_complete");
        output::start_timer("run/top_k_tf_in_documents_aggregation_and_load");
        
        aggregation *aggr = new map_aggregation();
        
        for (vector<DOMAIN_TYPE>::iterator iter = documents->begin(); iter != documents->end(); ++iter)
        {
            map_column_storage<DOMAIN_TYPE> *s;
            
            output::start_timer("run/top_k_tf_in_documents_load");
            
            if (input::omit_io)
            {
                s = new map_column_storage<DOMAIN_TYPE>();
                s->generate_randomly(input::b_TERMS_PER_DOCUMENT, input::T_PM, input::b_MAX_FREQUENCY);
            }
            else
            {
                s = map_column_storage<DOMAIN_TYPE>::load("document_tf", *iter);
            }
            
            output::stop_timer("run/top_k_tf_in_documents_load");
            
            unordered_map<DOMAIN_TYPE, DOMAIN_TYPE> *terms = s->elements_with_columns();
            
            for (auto term_iter = terms->begin(); term_iter != terms->end(); ++term_iter)
            {
                aggr->add(term_iter->first, term_iter->second);
            }
        }
        
        output::stop_timer("run/top_k_tf_in_documents_aggregation_and_load");
        
        show_info("Printing top-" << k << " documents with term frequency:");
        
        output::start_timer("run/top_k_tf_in_documents_extract_top_k");
        vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *top_k = aggr->top_k(k);
        output::stop_timer("run/top_k_tf_in_documents_extract_top_k");
        
        for (vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>>::iterator iter = top_k->begin(); iter != top_k->end(); ++iter)
        {
            show_info("  [" << (*iter).first << "]  " << (*iter).second);
        }
        
        output::stop_timer("run/top_k_tf_in_documents_complete");
        return NULL;
    }
    
    void stdin_top_k_tf_in_context()
    {
        show_info("Running query top_k_tf_in_context. Waiting for context size.");
        DOMAIN_TYPE count = input::read_value();
        
        show_info("Waiting for " << count << " terms.");
        vector<DOMAIN_TYPE> context;
        
        for (int i = 0; i < count; ++i)
        {
            context.push_back(input::read_value());
        }
        
        show_info("Waiting for k.");
        top_k_tf_in_context(&context, input::read_value());
    }
    
    void stdin_top_k_tf_in_documents()
    {
        show_info("Running query top_k_tf_in_documents. Waiting for number of documents.");
        DOMAIN_TYPE count = input::read_value();
        
        show_info("Waiting for " << count << " documents.");
        vector<DOMAIN_TYPE> documents;
        
        for (int i = 0; i < count; ++i)
        {
            documents.push_back(input::read_value());
        }
        
        show_info("Waiting for k.");
        top_k_tf_in_documents(&documents, input::read_value());
    }
}
