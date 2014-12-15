#include "q1_bench.h"
#include "input.h"
#include "pubmed.h"
#include "huffman.h"
#include <pthread.h>
#include <random>
#include <ibis.h>

#define NUM_THREADS 1
#define HUFFMAN
//#define FASTBIT

namespace benchmark
{
    unsigned short* p1_terms;
    unsigned short** p1_terms_fragments;
    
#ifdef HUFFMAN
    char** p1_terms_fragments_compressed;
    int* p1_terms_fragments_compressed_bytes;
    unsigned short* p1_huffman_array;
    bool* p1_terminator_array;
    Node<unsigned short>* p1_tree;
#endif
    
#ifdef FASTBIT
    ibis::bitvector** p1_terms_fragments_compressed;
#endif
    
    int* p2_docs;
    int** p2_docs_fragments;
    
#ifdef HUFFMAN
    int* p2_huffman_array;
    bool* p2_terminator_array;
    char** p2_docs_fragments_compressed;
    int* p2_docs_fragments_compressed_bytes;
    Node<int>* p2_tree;
#endif
    
#ifdef FASTBIT
    ibis::bitvector** p2_docs_fragments_compressed;
#endif
    
    
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
        
        // shuffle
        show_info("[P1] Shuffle...");
        shuffle(p1_terms, p1_terms + input::NUM_TUPLES, default_random_engine(42));
        
#ifdef HUFFMAN
        // compress
        show_info("[P1] Generating Huffman tree...");
        generate_array_tree_representation(p1_terms, input::NUM_TUPLES, p1_huffman_array, p1_terminator_array, p1_tree);
        encoding_dict<unsigned short> encoding_dict_terms;
        build_inverse_mapping(p1_tree, encoding_dict_terms);
        delete p1_tree;
#endif
        
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
    
#ifdef HUFFMAN
        // compress
        show_info("[P1] Compressing...");
        p1_terms_fragments_compressed = new char*[input::D_PM];
        p1_terms_fragments_compressed_bytes = new int[input::D_PM];
        
        for (int d = 0; d < input::D_PM; ++d)
        {
            p1_terms_fragments_compressed_bytes[d] = encode(p1_terms_fragments[d], pubmed::get_group_by_doc(d), p1_terms_fragments_compressed[d], encoding_dict_terms);
            
            delete[] p1_terms_fragments[d];
        }
        delete[] p1_terms_fragments;
#endif
    
#ifdef FASTBIT
        show_info("[P1] Compressing with FastBit...");
        p1_terms_fragments_compressed = new ibis::bitvector*[input::D_PM];
        
        for (int d = 0; d < input::D_PM; ++d)
        {
            //p1_terms_fragments_compressed[d] = new ibis::bitvector();
            ibis::bitvector* vector = new ibis::bitvector();
            
            for (int t_i = 0; t_i < pubmed::get_group_by_doc(d); ++t_i)
            {
                vector->setBit(p1_terms_fragments[d][t_i], 1);
            }
            
            vector->compress();
            // force new allocate
            ibis::array_t<uint32_t>* arr = new ibis::array_t<uint32_t>();
            vector->write(*arr);
            delete vector;
            p1_terms_fragments_compressed[d] = new ibis::bitvector(*arr);
            delete arr;
            
            delete[] p1_terms_fragments[d];
        }
        delete[] p1_terms_fragments;
#endif
        
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
        
        // shuffle
        show_info("[P2] Shuffle...");
        shuffle(p2_docs, p2_docs + input::NUM_TUPLES, default_random_engine(42));
        
#ifdef HUFFMAN
        // generate Huffman tree
        show_info("[P2] Generating Huffman tree...");
        generate_array_tree_representation(p2_docs, input::NUM_TUPLES, p2_huffman_array, p2_terminator_array, p2_tree);
        encoding_dict<int> encoding_dict_docs;
        build_inverse_mapping(p2_tree, encoding_dict_docs);
        delete p2_tree;
#endif
        
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
        
#ifdef HUFFMAN
        // compress
        show_info("[P2] Compressing...");
        p2_docs_fragments_compressed = new char*[input::T_PM];
        p2_docs_fragments_compressed_bytes = new int[input::T_PM];
        
        for (int t = 0; t < input::T_PM; ++t)
        {
            p2_docs_fragments_compressed_bytes[t] = encode(p2_docs_fragments[t], pubmed::get_group_by_term(t), p2_docs_fragments_compressed[t], encoding_dict_docs);
            
            delete[] p2_docs_fragments[t];
        }
        delete[] p2_docs_fragments;
#endif
        
#ifdef FASTBIT
        show_info("[P2] Compressing with FastBit...");
        p2_docs_fragments_compressed = new ibis::bitvector*[input::T_PM];
        
        for (int t = 0; t < input::T_PM; ++t)
        {
            ibis::bitvector* vector = new ibis::bitvector();
            
            for (int d_i = 0; d_i < pubmed::get_group_by_term(t); ++d_i)
            {
                vector->setBit(p2_docs_fragments[t][d_i], 1);
            }
            
            vector->compress();
            
            
            // force new allocate
            ibis::array_t<uint32_t>* arr = new ibis::array_t<uint32_t>();
            vector->write(*arr);
            delete vector;
            p2_docs_fragments_compressed[t] = new ibis::bitvector(*arr);
            delete arr;
            
            /** SANITY CHECK **/
            set<int> checkSet;
            ibis::bitvector::indexSet ones = p2_docs_fragments_compressed[t]->firstIndexSet();
            while (ones.nIndices() != 0)
            {
                for (int i = 0; i < ones.nIndices(); ++i)
                {
                    checkSet.insert(ones.indices()[i]);
                }
                
                ++ones;
            }
            
            for (int d = 0; d < pubmed::get_group_by_term(t); ++d)
            {
                if (checkSet.find(p2_docs_fragments[t][d]) == checkSet.end())
                {
                    show_info("WARNING: doc " << p2_docs_fragments[t][d] << " not found in set!");
                }
            }
            /** END OF SANITY CHECK **/
            
            delete[] p2_docs_fragments[t];
        }
        delete[] p2_docs_fragments;
#endif
        
        show_info("Swapping prevention...");
#ifdef HUFFMAN
        int tmp = 0;
        
        for (int d = 0; d < input::D_PM; ++d)
        {
            for (int t = 0; t < p1_terms_fragments_compressed_bytes[d]; ++t)
            {
                tmp = (tmp + p1_terms_fragments_compressed[d][t]) % 256;
            }
        }
        
        for (int t = 0; t < input::T_PM; ++t)
        {
            for (int d = 0; d < p2_docs_fragments_compressed_bytes[t]; ++d)
            {
                tmp = (tmp + p2_docs_fragments_compressed[t][d]) % 256;
            }
        }
#else
#ifndef FASTBIT
        int tmp = 0;
        
        for (int d = 0; d < input::D_PM; ++d)
        {
            for (int t = 0; t < pubmed::get_group_by_doc(d); ++t)
            {
                tmp = (tmp + p1_terms_fragments[d][t]) % 256;
            }
        }
        
        for (int t = 0; t < input::T_PM; ++t)
        {
            for (int d = 0; d < pubmed::get_group_by_doc(t); ++d)
            {
                tmp = (tmp + p2_docs_fragments[t][d]) % 256;
            }
        }
#endif
#endif
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
        debug("[T] Thread is running...");
        
        int cntr = 0;
        
        thread_args* args = (thread_args*) vargs;
        
        for (int t = 0; t < args->num_terms; ++t)
        {
            // retrieve doc fragments
            int* doc_fragment_uncompressed;
            int term = args->input_terms[t];
            
#ifdef HUFFMAN
            decode(p2_docs_fragments_compressed[term], pubmed::get_group_by_term(term), doc_fragment_uncompressed, p2_huffman_array, p2_terminator_array);
#else
#ifndef FASTBIT
            doc_fragment_uncompressed = p2_docs_fragments[term];
#endif
#endif
            
#ifndef FASTBIT
            int sz = pubmed::get_group_by_term(term);
            debug("Found " << sz << " documents for term " << term);
            
            for (int d = 0; d < sz; ++d)
            {
                (*args->doc_freq)[doc_fragment_uncompressed[d]]++;
                cntr++;
            }
#endif
            
#ifdef FASTBIT
            ibis::bitvector* doc_fragment = p2_docs_fragments_compressed[term];
            ibis::bitvector::indexSet ones = doc_fragment->firstIndexSet();
            
            while (ones.nIndices() != 0)
            {
                for (int i = 0; i < ones.nIndices(); ++i)
                {
                    (*args->doc_freq)[ones.indices()[i]]++;
                    cntr++;
                }
                
                ++ones;
            }
#endif
            
#ifdef HUFFMAN
            delete[] doc_fragment_uncompressed;
#endif
        }
        
        debug("Thread aggregated " << cntr << " documents.");
        return NULL;
    }
    
    void q1_final_bench()
    {
        int* input_docs = exact_docs_oo[5];
        
        show_info("Running Q1 with 10 repetitions and " << NUM_THREADS << " threads...");
        
        output::start_timer("run/q1_bench");
        
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
            decode(p1_terms_fragments_compressed[doc], term_cnt, terms_uncompressed, p1_huffman_array, p1_terminator_array);
#else
#ifdef FASTBIT
            ibis::bitvector* compressed_fragment = p1_terms_fragments_compressed[doc];
            terms_uncompressed = new unsigned short[term_cnt];
            
            ibis::bitvector::indexSet ones = compressed_fragment->firstIndexSet();
            int one_index = 0;
            
            while (ones.nIndices() != 0)
            {
                for (int i = 0; i < ones.nIndices(); ++i)
                {
                    terms_uncompressed[one_index++] += ones.indices()[i];
                }
                
                ++ones;
            }
#else
            terms_uncompressed = p1_terms_fragments[doc];
#endif
#endif
            
            pthread_t** threads = new pthread_t*[NUM_THREADS];
            thread_args** args = new thread_args*[NUM_THREADS];
            
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                args[t] = new thread_args;
                args[t]->doc_freq = new unordered_map<int, int>(input::D_PM);
                args[t]->num_terms = term_cnt / NUM_THREADS;
                if (args[t]->num_terms > 0)
                {
                    args[t]->input_terms = new unsigned short[args[t]->num_terms];
                    memcpy(args[t]->input_terms, terms_uncompressed + t * args[t]->num_terms, args[t]->num_terms * sizeof(unsigned short));
                }
                
                threads[t] = new pthread_t();
            }
            
            output::start_timer("run/phase2-threads");
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                pthread_create(threads[t], NULL, pthread_q1, (void*) args[t]);
            }
            
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                pthread_join(*threads[t], NULL);
            }
            output::stop_timer("run/phase2-threads");
            
            output::start_timer("run/phase2-aggregate-threads");
            
            if (NUM_THREADS > 1)
            {
                unordered_map<int, int> result(input::D_PM);
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
            else
            {
                // no aggregate necessary
                thread_args* arg = args[0];
                delete arg->doc_freq;
                delete arg->input_terms;
                delete arg;
            }
            output::stop_timer("run/phase2-aggregate-threads");

            
            // TODO: output aggregated map
            output::stop_timer("run/current_rep");
            output::show_stats();
        }
        
        output::stop_timer("run/q1_bench");
        output::show_stats();
    }
}