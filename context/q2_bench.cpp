#include "q2_bench.h"
#include "input.h"
#include "pubmed.h"
#include "huffman.h"
#include <pthread.h>
#include <random>
#include <ibis.h>

#define NUM_THREADS 4
#define HUFFMAN

namespace benchmark
{
    unsigned short* a_p1_terms;
    unsigned short** a_p1_terms_fragments;
    
#ifdef HUFFMAN
    char** a_p1_terms_fragments_compressed;
    unsigned short* a_p1_huffman_array;
    bool* a_p1_terminator_array;
    Node<unsigned short>* a_p1_tree;
#endif
    
    int* a_p2_docs;
    int** a_p2_docs_fragments;
    unsigned char* a_p2_freqs;
    unsigned char** a_p2_freqs_fragments;
    
#ifdef HUFFMAN
    int* a_p2_huffman_array;
    bool* a_p2_terminator_array;
    char** a_p2_docs_fragments_compressed;
    Node<int>* a_p2_tree;
    
    unsigned char* a_p2_f_huffman_array;
    bool* a_p2_f_terminator_array;
    char** a_p2_freqs_fragments_compressed;
    Node<unsigned char>* a_p2_f_tree;
#endif
    
    int** a_exact_docs_oo;
    
    void generate_data_q2()
    {
        a_exact_docs_oo = input::docs_bench_items(); // check at index 5
        
        show_info("[P1] Generating random data...");
        a_p1_terms = new unsigned short[input::NUM_TUPLES];
        int index = 0;
        
        for (int t = 0; t < input::T_PM; ++t)
        {
            int cnt = pubmed::get_group_by_term(t);
            
            for (int d = 0; d < cnt; ++d)
            {
                a_p1_terms[index++] = t;
            }
        }
        
        // shuffle
        show_info("[P1] Shuffle...");
        shuffle(a_p1_terms, a_p1_terms + input::NUM_TUPLES, default_random_engine(42));
        
#ifdef HUFFMAN
        // compress
        show_info("[P1] Generating Huffman tree...");
        generate_array_tree_representation(a_p1_terms, input::NUM_TUPLES, a_p1_huffman_array, a_p1_terminator_array, a_p1_tree);
        encoding_dict<unsigned short> encoding_dict_terms;
        build_inverse_mapping(a_p1_tree, encoding_dict_terms);
        delete a_p1_tree;
#endif
        
        // single lists
        show_info("[P1] Generating single lists...");
        a_p1_terms_fragments = new unsigned short*[input::D_PM];
        
        index = 0;
        for (int d = 0; d < input::D_PM; ++d)
        {
            int num_terms = pubmed::get_group_by_doc(d);
            a_p1_terms_fragments[d] = new unsigned short[num_terms];
            
            for (int t = 0; t < num_terms; ++t)
            {
                a_p1_terms_fragments[d][t] = a_p1_terms[index++];
            }
        }
        
        delete[] a_p1_terms;
        
#ifdef HUFFMAN
        // compress
        show_info("[P1] Compressing...");
        a_p1_terms_fragments_compressed = new char*[input::D_PM];
        
        for (int d = 0; d < input::D_PM; ++d)
        {
            encode(a_p1_terms_fragments[d], pubmed::get_group_by_doc(d), a_p1_terms_fragments_compressed[d], encoding_dict_terms);
            
            delete[] a_p1_terms_fragments[d];
        }
        delete[] a_p1_terms_fragments;
#endif
        
        show_info("[P2] Generating random data...");
        a_p2_docs = new int[input::NUM_TUPLES];
        a_p2_freqs = new unsigned char[input::NUM_TUPLES];
        index = 0;
        
        for (int d = 0; d < input::D_PM; ++d)
        {
            int cnt = pubmed::get_group_by_doc(d);
            
            for (int t = 0; t < cnt; ++t)
            {
                int r = rand() % 100;
                if (r < 25) a_p2_freqs[index] = 1;
                else if (r < 45) a_p2_freqs[index] = 2;
                else if (r < 60) a_p2_freqs[index] = 3;
                else if (r < 70) a_p2_freqs[index] = 4;
                else if (r < 75) a_p2_freqs[index] = 5;
                else if (r < 77) a_p2_freqs[index] = 6;
                else a_p2_freqs[index] = rand() % 40;
                
                a_p2_docs[index++] = d;
            }
        }
        
        // shuffle
        show_info("[P2] Shuffle...");
        shuffle(a_p2_docs, a_p2_docs + input::NUM_TUPLES, default_random_engine(42));
        shuffle(a_p2_freqs, a_p2_freqs + input::NUM_TUPLES, default_random_engine(42));
        
#ifdef HUFFMAN
        // generate Huffman tree
        show_info("[P2] Generating Huffman tree...");
        generate_array_tree_representation(a_p2_docs, input::NUM_TUPLES, a_p2_huffman_array, a_p2_terminator_array, a_p2_tree);
        encoding_dict<int> encoding_dict_docs;
        build_inverse_mapping(a_p2_tree, encoding_dict_docs);
        
        generate_array_tree_representation(a_p2_freqs, input::NUM_TUPLES, a_p2_f_huffman_array, a_p2_f_terminator_array, a_p2_f_tree);
        encoding_dict<unsigned char> encoding_dict_freqs;
        build_inverse_mapping(a_p2_f_tree, encoding_dict_freqs);
        delete a_p2_tree;
        delete a_p2_f_tree;
#endif
        
        // single lists
        show_info("[P2] Generating single lists...");
        a_p2_docs_fragments = new int*[input::T_PM];
        a_p2_freqs_fragments = new unsigned char*[input::T_PM];
        
        index = 0;
        for (int t = 0; t < input::T_PM; ++t)
        {
            int num_docs = pubmed::get_group_by_term(t);
            a_p2_docs_fragments[t] = new int[num_docs];
            a_p2_freqs_fragments[t] = new unsigned char[num_docs];
            
            for (int d = 0; d < num_docs; ++d)
            {
                a_p2_freqs_fragments[t][d] = a_p2_freqs[index];
                a_p2_docs_fragments[t][d] = a_p2_docs[index++];
            }
        }
        
        delete[] a_p2_docs;
        
#ifdef HUFFMAN
        // compress
        show_info("[P2] Compressing...");
        a_p2_docs_fragments_compressed = new char*[input::T_PM];
        a_p2_freqs_fragments_compressed = new char*[input::T_PM];
        
        for (int t = 0; t < input::T_PM; ++t)
        {
            encode(a_p2_docs_fragments[t], pubmed::get_group_by_term(t), a_p2_docs_fragments_compressed[t], encoding_dict_docs);
            delete[] a_p2_docs_fragments[t];
            
            encode(a_p2_freqs_fragments[t], pubmed::get_group_by_term(t), a_p2_freqs_fragments_compressed[t], encoding_dict_freqs);
            delete[] a_p2_freqs_fragments[t];
        }
        delete[] a_p2_docs_fragments;
        delete[] a_p2_freqs_fragments;
#endif
        
        show_info("DONE.");
    }
    
    struct thread_args
    {
        unordered_map<int, int>* doc_freq;
        unsigned short* input_terms;
        int num_terms;
    };
    
    
    void* pthread_q2(void* vargs)
    {
        debug("[T] Thread is running...");
        
        int cntr = 0;
        
        thread_args* args = (thread_args*) vargs;
        
        for (int t = 0; t < args->num_terms; ++t)
        {
            // retrieve freq fragmetns
            unsigned char* freq_fragment_uncompressed;
            
            // retrieve doc fragments
            int* doc_fragment_uncompressed;
            int term = args->input_terms[t];
            
#ifdef HUFFMAN
            decode(a_p2_docs_fragments_compressed[term], pubmed::get_group_by_term(term), doc_fragment_uncompressed, a_p2_huffman_array, a_p2_terminator_array);
            decode(a_p2_freqs_fragments_compressed[term], pubmed::get_group_by_term(term), freq_fragment_uncompressed, a_p2_f_huffman_array, a_p2_f_terminator_array);
#else
            doc_fragment_uncompressed = a_p2_docs_fragments[term];
            freq_fragment_uncompressed = a_p2_freqs_fragments[term];
#endif
            
            for (int d = 0; d < pubmed::get_group_by_term(term); ++d)
            {
                (*args->doc_freq)[doc_fragment_uncompressed[d]] += freq_fragment_uncompressed[d];
                cntr++;
            }
            
#ifdef HUFFMAN
            delete[] doc_fragment_uncompressed;
            delete[] freq_fragment_uncompressed;
#endif
        }
        
        debug("Thread aggregated " << cntr << " documents.");
        return NULL;
    }
    
    void q2_final_bench()
    {
        int* input_docs = a_exact_docs_oo[5];
        
        show_info("Running Q2 with 10 repetitions and " << NUM_THREADS << " threads...");
        
        output::start_timer("run/q2_bench");
        
        for (int r = 0; r < 10; ++r)
        {
            output::start_timer("run/current_rep");
            debug("REP " << r);
            int doc = input_docs[r];
            int term_cnt = pubmed::get_group_by_doc(doc);
            
            debug("Processing " << term_cnt << " terms for doc " << doc << "...");
            
            // retrieve terms for doc
            unsigned short* terms_uncompressed;
            
#ifdef HUFFMAN
            decode(a_p1_terms_fragments_compressed[doc], term_cnt, terms_uncompressed, a_p1_huffman_array, a_p1_terminator_array);
#else

            terms_uncompressed = a_p1_terms_fragments[doc];
#endif
            
            pthread_t** threads = new pthread_t*[NUM_THREADS];
            thread_args** args = new thread_args*[NUM_THREADS];
            
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                args[t] = new thread_args;
                args[t]->doc_freq = new unordered_map<int, int>();
                args[t]->num_terms = term_cnt / NUM_THREADS;
                if (args[t]->num_terms > 0)
                {
                    args[t]->input_terms = new unsigned short[args[t]->num_terms];
                    memcpy(args[t]->input_terms, terms_uncompressed + t * args[t]->num_terms, args[t]->num_terms * sizeof(unsigned short));
                }
                
                threads[t] = new pthread_t();
                pthread_create(threads[t], NULL, pthread_q2, (void*) args[t]);
            }
            
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                pthread_join(*threads[t], NULL);
            }
            
            output::start_timer("run/phase2-aggregate-threads");
            
            if (NUM_THREADS > 1)
            {
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
            }
            output::stop_timer("run/phase2-aggregate-threads");
            
            // TODO: output aggregated map
            output::stop_timer("run/current_rep");
            output::show_stats();
        }
        
        output::stop_timer("run/q2_bench");
        output::show_stats();
    }
}