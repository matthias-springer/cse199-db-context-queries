#include "count_context_benchmark.h"
#include "storage.h"
#include "output.h"
#include "input.h"
#include "count_context_query.h"

namespace benchmark
{
    void run_count_context()
    {
        show_info("Running benchmark for counting documents in context.");
        show_info("NUM_TERMS=" << input::b_NUM_TERMS << ", DOCUMENTS_PER_TERM=" << input::b_DOCUMENTS_PER_TERM << ", MAX_DOCUMENT=" << input::b_MAX_DOCUMENT);
        
        if (!input::no_generate_benchmark_data && !input::omit_io)
        {
            show_info("Generating term files.");
            
            for (int i = 0; i < input::b_NUM_TERMS; ++i)
            {
                storage *s = storage::new_instance();
                pair<long, long> position;
                
                for (int j = 0; j < input::b_DOCUMENTS_PER_TERM; ++j)
                {
                    s->add(rand() % input::b_MAX_DOCUMENT);
                }
                
                s->save_to_file("term", position);
                offset::set_offsets_for("term", i, position.first, position.second);
                
                delete s;
                if (i % (input::b_NUM_TERMS/1000) == 0) debug_n("  " << i*100.0/input::b_NUM_TERMS << " % complete.    ");
            }
            debug_n("  " << 100 << " % complete.    \n")
            
            offset::close_offset_files();
            
            output::show_stats();
            output::clear_stats();
        }
        
        if (input::omit_io)
        {
            count_context_query::randomly_generate_vectors();
        }
        
        int context_size[11] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
        for (int i = 0; i < 11; ++i)
        {
            int r = 0;
            
            for (r = 0; r < 500; ++r)
            {
                show_info("Running count_context_query with context size " << context_size[i] << ".");
                vector<DOMAIN_TYPE> *context = new vector<DOMAIN_TYPE>;
                
                for (int j = 0; j < context_size[i]; ++j)
                {
                    context->push_back(rand() % input::b_NUM_TERMS);
                }
                
                vector<DOMAIN_TYPE> *result = count_context_query::documents_in_context(context);
                delete result;
                delete context;
            }
            
            debug("Statistics for " << r << " repititions.");
            
            output::show_stats();
            output::clear_stats();
        }
    }
}