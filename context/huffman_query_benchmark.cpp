#include "huffman_query_benchmark.h"
#include "pubmed.h"
#include "output.h"
#include "input.h"
#include <algorithm>
#include <random>
#include <unordered_map>
#include <stdio.h>
#include <string.h>
#include "map_aggregation.h"
#include "huffman.h"

using namespace std;

namespace benchmark
{
    unsigned short* tuples;
    unordered_map<long, unsigned short*> terms_per_doc;
    unordered_map<long, char*> terms_per_doc_compressed;
    unordered_map<long, unsigned short> terms_per_doc_size;
    Node* tree;
    
    void huffman_query_generate_lists()
    {
        output::start_timer("run/bench_huffman_query_generate");
        
        // generate tuples
        show_info("[1] Generating tuples...");
        show_info("Allocating " << pubmed::tuples << " shorts (array).");
        tuples = new unsigned short[pubmed::tuples];
        show_info("Alloc success");
        
        long next_index = 0;
        
        for (short t = 0; t < T_PM; ++t)
        {
            for (long c = 0; c < pubmed::get_group_by_term(t); ++c)
            {
                tuples[next_index++] = t;
            }
        }
        
        // shuffle
        show_info("[2] Shuffling...");
        shuffle(tuples, tuples + pubmed::tuples, default_random_engine(42));
        
        // generate terms per doc
        show_info("[3] Generating separate lists...");
        next_index = 0;
        for (long d = 0; d < D_PM; ++d)
        {
            unsigned short* list = new unsigned short[pubmed::get_group_by_doc(d)];
            memcpy(list, tuples + next_index, pubmed::get_group_by_doc(d) * sizeof(unsigned short));
            terms_per_doc[d] = list;
            terms_per_doc_size[d] = pubmed::get_group_by_doc(d);
        }
        
        // compress
        show_info("[4] Generating Huffman tree...");
        tree = generate_tree(tuples, pubmed::tuples);
        generate_inverse_mapping(tree);
        
        show_info("[5] Compressing...");
        for (long d = 0; d < D_PM; ++d)
        {
            char* terms_compressed;
            encode(terms_per_doc[d], terms_per_doc_size[d], terms_compressed);
            terms_per_doc_compressed[d] = terms_compressed;
            delete terms_per_doc[d];
            
            if (d % (D_PM/1000) == 0) debug_n("  " << d*100.0/D_PM << " % complete.    ");
        }
        
        debug_n("  " << 100 << " % complete.    \n");
                                              
        output::stop_timer("run/bench_huffman_query_generate");
        
        show_info("Done.");
    }
    
    void huffman_query_run_benchmark()
    {
        int num_docs[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};
        
        for (int i = 0; i < 7; ++i)
        {
            show_info("Running benchmark for " << num_docs[i] << " documents.");
            output::start_timer("run/bench_huffman_query");
            
            map_aggregation term_counter;
            //unordered_map<unsigned short, long> term_counter;
            
            for (int di = 0; di < num_docs[i]; ++di)
            {
                // retrieve list and add to term counter
                long doc_id = rand() % D_PM;
                
                //unsigned short* terms = terms_per_doc[doc_id];
                unsigned short list_size = terms_per_doc_size[doc_id];
                unsigned short* terms_decompressed = new unsigned short[list_size];
                decode(terms_per_doc_compressed[doc_id], list_size, terms_decompressed, tree);
                
                for (int l = 0; l < list_size; ++l)
                {
                    term_counter.add(terms_decompressed[l], 1);
                    //term_counter[terms[l]]++;
                }
                
                delete terms_decompressed;
            }
            
            // sort list and extract top-k
            term_counter.top_k(5);
            
            // stats
            output::stop_timer("run/bench_huffman_query");
            output::show_stats();
            output::clear_stats();
        }
        
    }
}
