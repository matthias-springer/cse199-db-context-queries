#include "top_k_query.h"
#include "input.h"
#include "map_aggregation.h"
#include "count_context_query.h"

namespace top_k_query
{
    vector<DOMAIN_TYPE> *top_k_in_context(vector<DOMAIN_TYPE> *context, int k)
    {
        vector<DOMAIN_TYPE> *documents = count_context_query::documents_in_context(context);
        return top_k_in_documents(documents, k);
    }
    
    vector<DOMAIN_TYPE> *top_k_in_documents(vector<DOMAIN_TYPE> *documents, int k)
    {
        info("Number of documents is " << documents->size() << ".");
        
        aggregation *aggr = new map_aggregation();
        
        for (vector<DOMAIN_TYPE>::iterator iter = documents->begin(); iter != documents->end(); ++iter)
        {
            storage *s = storage::load("document", *iter);
            vector<DOMAIN_TYPE> *terms = s->elements();
            
            for (vector<DOMAIN_TYPE>::iterator term_iter = terms->begin(); term_iter != terms->end(); ++term_iter)
            {
                aggr->add(*term_iter, 1);
            }
        }
        
        info("Printing top-" << k << " documents:");
        
        vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *top_k = aggr->top_k(k);
        for (vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>>::iterator iter = top_k->begin(); iter != top_k->end(); ++iter)
        {
            info("  [" << (*iter).first << "]  " << (*iter).second);
        }
        
        return NULL;
    }
    
    void stdin_top_k_in_context()
    {
        info("Running query top_k_in_context. Waiting for context size.");
        DOMAIN_TYPE count = input::read_value();
        
        info("Waiting for " << count << " terms.");
        vector<DOMAIN_TYPE> context;
        
        for (int i = 0; i < count; ++i)
        {
            context.push_back(input::read_value());
        }
        
        info("Waiting for k.");
        top_k_in_context(&context, input::read_value());
    }
    
    void stdin_top_k_in_documents()
    {
        info("Running query top_k_in_documents. Waiting for number of documents.");
        DOMAIN_TYPE count = input::read_value();
        
        info("Waiting for " << count << " documents.");
        vector<DOMAIN_TYPE> documents;
        
        for (int i = 0; i < count; ++i)
        {
            documents.push_back(input::read_value());
        }
        
        info("Waiting for k.");
        top_k_in_documents(&documents, input::read_value());
    }
}
