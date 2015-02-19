#include "q5_bench_fastr.h"
#include "pubmed.h"
#include "input.h"
#include "output.h"
#include <algorithm>
#include <random>
#include <pthread.h>

#define NUM_THREADS 4
using namespace std;

namespace benchmark_q5
{
    // author --> docs --> terms --> docs --> authors
    //int** docs_per_author;
    //int** terms_per_doc;
    //int** docs_per_term;
    //int** authors_per_doc;
    
    int* t_terms;
    int* terms_per_doc_start;
    int* t_docs;
    int* docs_per_term_start;
    int* t_da_authors;
    int* authors_per_doc_start;
    int* t_da_docs;
    int* docs_per_author_start;
    
    int* len_docs_per_author;
    int* len_terms_per_doc;
    int* len_docs_per_term;
    int* len_authors_per_doc;
    
    struct thread_data
    {
        int num_items;
        int* items;
        int* target_array;
        int* previous_counter_array;
    };
    
    void* pthread_step3(void* vargs)
    {
        struct thread_data* args = (struct thread_data*) vargs;
        
        for (int tid = 0; tid < args->num_items; ++tid)
        {
            int term = args->items[tid];
            
            for (int i = 0; i < len_docs_per_term[term]; ++i)
            {
                args->target_array[t_docs[docs_per_term_start[term] + i]] += args->previous_counter_array[term];
            }
        }
        
        return NULL;
    }
    
    void* pthread_step4(void* vargs)
    {
        struct thread_data* args = (struct thread_data*) vargs;
        
        for (int tid = 0; tid < args->num_items; ++tid)
        {
            int doc = args->items[tid];
            
            for (int i = 0; i < len_authors_per_doc[doc]; ++i)
            {
                args->target_array[t_da_authors[authors_per_doc_start[doc] + i]] += args->previous_counter_array[doc];
            }
        }
        
        return NULL;
    }
    
    void generate_tuples_q5_fastr()
    {
        // scale
        int index = 0;
        
        show_info("[1] Generating terms per doc fragments...");
        t_terms = new int[input::NUM_TUPLES];
        
        for (int term = 0; term < input::T_PM; ++term)
        {
            for (int i = 0; i < pubmed::get_group_by_term(term); ++i)
            {
                t_terms[index++] = term;
            }
        }
        
        shuffle(t_terms, t_terms + input::NUM_TUPLES, default_random_engine(42));
        
        // split
        index = 0;
        terms_per_doc_start = new int[input::D_PM];
        len_terms_per_doc = new int[input::D_PM];
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            terms_per_doc_start[doc] = index;
            len_terms_per_doc[doc] = pubmed::get_group_by_doc(doc);
            index += pubmed::get_group_by_doc(doc);
        }
        
        show_info("[2] Generating docs per term fragments...");
        t_docs = new int[input::NUM_TUPLES];
        index = 0;
        
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            for (int i = 0; i < pubmed::get_group_by_doc(doc); ++i)
            {
                t_docs[index++] = doc;
            }
        }
        
        shuffle(t_docs, t_docs + input::NUM_TUPLES, default_random_engine(42));
        
        // split
        index = 0;
        docs_per_term_start = new int[input::T_PM];
        len_docs_per_term = new int[input::T_PM];
        for (int term = 0; term < input::T_PM; ++term)
        {
            docs_per_term_start[term] = index;
            len_docs_per_term[term] = pubmed::get_group_by_term(term);
            index += pubmed::get_group_by_term(term);
        }
        
        show_info("[3] Generate authors per doc fragments...");
        t_da_authors = new int[input::NUM_TUPLES_DA];
        index = 0;
        
        for (int author = 0; author < input::A_PM; ++author)
        {
            for (int i = 0; i < pubmed::get_DA_group_by_author(author); ++i)
            {
                t_da_authors[index++] = author;
            }
        }
        
        shuffle(t_da_authors, t_da_authors + input::NUM_TUPLES_DA, default_random_engine(42));
        
        // split
        index = 0;
        authors_per_doc_start = new int[input::D_PM];
        len_authors_per_doc = new int[input::D_PM];
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            authors_per_doc_start[doc] = index;
            len_authors_per_doc[doc] = pubmed::get_DA_group_by_doc(doc);
            index += pubmed::get_DA_group_by_doc(doc);
        }
        
        show_info("[4] Generate docs per author fragments...");
        t_da_docs = new int[input::NUM_TUPLES_DA];
        index = 0;
        
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            for (int i = 0; i < pubmed::get_DA_group_by_doc(doc); ++i)
            {
                t_da_docs[index++] = doc;
            }
        }
        
        shuffle(t_da_docs, t_da_docs + input::NUM_TUPLES_DA, default_random_engine(42));
        
        // split
        index = 0;
        docs_per_author_start = new int[input::A_PM];
        len_docs_per_author = new int[input::A_PM];
        for (int author = 0; author < input::A_PM; ++author)
        {
            docs_per_author_start[author] = index;
            len_docs_per_author[author] = pubmed::get_DA_group_by_author(author);
            index += pubmed::get_DA_group_by_author(author);
        }
    }
    
    void run_query_q5_fastr(int author)
    {
        int* docs_1_counter = new int[input::D_PM]();
        int* terms_2_counter = new int[input::T_PM]();
        int* docs_3_counter = new int[input::D_PM]();
        int* authors_4_counter = new int[input::A_PM]();
        
        /*
        for (int i = 0; i < len_docs_per_author[author]; ++i)
        {
            docs_1_counter[t_da_docs[docs_per_author_start[author] + i]]++;
        }
        */
        
        //for (int doc = 0; doc < input::D_PM; ++doc)
        for (int i = 0; i < len_docs_per_author[author]; ++i)
        {
            int doc = t_da_docs[docs_per_author_start[author] + i];
            // if (docs_1_counter[doc] > 0)
            {
                // get fragment and aggregate
                for (int i = 0; i < len_terms_per_doc[doc]; ++i)
                {
                    terms_2_counter[t_terms[terms_per_doc_start[doc] + i]]++; //+= docs_1_counter[doc];
                }
            }
        }
        
        int next_thread = 0;
        struct thread_data* targs = new struct thread_data[NUM_THREADS];
        pthread_t* threads = new pthread_t[NUM_THREADS]();
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            targs[i].items = new int[input::T_PM];
            targs[i].num_items = 0;
            targs[i].previous_counter_array = terms_2_counter;
            targs[i].target_array = docs_3_counter;
        }
        for (int term = 0; term < input::T_PM; ++term)
        {
            if (terms_2_counter[term] > 0)
            {
                /*
                 int* fragment_d2;
                 decode(docs_per_term_compressed[term], len_docs_per_term[term], fragment_d2, huffman_array_docs, terminator_array_docs);
                 // get fragment and aggregate
                 for (int i = 0; i < len_docs_per_term[term]; ++i)
                 {
                 docs_3_counter[fragment_d2[i]] += terms_2_counter[term];
                 }
                 delete[] fragment_d2;
                 */
                
                // get fragment and aggregate
                /*
                 for (int i = 0; i < len_docs_per_term[term]; ++i)
                 {
                 docs_3_counter[t_docs[docs_per_term_start[term] + i]] += terms_2_counter[term];
                 }*/
                
                targs[next_thread].items[targs[next_thread].num_items++] = term;
                next_thread = (next_thread + 1) % NUM_THREADS;
            }
        }
        
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            pthread_create(threads + i, NULL, pthread_step3, (void*) (targs + i));
        }
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            pthread_join(*(threads + i), NULL);
            delete[] targs[i].items;
        }
        
        int checksum = 0;
        next_thread = 0;
        threads = new pthread_t[NUM_THREADS]();
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            targs[i].items = new int[input::D_PM];
            targs[i].num_items = 0;
            targs[i].previous_counter_array = docs_3_counter;
            targs[i].target_array = authors_4_counter;
        }
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            if (docs_3_counter[doc] > 0)
            {
                /*
                 int* fragment_a1;
                 decode(authors_per_doc_compressed[doc], len_authors_per_doc[doc], fragment_a1, huffman_array_da_authors, terminator_array_da_authors);
                 // get fragment and aggregate
                 for (int i = 0; i < len_authors_per_doc[doc]; ++i)
                 {
                 authors_4_counter[fragment_a1[i]] += docs_3_counter[doc];
                 }
                 delete[] fragment_a1;
                 */
                
                // get fragment and aggregate
                //for (int i = 0; i < len_authors_per_doc[doc]; ++i)
                //{
                //    authors_4_counter[t_da_authors[authors_per_doc_start[doc] + i]] += docs_3_counter[doc];
                //    checksum += docs_3_counter[doc];
                //}
                targs[next_thread].items[targs[next_thread].num_items++] = doc;
                next_thread = (next_thread + 1) % NUM_THREADS;
            }
        }
        
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            pthread_create(threads + i, NULL, pthread_step4, (void*) (targs + i));
        }
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            pthread_join(*(threads + i), NULL);
            delete[] targs[i].items;
        }
    }
    
    void run_bench_q5_fastr()
    {
        int* authors = new int[1000];
        for (int i = 0; i < 1000; ++i)
        {
            authors[i] = rand() % input::A_PM;
        }
        
        show_info("Running with " << NUM_THREADS << " threads...");
        
        generate_tuples_q5_fastr();
        
        int bench_size = 20;
        show_info("Running Q5 with " << bench_size << " authors.");
        
        output::start_timer("run/bench_q5_fastr");
        for (int i = 0; i < bench_size; ++i)
        {
            run_query_q5_fastr(authors[i]);
            show_info("DONE (" << i << ")");
        }
        
        output::stop_timer("run/bench_q5_fastr");
        output::show_stats();
    }
}