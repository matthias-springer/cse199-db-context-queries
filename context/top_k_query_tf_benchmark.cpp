#include "top_k_query_tf_benchmark.h"
#include "input.h"
#include "output.h"
#include "map_column_storage.h"
#include "top_k_tf_query.h"

#define NUM_DOCUMENTS           10000 //1399999
#define TERMS_PER_DOCUMENT      100
#define MAX_TERM                50000
#define MAX_FREQUENCY           40
#define K                       5

namespace benchmark
{
    void run_top_k_tf()
    {
        show_info("Running benchmark for calculating top-" << K << " terms with term frequency in documents.");
        show_info("NUM_DOCUMENTS=" << NUM_DOCUMENTS << ", TERMS_PER_DOCUMENT=" << TERMS_PER_DOCUMENT << ", MAX_TERM=" << MAX_TERM << ", K=" << K);
        
        if (!input::no_generate_benchmark_data)
        {
            show_info("Generating document files.");
            
            for (int i = 0; i < NUM_DOCUMENTS; ++i)
            {
                map_column_storage<int> *s = new map_column_storage<int>();
                pair<long, long> position;
                
                for (int j = 0; j < TERMS_PER_DOCUMENT; ++j)
                {
                    s->add(rand() % MAX_TERM, rand() % MAX_FREQUENCY);
                }
                
                s->save_to_file("document_tf", position);
                offset::set_offsets_for("document_tf", i, position.first, position.second);
                
                delete s;
                if (i % (NUM_DOCUMENTS/1000) == 0) debug_n("  " << i*100.0/NUM_DOCUMENTS << " % complete.    ");
            }
            debug_n("  " << 100 << " % complete.    \n")
            
            offset::close_offset_files();
            
            output::show_stats();
            output::clear_stats();
        }
        
        int docs_size[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};
        for (int i = 0; i < 7; ++i)
        {
            show_info("Running top_k_in_documents_tf with number of documents " << docs_size[i] << ".");
            vector<DOMAIN_TYPE> *documents = new vector<DOMAIN_TYPE>;
            
            for (int j = 0; j < docs_size[i]; ++j)
            {
                documents->push_back(rand() % NUM_DOCUMENTS);
            }
            
            vector<DOMAIN_TYPE> *result = top_k_tf_query::top_k_tf_in_documents(documents, K);
            delete result;
            delete documents;
            
            output::show_stats();
            output::clear_stats();
        }
    }
}