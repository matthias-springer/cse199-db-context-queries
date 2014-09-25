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
    bool compress = true;
    
    unsigned short* tuples;
    unsigned char* tuples_freq;
    
    unordered_map<long, unsigned short*> terms_per_doc;
    unordered_map<long, char*> terms_per_doc_compressed;
    unordered_map<long, unsigned char*> freqs_per_doc;
    unordered_map<long, char*> freqs_per_doc_compressed;
    
    unordered_map<long, unsigned short> terms_per_doc_size;
    Node<unsigned short>* tree;
    Node<unsigned char>* tree_freqs;
    
    unsigned short* huffman_array_terms;
    unsigned char* huffman_array_freqs;
    bool* terminator_array_terms;
    bool* terminator_array_freqs;
    
    long terms_bytes_uncompressed = 0;
    long terms_bytes_compressed = 0;
    long freqs_bytes_uncompressed = 0;
    long freqs_bytes_compressed = 0;
    
    void huffman_query_generate_lists()
    {
        output::start_timer("run/bench_huffman_query_generate");
        
        // generate tuples
        show_info("[1] Generating tuples...");
        show_info("Allocating " << pubmed::tuples << " shorts/chars (array).");
        tuples = new unsigned short[pubmed::tuples];
        tuples_freq = new unsigned char[pubmed::tuples];
        show_info("Alloc success");
        
        long next_index = 0;
        
        for (short t = 0; t < input::T_PM; ++t)
        {
            for (long c = 0; c < pubmed::get_group_by_term(t); ++c)
            {
                if (rand() % 100 < 20) {
                    // 20% probability for < 1024
                    tuples_freq[next_index] = rand();
                } else {
                    tuples_freq[next_index] = rand() % 15;
                }
                
                tuples[next_index++] = t;
            }
            
            if (t % (input::T_PM/1000) == 0) debug_n("  " << t*100.0/input::T_PM << " % complete.    ");
        }
        debug_n("  " << 100 << " % complete.    \n");
        
        // shuffle
        show_info("[2] Shuffling...");
        shuffle(tuples, tuples + pubmed::tuples, default_random_engine(42));
        
        // generate terms per doc
        show_info("[3] Generating separate lists...");
        next_index = 0;
        for (long d = 0; d < input::D_PM; ++d)
        {
            unsigned short* list = new unsigned short[pubmed::get_group_by_doc(d)];
            memcpy(list, tuples + next_index, pubmed::get_group_by_doc(d) * sizeof(unsigned short));
            terms_per_doc[d] = list;
            
            unsigned char* freqs_list = new unsigned char[pubmed::get_group_by_doc(d)];
            memcpy(freqs_list, tuples_freq + next_index, pubmed::get_group_by_doc(d) * sizeof(unsigned char));
            freqs_per_doc[d] = freqs_list;
            
            terms_per_doc_size[d] = pubmed::get_group_by_doc(d);
            
            if (d % (input::D_PM/1000) == 0) debug_n("  " << d*100.0/input::D_PM << " % complete.    ");
        }
        debug_n("  " << 100 << " % complete.    \n");
        
        
        if (compress)
        {
            // compress
            show_info("[4] Generating Huffman tree for terms...");
            generate_array_tree_representation(tuples, pubmed::tuples, huffman_array_terms, terminator_array_terms, tree);
            encoding_dict<unsigned short> encoding_dict_terms;
            build_inverse_mapping(tree, encoding_dict_terms);
            
            delete(tuples);
            
            show_info("[5] Compressing terms...");
            for (long d = 0; d < input::D_PM; ++d)
            {
                char* terms_compressed;
                terms_bytes_uncompressed += terms_per_doc_size[d] * sizeof(unsigned short);
                terms_bytes_compressed += encode(terms_per_doc[d], terms_per_doc_size[d], terms_compressed, encoding_dict_terms);
                terms_per_doc_compressed[d] = terms_compressed;
                delete terms_per_doc[d];
                
                if (d % (input::D_PM/1000) == 0) debug_n("  " << d*100.0/input::D_PM << " % complete.    ");
            }
            debug_n("  " << 100 << " % complete.    \n");
            
            // compress
            show_info("[6] Generating Huffman tree for frequencies...");
            generate_array_tree_representation(tuples_freq, pubmed::tuples, huffman_array_freqs, terminator_array_freqs, tree_freqs);
            encoding_dict<unsigned char> encoding_dict_freqs;
            build_inverse_mapping(tree_freqs, encoding_dict_freqs);
            
            delete(tuples_freq);
            
            show_info("[7] Compressing frequencies...");
            for (long d = 0; d < input::D_PM; ++d)
            {
                char* freqs_compressed;
                freqs_bytes_uncompressed += terms_per_doc_size[d] * sizeof(unsigned char);
                freqs_bytes_compressed += encode(freqs_per_doc[d], terms_per_doc_size[d], freqs_compressed, encoding_dict_freqs);
                freqs_per_doc_compressed[d] = freqs_compressed;
                delete freqs_per_doc[d];
                
                if (d % (input::D_PM/1000) == 0) debug_n("  " << d*100.0/input::D_PM << " % complete.    ");
            }
            debug_n("  " << 100 << " % complete.    \n");
            
            show_info("terms bytes uncompressed: " << terms_bytes_uncompressed);
            show_info("terms bytes compressed: " << terms_bytes_compressed);
            show_info("freqs bytes uncompressed: " << freqs_bytes_uncompressed);
            show_info("freqs bytes compressed: " << freqs_bytes_compressed);
        }
        else
        {
            delete(tuples);
            delete(tuples_freq);
            show_info("No compression.");
        }
        
        output::stop_timer("run/bench_huffman_query_generate");
        
        show_info("Done.");
    }
    
    void huffman_query_run_benchmark()
    {
        int num_docs[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};
        
        for (int i = 0; i < 7; ++i)
        {
            show_info("Running benchmark for " << num_docs[i] << " documents (15 iterations).");
            output::start_timer("run/bench_huffman_query");
         
            for (int j = 0; j < 15; ++j)
            {
                map_aggregation term_counter;
                //unordered_map<unsigned short, long> term_counter;
                
                for (int di = 0; di < num_docs[i]; ++di)
                {
                    // retrieve list and add to term counter
                    long doc_id = rand() % input::D_PM;
                    
                    //unsigned short* terms = terms_per_doc[doc_id];
                    unsigned short list_size = terms_per_doc_size[doc_id];
                    
                    unsigned short* terms_decompressed;
                    unsigned char* freqs_decompressed;
                    
                    if (compress)
                    {
                        terms_decompressed = new unsigned short[list_size];
                        freqs_decompressed = new unsigned char[list_size];
                        decode(terms_per_doc_compressed[doc_id], list_size, terms_decompressed, huffman_array_terms, terminator_array_terms);
                        decode(freqs_per_doc_compressed[doc_id], list_size, freqs_decompressed, huffman_array_freqs, terminator_array_freqs);
                    }
                    else
                    {
                        terms_decompressed = terms_per_doc[doc_id];
                        freqs_decompressed = freqs_per_doc[doc_id];
                    }
                    
                    for (int l = 0; l < list_size; ++l)
                    {
                        term_counter.add(terms_decompressed[l], freqs_decompressed[l]);
                        //term_counter[terms[l]]++;
                    }
                    
                    if (compress)
                    {
                        delete terms_decompressed;
                        delete freqs_decompressed;
                    }
                }
                
                // sort list and extract top-k
                term_counter.top_k(5);
            }
            
            // stats
            output::stop_timer("run/bench_huffman_query");
            output::show_stats();
            output::clear_stats();
        }
        
    }
}
