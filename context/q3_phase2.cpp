#include "q3_phase2.h"
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
#include <bitvector.h>
#include <ibis.h>
#include <pthread.h>

#define s_compress 1
//#define use_fastbit 1
//#define s_compress false
//#define use_fastbit false

#define NUM_THREADS 4

using namespace std;

// this is a benchmark for Phase 2

namespace benchmark_q3
{
    int* tuples;
    
    int** terms_per_doc;
    char** terms_per_doc_compressed;
    ibis::bitvector** terms_per_doc_bitvector;
    
    int* terms_per_doc_size;
    Node<int>* tree;
    
    int* huffman_array_terms;
    bool* terminator_array_terms;
    
    long terms_bytes_uncompressed = 0;
    long terms_bytes_compressed = 0;
    
    int** exact_terms_a;
    int** exact_docs_a;
    
    void huffman_query_generate_lists()
    {
        output::start_timer("run/bench_q3_fastr_generate");
        
        // generate tuples
        show_info("[1] Generating tuples...");
        show_info("Allocating " << input::NUM_TUPLES_DA << " ints/chars (array).");
        tuples = new int[input::NUM_TUPLES_DA];
        show_info("Alloc success");
        
        long next_index = 0;
        
        for (int t = 0; t < input::A_PM; ++t)
        {
            for (long c = 0; c < pubmed::get_DA_group_by_author(t); ++c)
            {
                tuples[next_index++] = t;
            }
            
            if (t % (input::A_PM/1000) == 0) debug_n("  " << t*100.0/input::A_PM << " % complete.    ");
        }
        debug_n("  " << 100 << " % complete.    \n");
        
        // shuffle
        show_info("[2] Shuffling...");
        shuffle(tuples, tuples + input::NUM_TUPLES_DA, default_random_engine(42));
        
        // generate terms per doc
        show_info("[3] Generating separate lists...");
        terms_per_doc = new int*[input::D_PM];
        terms_per_doc_size = new int[input::D_PM];
        
        next_index = 0;
        for (long d = 0; d < input::D_PM; ++d)
        {
            int* list = new int[pubmed::get_DA_group_by_doc(d)];
            memcpy(list, tuples + next_index, pubmed::get_DA_group_by_doc(d) * sizeof(int));
            terms_per_doc[d] = list;
            
            terms_per_doc_size[d] = pubmed::get_DA_group_by_doc(d);
            
            if (d % (input::D_PM/1000) == 0) debug_n("  " << d*100.0/input::D_PM << " % complete.    ");
        }
        debug_n("  " << 100 << " % complete.    \n");
        
        
#ifdef s_compress
#ifndef use_fastbit
        // compress with Huffman
        show_info("[4] Generating Huffman tree for authors...");
        terms_per_doc_compressed = new char*[input::D_PM];
        
        generate_array_tree_representation(tuples, input::NUM_TUPLES_DA, huffman_array_terms, terminator_array_terms, tree);
        encoding_dict<int> encoding_dict_terms;
        build_inverse_mapping(tree, encoding_dict_terms);
        
        delete(tuples);
        
        show_info("[5] Compressing authors...");
        for (long d = 0; d < input::D_PM; ++d)
        {
            char* terms_compressed;
            terms_bytes_uncompressed += terms_per_doc_size[d] * sizeof(int);
            terms_bytes_compressed += encode(terms_per_doc[d], terms_per_doc_size[d], terms_compressed, encoding_dict_terms);
            terms_per_doc_compressed[d] = terms_compressed;
            delete terms_per_doc[d];
            
            if (d % (input::D_PM/1000) == 0) debug_n("  " << d*100.0/input::D_PM << " % complete.    ");
        }
        debug_n("  " << 100 << " % complete.    \n");
#else
        // compress with FastBit
        show_info("[4] Compressing terms with bit vector...");
        terms_per_doc_bitvector = new ibis::bitvector*[input::D_PM];
        
        delete tuples;
        
        for (long d = 0; d < input::D_PM; ++d)
        {
            ibis::bitvector* terms_compressed = new ibis::bitvector();
            terms_bytes_uncompressed += terms_per_doc_size[d] * sizeof(int);
            
            for (long term_index = 0; term_index < terms_per_doc_size[d]; ++ term_index)
            {
                terms_compressed->setBit(terms_per_doc[d][term_index], 1);
            }
            
            
            terms_compressed->compress();
            // force new allocate
            ibis::array_t<uint32_t>* arr = new ibis::array_t<uint32_t>();
            terms_compressed->write(*arr);
            delete terms_compressed;
            terms_compressed = new ibis::bitvector(*arr);
            delete arr;
            
            delete terms_per_doc[d];
            
            terms_per_doc_bitvector[d] = terms_compressed;
            terms_bytes_compressed += terms_compressed->bytes();
            
            
            if (d % (input::D_PM/1000) == 0)
            {
                debug_n("  " << d*100.0/input::D_PM << " % complete. Using " << terms_bytes_compressed << " / " << terms_bytes_uncompressed << " bytes.   ");
            }
        }
        
        debug_n("  " << 100 << " % complete.    \n");
        
        show_info("[5] n/a");
#endif
        

        delete terms_per_doc;
        
        show_info("authors bytes uncompressed: " << terms_bytes_uncompressed);
        show_info("authors bytes compressed: " << terms_bytes_compressed);
#else
        delete(tuples);
        show_info("No compression.");
#endif
        
        exact_docs_a = input::docs_bench_items();
        
        output::stop_timer("run/bench_q3_fastr_generate");
        
        show_info("Done.");
    }
    
    struct thread_args
    {
        unordered_map<int, int>* term_counter;
        long num_docs;
        int p;
        int start;
    };
    
    void* pthread_query(void* args_v)
    {
        int counter = 0;
        thread_args* args = (thread_args*) args_v;
        
        for (long di = 0; di < args->num_docs; ++di)
        {
            // retrieve list and add to term counter
            long doc_id = exact_docs_a[args->p][di + args->start];
            
            //int* terms = terms_per_doc[doc_id];
            int list_size = terms_per_doc_size[doc_id];
            
            int* terms_decompressed;
            
#ifdef s_compress
#ifndef use_fastbit
            decode(terms_per_doc_compressed[doc_id], list_size, terms_decompressed, huffman_array_terms, terminator_array_terms);
#endif
            
#else
            terms_decompressed = terms_per_doc[doc_id];
#endif
            
#if defined(s_compress) && defined(use_fastbit)
            long term_index = 0;
            ibis::bitvector::indexSet ones = terms_per_doc_bitvector[doc_id]->firstIndexSet();
            bool is_range = ones.isRange();
            
            while (ones.nIndices() != 0)
            {
                for (int i = 0; i < ones.nIndices(); ++i)
                {
                    int term;
                    if (is_range)
                    {
                        term = ones.indices()[0] + i;
                    }
                    else
                    {
                        term = ones.indices()[i];
                    }
                    
                    (*args->term_counter)[term]++;
                    ++term_index;
                }
                
                ++ones;
            }
#else
            for (int l = 0; l < list_size; ++l)
            {
                (*args->term_counter)[terms_decompressed[l]]++;
                //term_counter[terms[l]]++;
                counter++;
            }
#endif
            
#ifdef s_compress
#ifndef use_fastbit
            delete terms_decompressed;
#endif
            
#endif
        }
        
        debug("Thread aggregated " << counter << " authors.");
        return NULL;
    }
    
    void huffman_query_run_benchmark()
    {
        int num_docs[7] = {10, 100, 1000, 10000, 100000, 1000000};
        
        pthread_t** threads = new pthread_t*[NUM_THREADS];
        thread_args** args = new thread_args*[NUM_THREADS];
        
        for (int i = 0; i < 6; ++i)
        {
            show_info("Running benchmark for " << num_docs[i] << " documents (300 iterations).");
            output::start_timer("run/bench_q3_fastr");
            
            for (int j = 0; j < 300; ++j)
            {
                unordered_map<int, int>* term_counter = new unordered_map<int, int>[4]();
                //unordered_map<int, long> term_counter;
                
                for (int t = 0; t < NUM_THREADS; ++t)
                {
                    debug("Spawing thread " << t << "...");
                    
                    threads[t] = new pthread_t;
                    args[t] = new thread_args;
                    args[t]->num_docs = num_docs[i] / NUM_THREADS;
                    args[t]->term_counter = &term_counter[t];
                    args[t]->p = i;
                    args[t]->start = num_docs[i] * t / NUM_THREADS;
                    
                    int result = pthread_create(threads[t], NULL, pthread_query, (void*) args[t]);
                    
                    if (result)
                    {
                        error("Creating thread failed with error code " << result << ".");
                    }
                    
                    debug("Thread is running.");
                }
                
                for (int t = 0; t < NUM_THREADS; ++t)
                {
                    pthread_join(*(threads[t]), NULL);
                    delete args[t];
                    delete threads[t];
                    
                    debug("Joined thread " << t << ".");
                }
                
                // aggregate lists and sort list and extract top-k
                for (int t = 1; t < NUM_THREADS; ++t)
                {
                    for (auto el = term_counter[t].begin(); el != term_counter[t].end(); ++el)
                    {
                        term_counter[0][el->first] += el->second;
                    }
                }
                
                //term_counter[0].top_k(5);
                //delete term_counter;
            }
            
            // stats
            output::stop_timer("run/bench_q3_fastr");
            output::show_stats();
            output::clear_stats();
        }
        
    }
}
