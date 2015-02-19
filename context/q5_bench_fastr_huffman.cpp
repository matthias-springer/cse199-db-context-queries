#include "q5_bench_fastr_huffman.h"
#include "pubmed.h"
#include "input.h"
#include "output.h"
#include <algorithm>
#include <random>
#include "huffman.h"
#include <pthread.h>

#define NUM_THREADS 4

using namespace std;

namespace benchmark_q5_huffman
{
    // author --> docs --> terms --> docs --> authors
    //int** docs_per_author;
    //int** terms_per_doc;
    //int** docs_per_term;
    //int** authors_per_doc;
    
    int* t_terms;
    int* t_docs;
    int* t_da_authors;
    int* t_da_docs;
    
    int* docs_per_term_start;
    int* authors_per_doc_start;
    
    Node<int>* tree_terms;
    Node<int>* tree_docs;
    Node<int>* tree_da_authors;
    Node<int>* tree_da_docs;
    int* huffman_array_terms;
    int* huffman_array_docs;
    int* huffman_array_da_authors;
    int* huffman_array_da_docs;
    bool* terminator_array_terms;
    bool* terminator_array_docs;
    bool* terminator_array_da_authors;
    bool* terminator_array_da_docs;
    
    char** terms_per_doc_compressed;
    char** docs_per_term_compressed;
    char** authors_per_doc_compressed;
    char** docs_per_author_compressed;
    
    int* len_docs_per_author;
    int* len_terms_per_doc;
    int* len_docs_per_term;
    int* len_authors_per_doc;
    
    int* compressed_bytes_terms_per_doc;
    int* compressed_bytes_docs_per_term;
    int* compressed_bytes_authors_per_doc;
    int* compressed_bytes_docs_per_author;
    
    long terms_bytes_compressed = 0;
    long docs_bytes_compressed = 0;
    long da_authors_bytes_compressed = 0;
    long da_docs_bytes_compressed = 0;
    long terms_bytes_uncompressed = 0;
    long docs_bytes_uncompressed = 0;
    long da_authors_bytes_uncompressed = 0;
    long da_docs_bytes_uncompressed = 0;
    
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
            
            int* fragment_d2;
            decode(docs_per_term_compressed[term], len_docs_per_term[term], fragment_d2, huffman_array_docs, terminator_array_docs);
            // get fragment and aggregate
            for (int i = 0; i < len_docs_per_term[term]; ++i)
            {
                args->target_array[fragment_d2[i]] += args->previous_counter_array[term];
            }
            delete[] fragment_d2;
            
            /*
            for (int i = 0; i < len_docs_per_term[term]; ++i)
            {
                args->target_array[t_docs[docs_per_term_start[term] + i]] += args->previous_counter_array[term];
            }
             */
        }
        
        return NULL;
    }
    
    void* pthread_step4(void* vargs)
    {
        struct thread_data* args = (struct thread_data*) vargs;
        
        for (int tid = 0; tid < args->num_items; ++tid)
        {
            int doc = args->items[tid];
            
            int* fragment_a1;
            decode(authors_per_doc_compressed[doc], len_authors_per_doc[doc], fragment_a1, huffman_array_da_authors, terminator_array_da_authors);
            // get fragment and aggregate
            for (int i = 0; i < len_authors_per_doc[doc]; ++i)
            {
                args->target_array[fragment_a1[i]] += args->previous_counter_array[doc];
            }
            delete[] fragment_a1;
            
            /*
            for (int i = 0; i < len_authors_per_doc[doc]; ++i)
            {
                args->target_array[t_da_authors[authors_per_doc_start[doc] + i]] += args->previous_counter_array[doc];
            }
             */
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
        
        generate_array_tree_representation(t_terms, input::NUM_TUPLES, huffman_array_terms, terminator_array_terms, tree_terms);
        encoding_dict<int> encoding_dict_terms;
        build_inverse_mapping(tree_terms, encoding_dict_terms);
        
        // split
        index = 0;
        terms_per_doc_compressed = new char*[input::D_PM];
        compressed_bytes_terms_per_doc = new int[input::D_PM];
        len_terms_per_doc = new int[input::D_PM];
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            compressed_bytes_terms_per_doc[doc] = encode(t_terms + index, pubmed::get_group_by_doc(doc), terms_per_doc_compressed[doc], encoding_dict_terms);
            terms_bytes_compressed += compressed_bytes_terms_per_doc[doc];
            terms_bytes_uncompressed += sizeof(int) * pubmed::get_group_by_doc(doc);
            len_terms_per_doc[doc] = pubmed::get_group_by_doc(doc);
            index += pubmed::get_group_by_doc(doc);
        }
        delete[] t_terms;
        delete tree_terms;
        
        
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
        
        generate_array_tree_representation(t_docs, input::NUM_TUPLES, huffman_array_docs, terminator_array_docs, tree_docs);
        encoding_dict<int> encoding_dict_docs;
        build_inverse_mapping(tree_docs, encoding_dict_docs);
        
        // split
        index = 0;
        docs_per_term_compressed = new char*[input::T_PM];
        compressed_bytes_docs_per_term = new int[input::T_PM];
        len_docs_per_term = new int[input::T_PM];
        for (int term = 0; term < input::T_PM; ++term)
        {
            compressed_bytes_docs_per_term[term] = encode(t_docs + index, pubmed::get_group_by_term(term), docs_per_term_compressed[term], encoding_dict_docs);
            docs_bytes_compressed += compressed_bytes_docs_per_term[term];
            docs_bytes_uncompressed += sizeof(int) * pubmed::get_group_by_term(term);
            len_docs_per_term[term] = pubmed::get_group_by_term(term);
            index += pubmed::get_group_by_term(term);
        }
        delete[] t_docs;
        delete tree_docs;
        
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
        
        generate_array_tree_representation(t_da_authors, input::NUM_TUPLES_DA, huffman_array_da_authors, terminator_array_da_authors, tree_da_authors);
        encoding_dict<int> encoding_dict_da_authors;
        build_inverse_mapping(tree_da_authors, encoding_dict_da_authors);
        
        // split
        index = 0;
        authors_per_doc_compressed = new char*[input::D_PM];
        compressed_bytes_authors_per_doc = new int[input::D_PM];
        len_authors_per_doc = new int[input::D_PM];
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            compressed_bytes_authors_per_doc[doc] = encode(t_da_authors + index, pubmed::get_DA_group_by_doc(doc), authors_per_doc_compressed[doc], encoding_dict_da_authors);
            da_authors_bytes_compressed += compressed_bytes_authors_per_doc[doc];
            da_authors_bytes_uncompressed += sizeof(int) * pubmed::get_DA_group_by_doc(doc);
            len_authors_per_doc[doc] = pubmed::get_DA_group_by_doc(doc);
            index += pubmed::get_DA_group_by_doc(doc);
        }
        delete[] t_da_authors;
        delete tree_da_authors;
        
        
        /*
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
        }*/
        
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
        
        generate_array_tree_representation(t_da_docs, input::NUM_TUPLES_DA, huffman_array_da_docs, terminator_array_da_docs, tree_da_docs);
        encoding_dict<int> encoding_dict_da_docs;
        build_inverse_mapping(tree_da_docs, encoding_dict_da_docs);
        
        // split
        index = 0;
        docs_per_author_compressed = new char*[input::A_PM];
        compressed_bytes_docs_per_author = new int[input::A_PM];
        len_docs_per_author = new int[input::A_PM];
        for (int author = 0; author < input::A_PM; ++author)
        {
            compressed_bytes_docs_per_author[author] = encode(t_da_docs, pubmed::get_DA_group_by_author(author), docs_per_author_compressed[author], encoding_dict_da_docs);
            da_docs_bytes_compressed += compressed_bytes_docs_per_author[author];
            da_docs_bytes_uncompressed += sizeof(int) * pubmed::get_DA_group_by_author(author);
            len_docs_per_author[author] = pubmed::get_DA_group_by_author(author);
            index += pubmed::get_DA_group_by_author(author);
        }
        delete[] t_da_docs;
        delete tree_da_docs;
        
        show_info("terms_per_document_bytes_uncompressed: " << terms_bytes_uncompressed);
        show_info("terms_per_document_bytes_compressed: " << terms_bytes_compressed);
        show_info("docs_per_term_bytes_uncompressed: " << docs_bytes_uncompressed);
        show_info("docs_per_term_bytes_compressed: " << docs_bytes_compressed);
        show_info("authors_per_doc_bytes_uncompressed: " << da_authors_bytes_uncompressed);
        show_info("authors_per_doc_bytes_compressed: " << da_authors_bytes_compressed);
        show_info("docs_per_author_bytes_uncompressed: " << da_docs_bytes_uncompressed);
        show_info("docs_per_author_bytes_compressed: " << da_docs_bytes_compressed);
        
        show_info("[5] Anti Swap...");
        int checksum = 0;
        for (int doc = 0; doc < input::D_PM; ++doc)
        {
            for (int i = 0; i < compressed_bytes_terms_per_doc[doc]; ++i)
            {
                checksum += terms_per_doc_compressed[doc][i];
            }
            
            //for (int i = 0; i < compressed_bytes_authors_per_doc[doc]; ++i)
            //{
            //    checksum += authors_per_doc_compressed[doc][i];
            //}
        }
        
        /*
        for (int term = 0; term < input::T_PM; ++term)
        {
            for (int i = 0; i < compressed_bytes_docs_per_term[term]; ++i)
            {
                checksum += docs_per_term_compressed[term][i];
            }
        }*/
        
        for (int author = 0; author < input::A_PM; ++author)
        {
            for (int i = 0; i < compressed_bytes_docs_per_author[author]; ++i)
            {
                checksum += docs_per_author_compressed[author][i];
            }
        }
        
        show_info("Checksum: " << checksum);
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
        int* fragment_d1;
        decode(docs_per_author_compressed[author], len_docs_per_author[author], fragment_d1, huffman_array_da_docs, terminator_array_da_docs);
        
        for (int i = 0; i < len_docs_per_author[author]; ++i)
        {
            int doc = fragment_d1[i];
            // if (docs_1_counter[doc] > 0)
            {
                int* fragment_t1;
                decode(terms_per_doc_compressed[doc], len_terms_per_doc[doc], fragment_t1, huffman_array_terms, terminator_array_terms);
                // get fragment and aggregate
                for (int i = 0; i < len_terms_per_doc[doc]; ++i)
                {
                    terms_2_counter[fragment_t1[i]]++; //+= docs_1_counter[doc];
                }
                delete[] fragment_t1;
            }
        }
        delete[] fragment_d1;
        
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
        
        //show_info("checksum: " << checksum);
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