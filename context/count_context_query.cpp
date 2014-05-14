#include "count_context_query.h"
#include "input.h"

namespace count_context_query
{
    long size_of_context(vector<DOMAIN_TYPE> *context)
    {
        return documents_in_context(context)->size();
    }
    
    vector<DOMAIN_TYPE> *documents_in_context(vector<DOMAIN_TYPE> *context)
    {
        info("Context size is " << context->size() << ".");
        
        storage *s = storage::load("term", context->at(0));
        
        for (int i = 1; i < context->size(); ++i)
        {
            s->intersect(*storage::load("term", context->at(i)));
        }
        
        info("Context contains " << s->count() << " documents.");
        return s->elements();
    }
    
    void stdin_documents_in_context()
    {
        info("Running query documents_in_context. Waiting for context size.");
        DOMAIN_TYPE count = input::read_value();
        
        info("Waiting for " << count << " terms.");
        vector<DOMAIN_TYPE> context;
        
        for (int i = 0; i < count; ++i)
        {
            context.push_back(input::read_value());
        }
        
        documents_in_context(&context);
    }
}