#include "top_k_query.h"
#include "input.h"
#include "map_aggregation.h"
#include "array_aggregration.h"
#include "count_context_query.h"
#include "list_storage.h"

namespace top_k_query
{
    vector<DOMAIN_TYPE> *top_k_in_context(vector<DOMAIN_TYPE> *context, int k)
    {
        vector<DOMAIN_TYPE> *documents = count_context_query::documents_in_context(context);
        return top_k_in_documents(documents, k);
    }
    
    vector<DOMAIN_TYPE> *top_k_in_documents(vector<DOMAIN_TYPE> *documents, int k)
    {
        show_info("Number of documents is " << documents->size() << ".");
        
        output::start_timer("run/top_k_in_documents_complete");
        output::start_timer("run/top_k_in_documents_aggregation_and_load");
        
        aggregation *aggr = new map_aggregation();
        //aggregation *aggr = new array_aggregation(50001);
        
        for (vector<DOMAIN_TYPE>::iterator iter = documents->begin(); iter != documents->end(); ++iter)
        {
            storage *s;
            
            output::start_timer("run/top_k_in_documents_load");
            
            if (input::omit_io)
            {
                s = new list_storage();
                s->generate_randomly(input::b_TERMS_PER_DOCUMENT, input::T_PM);
            }
            else
            {
                s = storage::load("document", *iter, STORAGE_TYPE_LIST);
            }
            
            output::stop_timer("run/top_k_in_documents_load");
            
            vector<DOMAIN_TYPE> *terms = s->elements();
            
            for (vector<DOMAIN_TYPE>::iterator term_iter = terms->begin(); term_iter != terms->end(); ++term_iter)
            {
                aggr->add(*term_iter, 1);
            }
        }
        
        output::stop_timer("run/top_k_in_documents_aggregation_and_load");
        
        show_info("Printing top-" << k << " documents:");
        
        output::start_timer("run/top_k_in_documents_extract_top_k");
        vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *top_k = aggr->top_k(k);
        output::stop_timer("run/top_k_in_documents_extract_top_k");
        
        for (vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>>::iterator iter = top_k->begin(); iter != top_k->end(); ++iter)
        {
            show_info("  [" << (*iter).first << "]  " << (*iter).second);
        }
        
        output::stop_timer("run/top_k_in_documents_complete");
        
        return NULL;
    }
    
    void stdin_top_k_in_context()
    {
        show_info("Running query top_k_in_context. Waiting for context size.");
        DOMAIN_TYPE count = input::read_value();
        
        show_info("Waiting for " << count << " terms.");
        vector<DOMAIN_TYPE> context;
        
        for (int i = 0; i < count; ++i)
        {
            context.push_back(input::read_value());
        }
        
        show_info("Waiting for k.");
        top_k_in_context(&context, input::read_value());
    }
    
    void stdin_top_k_in_documents()
    {
        show_info("Running query top_k_in_documents. Waiting for number of documents.");
        DOMAIN_TYPE count = input::read_value();
        
        show_info("Waiting for " << count << " documents.");
        vector<DOMAIN_TYPE> documents;
        
        for (int i = 0; i < count; ++i)
        {
            documents.push_back(input::read_value());
        }
        
        show_info("Waiting for k.");
        top_k_in_documents(&documents, input::read_value());
    }
}
