#include "count_context_benchmark.h"
#include "storage.h"
#include "output.h"
#include "input.h"
#include "count_context_query.h"

#define NUM_TERMS               24999
#define DOCUMENTS_PER_TERM      50
#define MAX_DOCUMENT            2000000

namespace benchmark
{
    void run_count_context()
    {
        show_info("Running benchmark for counting documents in context.");
        show_info("NUM_TERMS=" << NUM_TERMS << ", DOCUMENTS_PER_TERM=" << DOCUMENTS_PER_TERM << ", MAX_DOCUMENT=" << MAX_DOCUMENT);
        
        if (!input::no_generate_benchmark_data)
        {
            show_info("Generating term files.");
            
            for (int i = 0; i < NUM_TERMS; ++i)
            {
                storage *s = storage::new_instance();
                pair<long, long> position;
                
                for (int j = 0; j < DOCUMENTS_PER_TERM; ++j)
                {
                    s->add(rand() % MAX_DOCUMENT);
                }
                
                s->save_to_file("term", position);
                offset::set_offsets_for("term", i, position.first, position.second);
                
                delete s;
                if (i % (NUM_TERMS/1000) == 0) debug_n("  " << i*100.0/NUM_TERMS << " % complete.    ");
            }
            debug_n("  " << 100 << " % complete.    \n")
            
            offset::close_offset_files();
            
            output::show_stats();
            output::clear_stats();
        }
        
        int context_size[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};
        for (int i = 0; i < 7; ++i)
        {
            show_info("Running count_context_query with context size " << context_size[i] << ".");
            vector<DOMAIN_TYPE> *context = new vector<DOMAIN_TYPE>;
            
            for (int j = 0; j < context_size[i]; ++j)
            {
                context->push_back(rand() % NUM_TERMS);
            }
            
            vector<DOMAIN_TYPE> *result = count_context_query::documents_in_context(context);
            delete result;
            delete context;
            
            output::show_stats();
            output::clear_stats();
        }
    }
}