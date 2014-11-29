#include "compression.h"
#include "pubmed.h"
#include "input.h"
#include "output.h"
#include "huffman.h"

#define TUPLES 100000
#define REPS 1000

namespace benchmark {
    
    unsigned short* cb_terms_uncompressed;
    int num_tuples = 0;
    
    unsigned short* huffman_array;
    bool* terminator_array;
    Node<unsigned short>* huffman_tree;
    char* cb_terms_huffman;
    
    struct rle_pair
    {
        unsigned char value;
        int count;
    };
    rle_pair* cb_terms_rle;
    int rle_len = 0;
    
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
        show_info("Generated " << num_tuples << " terms.");
        
        show_info("[2] Compressing with Huffman...");
        generate_array_tree_representation(cb_terms_uncompressed, num_tuples, huffman_array, terminator_array, huffman_tree);
        encoding_dict<unsigned short> encoding_dict;
        build_inverse_mapping(huffman_tree, encoding_dict);
        long size_compressed = encode(cb_terms_uncompressed, num_tuples, cb_terms_huffman, encoding_dict);
        show_info("Compressed " << num_tuples * sizeof(unsigned short) << " bytes -> " << size_compressed << " bytes.");
        
        show_info("[3] Compressing with RLE...");
        cb_terms_rle = new rle_pair[rle_len];
        int counter = 0;
        for (int t = 0; t < input::T_PM; ++t)
        {
            double num_times = pubmed::get_group_by_term(t) / divider;
            
            if (num_times > 0)
            {
                cb_terms_rle[counter].value = t;
                cb_terms_rle[counter].count = 0;
                for (int i = 0; i < num_times; ++i)
                {
                    cb_terms_rle[counter].count++;
                }
                
                counter++;
            }
        }
        show_info("Compressed " << num_tuples * sizeof(unsigned short) << " bytes -> " << rle_len * sizeof(rle_pair) << " bytes.");
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
}