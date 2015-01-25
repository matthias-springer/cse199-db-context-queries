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
#include <pthread.h>
#include "ewah.h"

#define NUM_THREADS 4
//#define FASTBIT 1
//#define HUFFMAN 1
//#define EWAH
#define UNCOMPRESSED 1

namespace benchmark
{
    int* column_doc;
    unsigned short* column_term;
    
#if defined(UNCOMPRESSED) || defined(HUFFMAN)
    int** docs_per_term;
    int* len_docs_per_term;
    short** exact_terms_b;
#endif
#ifdef HUFFMAN
    char** docs_per_term_compressed;
    int* huffman_array_docs_per_term;
    bool* terminator_array_docs_per_term;
    Node<int>* huffman_tree;
#endif
#ifdef FASTBIT
    ibis::bitvector** bit_vector_for_term;
#endif
#ifdef EWAH
    EWAHBoolArray<uint32_t>** bit_vector_for_term;
#endif
    
    void generate_bit_vectors()
    {
        show_info("[1] Generating tuples...");
        column_doc = new int[input::NUM_TUPLES];
        
#if defined(UNCOMPRESSED) || defined(HUFFMAN)
        docs_per_term = new int*[input::T_PM];
        len_docs_per_term = new int[input::T_PM];
        exact_terms_b = input::terms_bench_items();
        
        int num_terms[6] = {5, 10, 100, 1000, 10000, 25000};
        
        for (int i = 0; i < 6; ++i)
        {
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                // hardcode first term to start intersection with: has 59015 docs (avg is 55715)
                exact_terms_b[i][num_terms[i] * t / NUM_THREADS] = 3492;
            }
        }
#endif
#if defined(HUFFMAN)
        docs_per_term_compressed = new char*[input::T_PM];
        long huffman_encoded_bytes = 0;
#endif
#ifdef FASTBIT
        bit_vector_for_term = new ibis::bitvector*[input::T_PM];
#endif
#ifdef EWAH
        bit_vector_for_term = new EWAHBoolArray<uint32_t>*[input::T_PM];
#endif
        
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
        
#ifdef HUFFMAN
        show_info("[2.5] Generating Huffman tree...");
                   
        generate_array_tree_representation(column_doc, input::NUM_TUPLES, huffman_array_docs_per_term, terminator_array_docs_per_term, huffman_tree);
        encoding_dict<int> encoding_dict_terms;
        build_inverse_mapping(huffman_tree, encoding_dict_terms);
#endif
        
        show_info("[3] Generating bit vectors or compressing...");
        unsigned long docs_compressed_bytes = 0;
        next_index = 0;
        
        for (int term = 0; term < input::T_PM; ++term)
        {
#if defined(UNCOMPRESSED) || defined(HUFFMAN)
            docs_per_term[term] = new int[pubmed::get_group_by_term(term)];
            len_docs_per_term[term] = pubmed::get_group_by_term(term);
            
            for (int i = 0; i < pubmed::get_group_by_term(term); ++i)
            {
                docs_per_term[term][i] = column_doc[next_index++];
            }
#endif
#ifdef FASTBIT
            ibis::bitvector* bit_vector = new ibis::bitvector();
#endif
#ifdef EWAH
            EWAHBoolArray<uint32_t>* bit_vector = new EWAHBoolArray<uint32_t>();
#endif
            
#ifdef FASTBIT
            
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
#endif
#ifdef EWAH
            int cnt_docs = pubmed::get_group_by_term(term);
            
            uint32_t* doc_list = new uint32_t[cnt_docs];
            for (int i = 0; i < cnt_docs; ++i)
            {
                doc_list[i] = column_doc[next_index++];
            }
            
            sort(doc_list, doc_list + cnt_docs);
            
            for (int i = 0; i < cnt_docs; ++i)
            {
                bit_vector->set(doc_list[i]);
            }
            
            docs_compressed_bytes += bit_vector->sizeInBytes();
            
            delete doc_list;
#endif
#if defined(HUFFMAN)
            huffman_encoded_bytes += encode(docs_per_term[term], len_docs_per_term[term], docs_per_term_compressed[term], encoding_dict_terms);
#endif
#if !defined(HUFFMAN) && !defined(UNCOMPRESSED)
            bit_vector_for_term[term] = bit_vector;
#endif
            
            if (term % (input::T_PM/1000) == 0) debug_n("  " << term*100.0/input::T_PM << " % complete.    ");
        }
        
        debug_n("  " << 100 << " % complete.    \n");
        show_info("size of compressed bit vectors (bytes): " << docs_compressed_bytes << ".");
        
#ifdef HUFFMAN
        show_info("size of huffman compressed lists (bytes): " << huffman_encoded_bytes << ".");
#endif
        
        delete column_doc;
    }
    
    ibis::bitvector* final_bit_vectors;
    int num_final_bit_vectors;
    
    struct thread_args
    {
#ifdef FASTBIT
        ibis::bitvector* base_vector;
#endif
#ifdef EWAH
        EWAHBoolArray<uint32_t>* base_vector;
#endif
#if defined(HUFFMAN) || defined(UNCOMPRESSED)
        vector<int>* docs_result;
#endif
        int cnt_more_vectors;
        int p;
        int start;
    };
    
    void* pthread_bitvector_intersect(void* args_v)
    {
        thread_args* args = (thread_args*) args_v;
        
#if !defined(HUFFMAN) && !defined(UNCOMPRESSED)
        for (int a = 0; a < args->cnt_more_vectors; ++a)
        {
#ifdef FASTBIT
            *args->base_vector &= *bit_vector_for_term[rand() % input::T_PM];
#endif
#ifdef EWAH
            EWAHBoolArray<uint32_t>* base_copy = new EWAHBoolArray<uint32_t>(*args->base_vector);
            base_copy->logicaland(*bit_vector_for_term[rand() % input::T_PM], *args->base_vector);
            delete base_copy;
#endif
        }
#else
        // HUFFMAN || UNCOMPRESSED
        int* first_decoded;
        int idx = exact_terms_b[args->p][args->start];
#ifndef HUFFMAN
        first_decoded = docs_per_term[idx];
#else
        decode(docs_per_term_compressed[idx], len_docs_per_term[idx], first_decoded, huffman_array_docs_per_term, terminator_array_docs_per_term);
#endif
        
        vector<int>* result = new vector<int>(first_decoded, first_decoded + len_docs_per_term[idx]);
        int* next_decoded;
        
        for (int i = 1; i < args->cnt_more_vectors; ++i)
        {
            idx = exact_terms_b[args->p][args->start + i];
#ifndef HUFFMAN
            next_decoded = docs_per_term[idx];
#else
            decode(docs_per_term_compressed[idx], len_docs_per_term[idx], next_decoded, huffman_array_docs_per_term, terminator_array_docs_per_term);
#endif
            for (auto it = result->begin(); it != result->end(); )
            {
                if (find(next_decoded, next_decoded + len_docs_per_term[idx], *it) == next_decoded + len_docs_per_term[idx])
                {
                    it = result->erase(it);
                }
                else
                {
                    ++it;
                }
            }
            
#ifndef UNCOMPRESSED
            delete[] next_decoded;
#endif
        }
        
        args->docs_result = result;
#endif
        
        return NULL;
    }
    
    void run_phase1_bench_final()
    {
        int num_terms[6] = {5, 10, 100, 1000, 10000, 25000};
        long mem_counter = 0;   // avoid optimizations
        
        pthread_t** threads = new pthread_t*[NUM_THREADS];
        thread_args** args = new thread_args*[NUM_THREADS];
        
        for (int i = 0; i < 6; ++i)
        {
            int cnt_terms = num_terms[i];
            
            show_info("Running for " << cnt_terms << " terms using 30 repititions.");
            output::start_timer("run/phase1_final");
            for (int r = 0; r < 30; ++r)
            {
                for (int t = 0; t < NUM_THREADS; ++t)
                {
                    //debug("[pthread] Spawning thread " << t << "...");
                    threads[t] = new pthread_t;
                    args[t] = new thread_args;
                    
#ifdef FASTBIT
                    args[t]->base_vector = new ibis::bitvector(*bit_vector_for_term[rand() % input::T_PM]);
#endif
#ifdef EWAH
                    args[t]->base_vector = new EWAHBoolArray<uint32_t>(*bit_vector_for_term[rand() % input::T_PM]);
#endif
                    
                    args[t]->cnt_more_vectors = cnt_terms / NUM_THREADS - 1;
                    args[t]->p = i;
                    args[t]->start = cnt_terms * t / NUM_THREADS;
                    
                    int result = pthread_create(threads[t], NULL, pthread_bitvector_intersect, (void*) args[t]);
                    
                    if (result)
                    {
                        error("Creating thread failed with error code " << result << ".");
                    }
                    
                    debug("[pthread] Thread is running.");
                }
                
                for (int t = 0; t < NUM_THREADS; ++t)
                {
                    pthread_join(*(threads[t]), NULL);
                    delete threads[t];
                    
                    debug("Joined thread " << t << ".");
                }
                
                
#ifdef FASTBIT
                ibis::bitvector* base_vector = args[0]->base_vector;
                
                for (int a = 1; a < NUM_THREADS; ++a)
                {
                    *base_vector &= *args[a]->base_vector;
                    delete args[a]->base_vector;
                    delete args[a];
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
#endif
#ifdef EWAH
                EWAHBoolArray<uint32_t>* base_vector = args[0]->base_vector;
                
                for (int a = 1; a < NUM_THREADS; ++a)
                {
                    EWAHBoolArray<uint32_t>* base_copy = new EWAHBoolArray<uint32_t>(*base_vector);
                    base_copy->logicaland(*args[1]->base_vector, *base_vector);
                    delete base_copy;
                }
                
                // extract ones
                base_vector->toArray();
#endif
#if defined(HUFFMAN) || defined(UNCOMPRESSED)
                vector<int> intersection;
                
                for (int i = 0; i < args[0]->docs_result->size(); ++i)
                {
                    bool not_found = false;
                    int doc_id = args[0]->docs_result->at(i);
                    
                    for (int l = 1; l < NUM_THREADS; ++l)
                    {
                        if (find(args[l]->docs_result->begin(), args[l]->docs_result->end(), doc_id) == args[l]->docs_result->end())
                        {
                            not_found = true;
                            break;
                        }
                    }
                    
                    if (!not_found)
                        intersection.push_back(doc_id);
                    
                }
                
                for (int i = 0; i < NUM_THREADS; ++i)
                {
                    delete args[i]->docs_result;
                }
#endif
                
#if !defined(HUFFMAN) && !defined(UNCOMPRESSED)
                delete args[0]->base_vector;
#endif
                //delete args[0];
            }
            
            
            output::stop_timer("run/phase1_final");
            output::show_stats();
        }
        
        debug("mem counter output: " << mem_counter);
    }

}