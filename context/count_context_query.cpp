#include "count_context_query.h"
#include "input.h"
#include "bit_storage.h"

namespace count_context_query
{
    long size_of_context(vector<DOMAIN_TYPE> *context)
    {
        return documents_in_context(context)->size();
    }
    
    vector<DOMAIN_TYPE> *documents_in_context(vector<DOMAIN_TYPE> *context)
    {
        show_info("Reading terms file as storage type bit vector.");
        show_info("Context size is " << context->size() << ".");
        
        storage *s;
        
        if (input::omit_io)
        {
            s = new bit_storage();
            s->generate_randomly(input::b_DOCUMENTS_PER_TERM, input::b_MAX_DOCUMENT);
        }
        else
        {
            s = storage::load("term", context->at(0), STORAGE_TYPE_BITVECTOR);
        }
        
        for (int i = 1; i < context->size(); ++i)
        {
            storage *next_s;
            
            if (input::omit_io)
            {
                next_s = new bit_storage();
                next_s->generate_randomly(input::b_DOCUMENTS_PER_TERM, input::b_MAX_DOCUMENT);
            }
            else
            {
                next_s = storage::load("term", context->at(i));
            }
            
            s->intersect(*next_s);
            delete next_s;
        }
        
        show_info("Context contains " << s->count() << " documents.");
        
        vector<DOMAIN_TYPE> *result = s->elements();
        delete s;
        
        return result;
    }
    
    void stdin_documents_in_context()
    {
        show_info("Running query documents_in_context. Waiting for context size.");
        DOMAIN_TYPE count = input::read_value();
        
        show_info("Waiting for " << count << " terms.");
        vector<DOMAIN_TYPE> context;
        
        for (int i = 0; i < count; ++i)
        {
            context.push_back(input::read_value());
        }
        
        vector<DOMAIN_TYPE> *result = documents_in_context(&context);
        delete result;
    }
}