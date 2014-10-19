#include "final_phase1.h"
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

namespace benchmark
{
    unsigned int* column_doc;
    unsigned short* column_term;
    
    ibis::bitvector** bit_vector_for_term;
    
    void generate_bit_vectors()
    {
        show_info("[1] Generating tuples...");
        column_doc = new unsigned int[input::NUM_TUPLES];
        column_term = new unsigned short[input::NUM_TUPLES];
        bit_vector_for_term = new ibis::bitvector*[input::T_PM];
        
        long next_index = 0;
        
        for (unsigned int t = 0; t < input::D_PM; ++t)
        {
            for (long c = 0; c < pubmed::get_group_by_doc(t); ++c)
            {
                column_doc[next_index++] = t;
            }
            
            if (t % (input::D_PM/1000) == 0) debug_n("  " << t*100.0/input::D_PM << " % complete.    ");
        }
        debug_n("  " << 100 << " % complete.    \n");
        debug("Generated " << next_index << " tuples.");
        
        // shuffle
        show_info("[2] Shuffling...");
        shuffle(column_doc, column_doc + input::NUM_TUPLES, default_random_engine(42));
        
        show_info("[3] Generating bit vectors...");
        unsigned long docs_compressed_bytes = 0;
        
        for (int term = 0; term < input::T_PM; ++term)
        {
            ibis::bitvector* bit_vector = new ibis::bitvector();
            
            for (int i = 0; i < pubmed::get_group_by_term(term); ++i)
            {
                bit_vector->setBit(column_doc[next_index++], 1);
            }
            
            bit_vector->compress();
            docs_compressed_bytes += bit_vector->bytes();
            
            // force new allocate
            ibis::array_t<uint32_t>* arr = new ibis::array_t<uint32_t>();
            bit_vector->write(*arr);
            delete bit_vector;
            bit_vector = new ibis::bitvector(*arr);
            delete arr;
            
            bit_vector_for_term[term] = bit_vector;
            
            if (term % (input::T_PM/1000) == 0) debug_n("  " << term*100.0/input::T_PM << " % complete.    ");
        }
        
        debug_n("  " << 100 << " % complete.    \n");
        show_info("size of compressed bit vectors (bytes): " << docs_compressed_bytes << ".");
    }
    
    void run_phase1_bench_final()
    {
        int num_terms[6] = {5, 10, 100, 1000, 10000, 25000};
        long mem_counter = 0;   // avoid optimizations
        
        for (int i = 0; i < 6; ++i)
        {
            int cnt_terms = num_terms[i];
            
            show_info("Running for " << cnt_terms << " terms using 300 repititions.");
            output::start_timer("run/phase1_final");
            for (int r = 0; r < 300; ++r)
            {
                ibis::bitvector* base_vector = new ibis::bitvector(*bit_vector_for_term[rand() % input::T_PM]);
                
                for (int a = 0; a < cnt_terms - 1; ++a)
                {
                    *base_vector &= *bit_vector_for_term[rand() % input::T_PM];
                }
                
                // extract ones
                ibis::bitvector::indexSet ones = base_vector->firstIndexSet();
                bool is_range = ones.isRange();
                
                while (ones.nIndices() != 0)
                {
                    for (int i = 0; i < ones.nIndices(); ++i)
                    {
                        if (is_range)
                        {
                            mem_counter += ones.indices()[0] + i;
                        }
                        else
                        {
                            mem_counter += ones.indices()[i];
                        }
                    }
                    
                    ++ones;
                }
                
            }
            
            output::stop_timer("run/phase1_final");
            output::show_stats();
        }
        
        debug("mem counter output: " << mem_counter);
    }

}