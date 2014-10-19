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
    
    ibis::bitvector* bit_vector_for_term;
    
    void generate_bit_vectors()
    {
        show_info("[1] Generating tuples...");
        column_doc = new unsigned int[input::NUM_TUPLES];
        column_term = new unsigned short[input::NUM_TUPLES];
        
        long next_index = 0;
        
        for (unsigned int t = 0; t < input::D_PM; ++t)
        {
            for (long c = 0; c < pubmed::get_group_by_doc(t); ++c)
            {
                column_doc[next_index++] = t;
            }
            
            if (t % (input::T_PM/1000) == 0) debug_n("  " << t*100.0/input::T_PM << " % complete.    ");
        }
        debug_n("  " << 100 << " % complete.    \n");
        
        // shuffle
        show_info("[2] Shuffling...");
        shuffle(column_doc, column_doc + input::NUM_TUPLES, default_random_engine(42));
    }
}