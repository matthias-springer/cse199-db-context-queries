#include "count_context_query.h"
#include "input.h"
#include "bit_storage.h"
#include "pubmed.h"

namespace count_context_query
{
    bit_storage **bit_vectors_in_ram;
    
    void randomly_generate_vectors()
    {
        debug("Generating " << input::T_PM << " bit vectors.");
        bit_vectors_in_ram = new bit_storage*[input::T_PM];
        
        for (int i = 0; i < input::T_PM; ++i)
        {
            bit_vectors_in_ram[i] = new bit_storage();
            
            debug(i);
            //double docs = pubmed::get_random_group_by_term_count();
            //docs = docs / D_PM * input::b_NUM_DOCUMENTS;
            double docs = pubmed::get_group_by_term(i);
            bit_vectors_in_ram[i]->generate_randomly((int) docs, input::D_PM);
            
            delete bit_vectors_in_ram[i];
        }
        
        debug("Done.");
        output::show_stats();
    }
    
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
            s = bit_vectors_in_ram[context->at(0)]->copy();
        }
        else
        {
            s = storage::load("term", context->at(0), STORAGE_TYPE_BITVECTOR);
        }
        
        output::start_timer("run/count_context_query");
        
        for (int i = 1; i < context->size(); ++i)
        {
            storage *next_s;
            
            if (input::omit_io)
            {
                next_s = bit_vectors_in_ram[context->at(i)];
            }
            else
            {
                next_s = storage::load("term", context->at(i));
            }
            
            s->intersect(*next_s);
            
            if (!input::omit_io)
            {
                delete next_s;
            }
        }
        
        show_info("Context contains " << s->count() << " documents.");
        
        vector<DOMAIN_TYPE> *result = s->elements();
        
        delete s;
        
        
        output::stop_timer("run/count_context_query");
        
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
