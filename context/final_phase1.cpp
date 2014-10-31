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
//#define FASTBIT

namespace benchmark
{
    unsigned int* column_doc;
    unsigned short* column_term;
    
#ifdef FASTBIT
    ibis::bitvector** bit_vector_for_term;
#else
    EWAHBoolArray<uint32_t>** bit_vector_for_term;
#endif
    
    void generate_bit_vectors()
    {
        show_info("[1] Generating tuples...");
        column_doc = new unsigned int[input::NUM_TUPLES];
        
#ifdef FASTBIT
        bit_vector_for_term = new ibis::bitvector*[input::T_PM];
#else
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
        
        show_info("[3] Generating bit vectors...");
        unsigned long docs_compressed_bytes = 0;
        next_index = 0;
        
        for (int term = 0; term < input::T_PM; ++term)
        {
#ifdef FASTBIT
            ibis::bitvector* bit_vector = new ibis::bitvector();
#else
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
#else
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
            
            delete doc_list;
#endif
            
            bit_vector_for_term[term] = bit_vector;
            
            if (term % (input::T_PM/1000) == 0) debug_n("  " << term*100.0/input::T_PM << " % complete.    ");
        }
        
        debug_n("  " << 100 << " % complete.    \n");
        show_info("size of compressed bit vectors (bytes): " << docs_compressed_bytes << ".");
        
        delete column_doc;
    }
    
    ibis::bitvector* final_bit_vectors;
    int num_final_bit_vectors;
    
    struct thread_args
    {
#ifdef FASTBIT
        ibis::bitvector* base_vector;
#else
        EWAHBoolArray<uint32_t>* base_vector;
#endif
        int cnt_more_vectors;
    };
    
    void* pthread_bitvector_intersect(void* args_v)
    {
        thread_args* args = (thread_args*) args_v;
        
        for (int a = 0; a < args->cnt_more_vectors; ++a)
        {
#ifdef FASTBIT
            *args->base_vector &= *bit_vector_for_term[rand() % input::T_PM];
#else
            EWAHBoolArray<uint32_t>* base_copy = new EWAHBoolArray<uint32_t>(*args->base_vector);
            base_copy->logicaland(*bit_vector_for_term[rand() % input::T_PM], *args->base_vector);
            delete base_copy;
#endif
        }
        
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
            
            show_info("Running for " << cnt_terms << " terms using 300 repititions.");
            output::start_timer("run/phase1_final");
            for (int r = 0; r < 300; ++r)
            {
                for (int t = 0; t < NUM_THREADS; ++t)
                {
                    //debug("[pthread] Spawning thread " << t << "...");
                    threads[t] = new pthread_t;
                    args[t] = new thread_args;
                    
#ifdef FASTBIT
                    args[t]->base_vector = new ibis::bitvector(*bit_vector_for_term[rand() % input::T_PM]);
#else
                    args[t]->base_vector = new EWAHBoolArray<uint32_t>(*bit_vector_for_term[rand() % input::T_PM]);
#endif
                    
                    args[t]->cnt_more_vectors = cnt_terms / NUM_THREADS - 1;
                    
                    int result = pthread_create(threads[t], NULL, pthread_bitvector_intersect, (void*) args[t]);
                    
                    if (result)
                    {
                        error("Creating thread failed with error code " << result << ".");
                    }
                    
                    //debug("[pthread] Thread is running.");
                }
                
                for (int t = 0; t < NUM_THREADS; ++t)
                {
                    pthread_join(*(threads[t]), NULL);
                    delete threads[t];
                    
                    //debug("Joined thread " << t << ".");
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
#else
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
                
                delete args[0]->base_vector;
                delete args[0];
            }
            
            
            output::stop_timer("run/phase1_final");
            output::show_stats();
        }
        
        debug("mem counter output: " << mem_counter);
    }

}