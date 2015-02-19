#include "q5_bench_omc.h"
#include <algorithm>
#include <random>
#include "input.h"
#include "output.h"
#include "pubmed.h"
#include <pthread.h>

#define NUM_THREADS 4

namespace benchmark_q5_omc
{
    struct rle_tuple
    {
        int id;
        int length;
        int row_id;
    };
    
    int* t_terms_per_doc;
    rle_tuple* t_terms_per_doc_docs;
    
    int* t_docs_per_term;
    rle_tuple* t_docs_per_term_terms;
    
    int* t_authors_per_doc;
    rle_tuple* t_authors_per_doc_docs;
    
    int* t_docs_per_author;
    rle_tuple* t_docs_per_author_authors;
    
    rle_tuple* binary_search(rle_tuple* container, int start, int end, int id)
    {
        int middle = (start+end) / 2;
        while (container[middle].id != id)
        {
            middle = (start + end)/2;
            if (container[middle].id < id)
            {
                start = middle;
            }
            else
            {
                end = middle;
            }
        }
        
        return container + middle;
    }
    
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
            
            // go terms->docs
            rle_tuple* term_rle_tuple = binary_search(t_docs_per_term_terms, 0, input::T_PM, term);
            int* docs_per_term_row_ids = new int[term_rle_tuple->length];
            for (int j = 0; j < term_rle_tuple->length; ++j)
            {
                docs_per_term_row_ids[j] = j + term_rle_tuple->row_id;
            }
            int* docs_per_term = new int[term_rle_tuple->length];
            for (int j = 0; j < term_rle_tuple->length; ++j)
            {
                docs_per_term[j] = t_docs_per_term[docs_per_term_row_ids[j]];
            }
            delete[] docs_per_term_row_ids;
            for (int j = 0; j < term_rle_tuple->length; ++j)
            {
                args->target_array[docs_per_term[j]] += args->previous_counter_array[term];
            }
            delete[] docs_per_term;
        }
        
        return NULL;
    }
    
    void* pthread_step4(void* vargs)
    {
        struct thread_data* args = (struct thread_data*) vargs;
        
        for (int tid = 0; tid < args->num_items; ++tid)
        {
            int doc = args->items[tid];
            
            rle_tuple* doc_rle_tuple = binary_search(t_authors_per_doc_docs, 0, input::D_PM, doc);
            int* authors_per_doc_row_ids = new int[doc_rle_tuple->length];
            for (int j = 0; j < doc_rle_tuple->length; ++j)
            {
                authors_per_doc_row_ids[j] = j + doc_rle_tuple->row_id;
            }
            int* authors_per_doc = new int[doc_rle_tuple->length];
            for (int j = 0; j < doc_rle_tuple->length; ++j)
            {
                authors_per_doc[j] = t_authors_per_doc[authors_per_doc_row_ids[j]];
            }
            delete[] authors_per_doc_row_ids;
            for (int j = 0; j < doc_rle_tuple->length; ++j)
            {
                args->target_array[authors_per_doc[j]] += args->previous_counter_array[doc];
            }
            delete[] authors_per_doc;
        }
        
        return NULL;
    }
    
    void generate_tuples_q5_omc()
    {
        // scale
        int index = 0;
        
        show_info("[1] Generating terms per doc fragments...");
        t_terms_per_doc = new int[input::NUM_TUPLES];
        
        for (int term = 0; term < input::T_PM; ++term)
        {
            for (int i = 0; i < pubmed::get_group_by_term(term); ++i)
            {
                t_terms_per_doc[index++] = term;
            }
        }
        
        shuffle(t_terms_per_doc, t_terms_per_doc + input::NUM_TUPLES, default_random_engine(42));
        
        // split
        index = 0;
        t_terms_per_doc_docs = new rle_tuple[input::D_PM];
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            t_terms_per_doc_docs[doc].row_id = index;
            t_terms_per_doc_docs[doc].length = pubmed::get_group_by_doc(doc);
            t_terms_per_doc_docs[doc].id = doc;
            index += pubmed::get_group_by_doc(doc);
        }
        
        show_info("[2] Generating docs per term fragments...");
        t_docs_per_term = new int[input::NUM_TUPLES];
        index = 0;
        
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            for (int i = 0; i < pubmed::get_group_by_doc(doc); ++i)
            {
                t_docs_per_term[index++] = doc;
            }
        }
        
        shuffle(t_docs_per_term, t_docs_per_term + input::NUM_TUPLES, default_random_engine(42));
        
        // split
        index = 0;
        t_docs_per_term_terms = new rle_tuple[input::T_PM];
        for (int term = 0; term < input::T_PM; ++term)
        {
            t_docs_per_term_terms[term].row_id = index;
            t_docs_per_term_terms[term].length = pubmed::get_group_by_term(term);
            t_docs_per_term_terms[term].id = term;
            index += pubmed::get_group_by_term(term);
        }
        
        show_info("[3] Generate authors per doc fragments...");
        t_authors_per_doc = new int[input::NUM_TUPLES_DA];
        index = 0;
        
        for (int author = 0; author < input::A_PM; ++author)
        {
            for (int i = 0; i < pubmed::get_DA_group_by_author(author); ++i)
            {
                t_authors_per_doc[index++] = author;
            }
        }
        
        shuffle(t_authors_per_doc, t_authors_per_doc + input::NUM_TUPLES_DA, default_random_engine(42));
        
        // split
        index = 0;
        t_authors_per_doc_docs = new rle_tuple[input::D_PM];
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            t_authors_per_doc_docs[doc].row_id = index;
            t_authors_per_doc_docs[doc].length = pubmed::get_DA_group_by_doc(doc);
            t_authors_per_doc_docs[doc].id = doc;
            index += pubmed::get_DA_group_by_doc(doc);
        }
        
        show_info("[4] Generate docs per author fragments...");
        t_docs_per_author = new int[input::NUM_TUPLES_DA];
        index = 0;
        
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            for (int i = 0; i < pubmed::get_DA_group_by_doc(doc); ++i)
            {
                t_docs_per_author[index++] = doc;
            }
        }
        
        shuffle(t_docs_per_author, t_docs_per_author + input::NUM_TUPLES_DA, default_random_engine(42));
        
        // split
        index = 0;
        t_docs_per_author_authors = new rle_tuple[input::A_PM];
        for (int author = 0; author < input::A_PM; ++author)
        {
            t_docs_per_author_authors[author].row_id = index;
            t_docs_per_author_authors[author].length = pubmed::get_DA_group_by_author(author);
            t_docs_per_author_authors[author].id = author;
            index += pubmed::get_DA_group_by_author(author);
        }
    }
    
    void run_query_q5(int author)
    {
        // get docs for authors
        rle_tuple* author_rle_tuple = binary_search(t_docs_per_author_authors, 0, input::A_PM, author);
        int* docs_per_author_row_ids = new int[author_rle_tuple->length];
        int* authors_per_doc_counter = new int[input::D_PM];
        for (int i = 0; i < author_rle_tuple->length; ++i)
        {
            docs_per_author_row_ids[i] = i + author_rle_tuple->row_id;
        }
        int* docs_per_author = new int[author_rle_tuple->length];
        for (int i = 0; i < author_rle_tuple->length; ++i)
        {
            docs_per_author[i] = t_docs_per_author[docs_per_author_row_ids[i]];
        }
        delete[] docs_per_author_row_ids;
        for (int i = 0; i < author_rle_tuple->length; ++i)
        {
            authors_per_doc_counter[docs_per_author[i]]++;
        }
        delete[] docs_per_author;
        
        // get #docs per term(s)
        int* docs_per_term_counter = new int[input::T_PM]();
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            if (authors_per_doc_counter[doc] > 0)
            {
                rle_tuple* doc_rle_tuple = binary_search(t_terms_per_doc_docs, 0, input::D_PM, doc);
                int* terms_per_doc_row_ids = new int[doc_rle_tuple->length];
                for (int j = 0; j < doc_rle_tuple->length; ++j)
                {
                    terms_per_doc_row_ids[j] = j + doc_rle_tuple->row_id;
                }
                int* terms_per_doc = new int[doc_rle_tuple->length];
                for (int j = 0; j < doc_rle_tuple->length; ++j)
                {
                    terms_per_doc[j] = t_terms_per_doc[terms_per_doc_row_ids[j]];
                }
                delete[] terms_per_doc_row_ids;
                for (int j = 0; j < doc_rle_tuple->length; ++j)
                {
                    docs_per_term_counter[terms_per_doc[j]] += authors_per_doc_counter[doc];
                }
                delete[] terms_per_doc;
            }
        }
        
        /*
        // get #terms per doc(s)
        int* terms_per_doc_counter = new int[input::D_PM]();
        for (int term = 0; term < input::T_PM; ++term)
        {
            if (docs_per_term_counter[term] > 0)
            {
                // go terms->docs
                rle_tuple* term_rle_tuple = binary_search(t_docs_per_term_terms, 0, input::T_PM, term);
                int* docs_per_term_row_ids = new int[term_rle_tuple->length];
                for (int j = 0; j < term_rle_tuple->length; ++j)
                {
                    docs_per_term_row_ids[j] = j + term_rle_tuple->row_id;
                }
                int* docs_per_term = new int[term_rle_tuple->length];
                for (int j = 0; j < term_rle_tuple->length; ++j)
                {
                    docs_per_term[j] = t_docs_per_term[docs_per_term_row_ids[j]];
                }
                delete[] docs_per_term_row_ids;
                for (int j = 0; j < term_rle_tuple->length; ++j)
                {
                    terms_per_doc_counter[docs_per_term[j]] += docs_per_term_counter[term];
                }
                delete[] docs_per_term;
            }
        }
        delete[] docs_per_term_counter;
        
        // get #docs per author
        int* docs_per_author_counter = new int[input::A_PM]();
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            if (terms_per_doc_counter[doc] > 0)
            {
                rle_tuple* doc_rle_tuple = binary_search(t_authors_per_doc_docs, 0, input::D_PM, doc);
                int* authors_per_doc_row_ids = new int[doc_rle_tuple->length];
                for (int j = 0; j < doc_rle_tuple->length; ++j)
                {
                    authors_per_doc_row_ids[j] = j + doc_rle_tuple->row_id;
                }
                int* authors_per_doc = new int[doc_rle_tuple->length];
                for (int j = 0; j < doc_rle_tuple->length; ++j)
                {
                    authors_per_doc[j] = t_authors_per_doc[authors_per_doc_row_ids[j]];
                }
                delete[] authors_per_doc_row_ids;
                for (int j = 0; j < doc_rle_tuple->length; ++j)
                {
                    docs_per_author_counter[authors_per_doc[j]] += terms_per_doc_counter[doc];
                }
                delete[] authors_per_doc;
            }
        }
        delete[] terms_per_doc_counter;
         */
        
        int* terms_per_doc_counter = new int[input::D_PM]();
        int next_thread = 0;
        struct thread_data* targs = new struct thread_data[NUM_THREADS];
        pthread_t* threads = new pthread_t[NUM_THREADS]();
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            targs[i].items = new int[input::T_PM];
            targs[i].num_items = 0;
            targs[i].previous_counter_array = docs_per_term_counter;
            targs[i].target_array = terms_per_doc_counter;
        }
        for (int term = 0; term < input::T_PM; ++term)
        {
            if (docs_per_term_counter[term] > 0)
            {
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
        delete[] docs_per_term_counter;
        
        int checksum = 0;
        int* docs_per_author_counter = new int[input::A_PM]();
        next_thread = 0;
        threads = new pthread_t[NUM_THREADS]();
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            targs[i].items = new int[input::D_PM];
            targs[i].num_items = 0;
            targs[i].previous_counter_array = terms_per_doc_counter;
            targs[i].target_array = docs_per_author_counter;
        }
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            if (terms_per_doc_counter[doc] > 0)
            {
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
        delete[] terms_per_doc_counter;
    }
    
    void run_bench_q5_omc()
    {
        int* authors = new int[1000];
        for (int i = 0; i < 1000; ++i)
        {
            authors[i] = rand() % input::A_PM;
        }
        
        generate_tuples_q5_omc();
        
        show_info("Running with " << NUM_THREADS << " threads...");
        
        int bench_size = 20;
        show_info("Running Q5 with " << bench_size << " authors.");
        
        output::start_timer("run/bench_q5_omc");
        for (int i = 0; i < bench_size; ++i)
        {
            run_query_q5(authors[i]);
            show_info("DONE (" << i << ")");
        }
        
        output::stop_timer("run/bench_q5_omc");
        output::show_stats();
    }
    
}