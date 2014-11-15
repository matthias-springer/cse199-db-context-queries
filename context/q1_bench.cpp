#include "q1_bench.h"
#include "input.h"
#include "pubmed.h"
#include "huffman.h"
#include <pthread.h>

#define NUM_THREADS 4

namespace benchmark
{
    unsigned short* p1_terms;
    unsigned short* p1_huffman_array;
    bool* p1_terminator_array;
    Node<unsigned short>* p1_tree;
    
    unsigned short** p1_terms_fragments;
    char** p1_terms_fragments_compressed;
    
    
    int* p2_docs;
    int* p2_huffman_array;
    bool* p2_terminator_array;
    Node<int>* p2_tree;
    
    int** p2_docs_fragments;
    char** p2_docs_fragments_compressed;
    
    int** exact_docs_oo;
    
    void generate_data_q1()
    {
        exact_docs_oo = input::docs_bench_items(); // check at index 5
        
        show_info("[P1] Generating random data...");
        p1_terms = new unsigned short[input::NUM_TUPLES];
        int index = 0;
        
        for (int t = 0; t < input::T_PM; ++t)
        {
            int cnt = pubmed::get_group_by_term(t);
            
            for (int d = 0; d < cnt; ++d)
            {
                p1_terms[index++] = t;
            }
        }
        
        // compress
        show_info("[P1] Generating Huffman tree...");
        generate_array_tree_representation(p1_terms, input::NUM_TUPLES, p1_huffman_array, p1_terminator_array, p1_tree);
        encoding_dict<unsigned short> encoding_dict_terms;
        build_inverse_mapping(p1_tree, encoding_dict_terms);
        
        // single lists
        show_info("[P1] Generating single lists...");
        p1_terms_fragments = new unsigned short*[input::D_PM];
        
        index = 0;
        for (int d = 0; d < input::D_PM; ++d)
        {
            int num_terms = pubmed::get_group_by_doc(d);
            p1_terms_fragments[d] = new unsigned short[num_terms];
            
            for (int t = 0; t < num_terms; ++t)
            {
                p1_terms_fragments[d][t] = p1_terms[index++];
            }
        }
        
        delete[] p1_terms;
    
        // compress
        show_info("[P1] Compressing...");
        p1_terms_fragments_compressed = new char*[input::D_PM];
        
        for (int d = 0; d < input::D_PM; ++d)
        {
            encode(p1_terms_fragments[d], pubmed::get_group_by_doc(d), p1_terms_fragments_compressed[d], encoding_dict_terms);
            
            delete[] p1_terms_fragments[d];
        }
        delete[] p1_terms_fragments;
        
    
        show_info("[P2] Generating random data...");
        p2_docs = new int[input::NUM_TUPLES];
        index = 0;
        
        for (int d = 0; d < input::D_PM; ++d)
        {
            int cnt = pubmed::get_group_by_doc(d);
            
            for (int t = 0; t < cnt; ++t)
            {
                p2_docs[index++] = d;
            }
        }
        
        // generate Huffman tree
        show_info("[P2] Generating Huffman tree...");
        generate_array_tree_representation(p2_docs, input::NUM_TUPLES, p2_huffman_array, p2_terminator_array, p2_tree);
        encoding_dict<int> encoding_dict_docs;
        build_inverse_mapping(p2_tree, encoding_dict_docs);
        
        // single lists
        show_info("[P2] Generating single lists...");
        p2_docs_fragments = new int*[input::T_PM];
        
        index = 0;
        for (int t = 0; t < input::T_PM; ++t)
        {
            int num_docs = pubmed::get_group_by_term(t);
            p2_docs_fragments[t] = new int[num_docs];
            
            for (int d = 0; d < num_docs; ++d)
            {
                p2_docs_fragments[t][d] = p2_docs[index++];
            }
        }
        
        delete[] p2_docs;
        
        // compress
        show_info("[P2] Compressing...");
        p2_docs_fragments_compressed = new char*[input::T_PM];
        
        for (int t = 0; t < input::T_PM; ++t)
        {
            encode(p2_docs_fragments[t], pubmed::get_group_by_term(t), p2_docs_fragments_compressed[t], encoding_dict_docs);
            
            delete[] p2_docs_fragments[t];
        }
        delete[] p2_docs_fragments;
        
        show_info("DONE.");
    }
    
    struct thread_args
    {
        unordered_map<int, int>* doc_freq;
        unsigned short* input_terms;
        int num_terms;
    };
    

    void* pthread_q1(void* vargs)
    {
        thread_args* args = (thread_args*) vargs;
        show_info("[TT] Thread started with " << args->num_terms << " terms!");
        
        for (int t = 0; t < args->num_terms; ++t)
        {
            // retrieve doc fragments
            show_info("[T] Decompressing doc fragment " << t << "...");
            int* doc_fragment_uncompressed;
            int term = args->input_terms[t];
            show_info("[T] Term number " << term << ".");
            decode(p2_docs_fragments_compressed[term], pubmed::get_group_by_term(term), doc_fragment_uncompressed, p2_huffman_array, p2_terminator_array);
            
            show_info("[T] Aggregating...");
            for (int d = 0; d < pubmed::get_group_by_term(term); ++d)
            {
                (*args->doc_freq)[doc_fragment_uncompressed[d]]++;
            }
            
            delete[] doc_fragment_uncompressed;
        }
        
        return NULL;
    }
    
    void q1_final_bench()
    {
        int* input_docs = exact_docs_oo[5];
        
        show_info("Running Q1 with 1000 repetitions and " << NUM_THREADS << " threads...");
        
        output::start_timer("run/q1_bench");
        
        for (int r = 0; r < 1000; ++r)
        {
            int doc = input_docs[r];
            int term_cnt = pubmed::get_group_by_doc(doc);
            
            show_info("Uncompressing for doc " << doc << " with " << term_cnt << " terms...");
            // retrieve terms for doc
            unsigned short* terms_uncompressed;
            decode(p1_terms_fragments_compressed[doc], term_cnt, terms_uncompressed, p1_huffman_array, p1_terminator_array);
            
            /* SANITY CHECK */
            for (int i = 0; i < term_cnt; ++i)
            {
                show_info("SC[" << i << "/" << term_cnt << "] = " << terms_uncompressed[i]);
            }
            /* END SANITY CHECK */
            
            show_info("Allocating thread data...");
            pthread_t** threads = new pthread_t*[NUM_THREADS];
            thread_args** args = new thread_args*[NUM_THREADS];
            
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                show_info("Preparing thread data...");
                args[t] = new thread_args;
                args[t]->doc_freq = new unordered_map<int, int>();
                args[t]->num_terms = term_cnt / NUM_THREADS;
                if (args[t]->num_terms > 0)
                {
                    args[t]->input_terms = new unsigned short[args[t]->num_terms];
                    memcpy(args[t]->input_terms, terms_uncompressed + (t-1) * args[t]->num_terms, args[t]->num_terms * sizeof(unsigned short));
                }
                
                threads[t] = new pthread_t();
                show_info("Starting thread...");
                pthread_create(threads[t], NULL, pthread_q1, (void*) args[t]);
            }
            
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                show_info("Joining thread...");
                pthread_join(*threads[t], NULL);
            }
            
            show_info("Merging thread results...");
            unordered_map<int, int> result;
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                thread_args* arg = args[t];
                for (auto it = arg->doc_freq->begin(); it != arg->doc_freq->end(); ++it)
                {
                    result[it->first] += it->second;
                }
                
                delete arg->doc_freq;
                delete arg->input_terms;
                delete arg;
            }
            
            // TODO: output aggregated map
        }
        
        output::stop_timer("run/q1_bench");
        output::show_stats();
    }
}