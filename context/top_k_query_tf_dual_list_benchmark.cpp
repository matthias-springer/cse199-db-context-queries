#include "top_k_query_tf_dual_list_benchmark.h"
#include "input.h"
#include "output.h"
#include "list_storage.h"
#include "top_k_tf_dual_list_query.h"

namespace benchmark
{
    void run_top_k_tf_dual_list()
    {
        show_info("Running benchmark for calculating top-" << input::b_K << " terms with term frequency in documents.");
        show_info("NUM_DOCUMENTS=" << input::b_NUM_DOCUMENTS << ", TERMS_PER_DOCUMENT=" << input::b_TERMS_PER_DOCUMENT << ", MAX_TERM=" << input::b_MAX_TERM << ", K=" << input::b_K);
        
        if (!input::no_generate_benchmark_data && !input::omit_io)
        {
            show_info("Generating document files.");
            
            for (int i = 0; i < input::b_NUM_DOCUMENTS; ++i)
            {
                storage *s = new list_storage();
                storage *s_freq = new list_storage();
                pair<long, long> position;
                
                for (int j = 0; j < input::b_TERMS_PER_DOCUMENT; ++j)
                {
                    s->add(rand() % input::b_MAX_TERM);
                    s_freq->add(rand() % input::b_MAX_FREQUENCY);
                }
                
                s->save_to_file("document_tf_list1", position);
                offset::set_offsets_for("document_tf_list1", i, position.first, position.second);
                
                s_freq->save_to_file("document_tf_list2", position);
                offset::set_offsets_for("document_tf_list2", i, position.first, position.second);
                
                delete s;
                delete s_freq;
                
                if (i % (input::b_NUM_DOCUMENTS/1000) == 0) debug_n("  " << i*100.0/input::b_NUM_DOCUMENTS << " % complete.    ");
            }
            debug_n("  " << 100 << " % complete.    \n")
            
            offset::close_offset_files();
            
            output::show_stats();
            output::clear_stats();
        }
        
        if (input::omit_io)
        {
            top_k_tf_dual_list_query::generate_random_data();
        }
        
        int docs_size[8] = {1, 10, 100, 1000, 10000, 50000, 100000, 1000000};
        for (int i = 0; i < 8; ++i)
        {
            int r = 0;
            for (r = 0; r < 25; ++r)
            {
                show_info("Running top_k_tf_dual_list_in_documents_tf with number of documents " << docs_size[i] << ".");
                vector<DOMAIN_TYPE> *documents = new vector<DOMAIN_TYPE>;
                
                for (int j = 0; j < docs_size[i]; ++j)
                {
                    documents->push_back(rand() % input::b_NUM_DOCUMENTS);
                }
                
                vector<DOMAIN_TYPE> *result = top_k_tf_dual_list_query::top_k_tf_in_documents(documents, input::b_K);
                delete result;
                delete documents;
            }
            
            debug("Statistics for " << r << " runs.");
            
            output::show_stats();
            output::clear_stats();
        }
    }
}