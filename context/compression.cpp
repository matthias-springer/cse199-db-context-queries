#include "compression.h"
#include "pubmed.h"
#include "input.h"
#include "output.h"
#include "huffman.h"
#include <ibis.h>

#define TUPLES 100000
#define REPS 10000

namespace benchmark {
    
    unsigned short* cb_terms_uncompressed;
    int num_tuples = 0;
    
    unsigned short* huffman_array;
    bool* terminator_array;
    Node<unsigned short>* huffman_tree;
    char* cb_terms_huffman;
    
    struct rle_pair
    {
        unsigned short value;
        int count;
    };
    rle_pair* cb_terms_rle;
    int rle_len = 0;
    
    
    ibis::bitvector* vector;
    
    void cb_generate_tuples()
    {
        show_info("Benchmark: compression with duplicates");
        
        show_info("[1] Generating uncompressed terms...");
        cb_terms_uncompressed = new unsigned short[input::NUM_TUPLES];
        double divider = ((double)input::NUM_TUPLES) / TUPLES;
        
        for (int t = 0; t < input::T_PM; ++t)
        {
            bool have_term = false;
            
            for (int d = 0; d < pubmed::get_group_by_term(t) / divider; ++d)
            {
                cb_terms_uncompressed[num_tuples++] = t;
                have_term = true;
            }
            
            if (have_term) rle_len++;
        }
        show_info("Generated " << num_tuples << " terms (" << rle_len << " distinct terms).");
        
        show_info("[2] Compressing with Huffman...");
        generate_array_tree_representation(cb_terms_uncompressed, num_tuples, huffman_array, terminator_array, huffman_tree);
        encoding_dict<unsigned short> encoding_dict;
        build_inverse_mapping(huffman_tree, encoding_dict);
        long size_compressed = encode(cb_terms_uncompressed, num_tuples, cb_terms_huffman, encoding_dict);
        show_info("Compressed " << num_tuples * sizeof(unsigned short) << " bytes -> " << size_compressed << " bytes.");
        
        show_info("[3] Compressing with RLE...");
        cb_terms_rle = new rle_pair[rle_len];
        int counter = -1;
        int last = -1;
        
        for (int i = 0; i < num_tuples; ++i)
        {
            if (last != cb_terms_uncompressed[i])
            {
                last = cb_terms_uncompressed[i];
                counter++;
                
                cb_terms_rle[counter].value = last;
                cb_terms_rle[counter].count = 0;
            }
            
            cb_terms_rle[counter].count++;
        }
        
        counter++;
        
        if (counter != rle_len) error("counter " << counter << " == rle_len " << rle_len << " failed.");
        
        show_info("Compressed " << num_tuples * sizeof(unsigned short) << " bytes -> " << rle_len * sizeof(rle_pair) << " bytes.");
        
        show_info("[3] Generating random bit vector...")
        vector = new ibis::bitvector();
        
        for (int i = 0; i < num_tuples; ++i)
        {
            vector->setBit(rand() % 10000000, 1);
        }
        vector->compress();
    }
    
    void cb_uncompressed()
    {
        show_info("Running uncompressed with " << REPS << " repetitions...");
        output::start_timer("cb/uncompressed");
        
        int sum = 0;
        
        for (int r = 0; r < REPS; ++r)
        {
            sum = 0;
            
            for (int i = 0; i < num_tuples; ++i)
            {
                sum += cb_terms_uncompressed[i];
            }
        }
        
        output::stop_timer("cb/uncompressed");
        output::show_stats();
        show_info("checksum: " << sum);
    }
    
    void cb_huffman()
    {
        show_info("Running huffman with " << REPS << " repetitions...");
        output::start_timer("cb/huffman");
        int sum = 0;
        
        for (int r = 0; r < REPS; ++r)
        {
            unsigned short* huffman_uncompressed;
            decode(cb_terms_huffman, num_tuples, huffman_uncompressed, huffman_array, terminator_array);
            
            sum = 0;
            
            for (int i = 0; i < num_tuples; ++i)
            {
                sum += huffman_uncompressed[i];
            }
        }
        
        output::stop_timer("cb/huffman");
        output::show_stats();
        show_info("checksum: " << sum);
    }
    
    void cb_rle()
    {
        show_info("Running RLE with " << REPS << " repetitions...");
        output::start_timer("cb/rle");
        
        int sum = 0;
        
        for (int r = 0; r < REPS; ++r)
        {
            sum = 0;
            
            for (int i = 0; i < rle_len; ++i)
            {
                sum += cb_terms_rle[i].value * cb_terms_rle[i].count;
            }
        }
        
        output::stop_timer("cb/rle");
        output::show_stats();
        show_info("checksum: " << sum);
    }
    
    void cb_bitvector()
    {
        show_info("Running bit vector with " << REPS << " repetitions (different data set)...");
        output::start_timer("cb/bitvector");
        
        int sum = 0;
        
        for (int r = 0; r < REPS; ++r)
        {
            sum = 0;
            
            ibis::bitvector::indexSet ones = vector->firstIndexSet();
            
            while (ones.nIndices() > 0)
            {
                for (int i = 0; i < ones.nIndices(); ++i)
                {
                    sum += ones.indices()[i];
                }
                
                ++ones;
            }
        }
        
        output::stop_timer("cb/bitvector");
        output::show_stats();
        show_info("checksum: " << sum);
    }
}