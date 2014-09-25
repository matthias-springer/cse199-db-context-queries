#include "top_k_query_benchmark.h"
#include "input.h"
#include "output.h"
#include "list_storage.h"
#include "top_k_query.h"

namespace benchmark
{
    void run_top_k()
    {
        show_info("Running benchmark for calculating top-5 terms in documents.");
        show_info("NUM_DOCUMENTS=" << input::D_PM << ", TERMS_PER_DOCUMENT=" << input::b_TERMS_PER_DOCUMENT << ", T_PM=" << input::T_PM);
        
        if (!input::no_generate_benchmark_data && !input::omit_io)
        {
            show_info("Generating document files.");
            
            for (int i = 0; i < input::D_PM; ++i)
            {
                storage *s = new list_storage();
                pair<long, long> position;
                
                for (int j = 0; j < input::b_TERMS_PER_DOCUMENT; ++j)
                {
                    s->add(rand() % input::T_PM);
                }
                
                s->save_to_file("document", position);
                offset::set_offsets_for("document", i, position.first, position.second);
                
                delete s;
                if (i % (input::D_PM/1000) == 0) debug_n("  " << i*100.0/input::D_PM << " % complete.    ");
            }
            debug_n("  " << 100 << " % complete.    \n")
            
            offset::close_offset_files();
            
            output::show_stats();
            output::clear_stats();
        }
        
        int docs_size[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};
        for (int i = 0; i < 7; ++i)
        {
            show_info("Running top_k_in_documents with number of documents " << docs_size[i] << ".");
            vector<DOMAIN_TYPE> *documents = new vector<DOMAIN_TYPE>;
            
            for (int j = 0; j < docs_size[i]; ++j)
            {
                documents->push_back(rand() % input::D_PM);
            }
            
            vector<DOMAIN_TYPE> *result = top_k_query::top_k_in_documents(documents, 5);
            delete result;
            delete documents;
            
            output::show_stats();
            output::clear_stats();
        }
    }
}
