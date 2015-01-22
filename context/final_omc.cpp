#include "final_omc.h"
#include "pubmed.h"
#include "output.h"
#include "input.h"
#include <unordered_map>
#include <pthread.h>
#include "aggregation.h"
#include "map_aggregation.h"
#include <random>

namespace benchmark
{
#define NUM_THREADS 1
    
    template<typename id_t, typename len_t>
    struct rle_tuple
    {
        id_t id;
        len_t length;
        int row_id;
    };
    
    // DT_1
    int* dt1_docs;
    rle_tuple<int, int>* dt1_terms;
    int** exact_terms;
    
    // DT_2
    rle_tuple<int, int>* dt2_docs;
    int* dt2_terms;
    unsigned char* dt2_freqs;
    int** exact_docs;
    
    // input
    int** exact_docs_ooo;
    
    struct thread_args_omc_p1
    {
        int* arr_t;
        int len_arr_t;
        int** arr_d;
        int* len_arr_d;
    };
    
    void* q1_omc_final_pthread(void* args)
    {
        thread_args_omc_p1* t_args = (thread_args_omc_p1*) args;
        
        if (t_args->len_arr_t > 0)
        {
            t_args->len_arr_d = new int[t_args->len_arr_t];
            t_args->arr_d = new int*[t_args->len_arr_t];
        }
        
        for (int t = 0; t < t_args->len_arr_t; ++t)
        {
            // find RLE-encoded tuple (DT_1)
            int start = 0;
            int end = input::T_PM;
            int middle = (start+end) / 2;
            while (dt1_terms[middle].id != t_args->arr_t[t])
            {
                middle = (start + end)/2;
                if (dt1_terms[middle].id < t_args->arr_t[t])
                {
                    start = middle;
                }
                else
                {
                    end = middle;
                }
            }
            
            // build arr_d
            rle_tuple<int, int> tuple = dt1_terms[middle];
            debug("Found " << tuple.length << " documents for term " << t_args->arr_t[t]);
            
            t_args->arr_d[t] = new int[tuple.length];
            t_args->len_arr_d[t] = tuple.length;
            
            for (int i = 0; i < tuple.length; ++i)
            {
                t_args->arr_d[t][i] = dt1_docs[i + tuple.row_id];
            }
        }
        
        return NULL;
    }
    
    void q1_omc_final_bench()
    {
        int* input_docs = exact_docs_ooo[5];
        
        show_info("Running Q1 with 10 repetitions and " << NUM_THREADS << " threads...");
        
        output::start_timer("run/q1_omc_bench");
        
        for (int r = 0; r < 10; ++r)
        {
            output::start_timer("run/current_rep");
            
            int doc = input_docs[r];
        
            // binary search to find RLE-encoded tuple (DT_2)
            int start = 0;
            int end = input::D_PM;
            int middle = (start+end) / 2;
            while (dt2_docs[middle].id != doc)
            {
                middle = (start + end)/2;
                if (dt2_docs[middle].id < doc)
                {
                    // continue on right side
                    start = middle;
                }
                else
                {
                    end = middle;
                }
            }
            
            // build arr_t
            rle_tuple<int, int> tuple = dt2_docs[middle];
            int* arr_t = new int[tuple.length];
            
            for (int i = 0; i < tuple.length; ++i)
            {
                arr_t[i] = dt2_terms[i + tuple.row_id];
            }
            
            debug("Found " << tuple.length << " tuples for document " << doc);
            
            // run in parallel
            pthread_t** threads = new pthread_t*[NUM_THREADS];
            thread_args_omc_p1** args = new thread_args_omc_p1*[NUM_THREADS];
            
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                threads[t] = new pthread_t();
                args[t] = new thread_args_omc_p1();
                
                int num_terms = tuple.length / NUM_THREADS;
                args[t]->arr_t = new int[num_terms];
                args[t]->len_arr_t = num_terms;
                for (int i = 0; i < num_terms; ++i)
                {
                    args[t]->arr_t[i] = arr_t[i + t * num_terms];
                }
                
                int result = pthread_create(threads[t], NULL, q1_omc_final_pthread, (void*) args[t]);
                
                if (result)
                {
                    error("Creating thread failed with error code " << result << ".");
                }
            }
            
            debug("All threads finished.");
            unordered_map<int, int> result;
            
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                pthread_join(*threads[t], NULL);
                int ctr = 0;
                
                for (int term = 0; term < args[t]->len_arr_t; ++term)
                {
                    //debug("Aggregating " << args[t]->len_arr_d[term] << " documents...");
                    for (int i = 0; i < args[t]->len_arr_d[term]; ++i)
                    {
                        result[args[t]->arr_d[term][i]]++;
                        ctr++;
                    }
                    
                    delete[] args[t]->arr_d[term];
                }
                
                debug("Thread aggregated " << ctr << " documents.");
                
                delete threads[t];
                
                if (args[t]->len_arr_t > 0)
                {
                    delete[] args[t]->len_arr_d;
                    delete[] args[t]->arr_d;
                }
                
                delete[] args[t]->arr_t;
            }
            
            output::stop_timer("run/current_rep");
            output::show_stats();
            
        }
        
        output::stop_timer("run/q1_omc_bench");
        output::show_stats();
    }
    

    struct thread_args_omc_p2
    {
        int* arr_t;
        int len_arr_t;
        int** arr_d;
        int** arr_f;
        int* len_arr_d;
    };
    
    void* q2_omc_final_pthread(void* args)
    {
        thread_args_omc_p2* t_args = (thread_args_omc_p2*) args;
        
        if (t_args->len_arr_t > 0)
        {
            t_args->len_arr_d = new int[t_args->len_arr_t];
            t_args->arr_d = new int*[t_args->len_arr_t];
            t_args->arr_f = new int*[t_args->len_arr_t];
        }
        
        for (int t = 0; t < t_args->len_arr_t; ++t)
        {
            // find RLE-encoded tuple (DT_1)
            int start = 0;
            int end = input::T_PM;
            int middle = (start+end) / 2;
            while (dt1_terms[middle].id != t_args->arr_t[t])
            {
                middle = (start + end)/2;
                if (dt1_terms[middle].id < t_args->arr_t[t])
                {
                    start = middle;
                }
                else
                {
                    end = middle;
                }
            }
            
            // build arr_d
            rle_tuple<int, int> tuple = dt1_terms[middle];
            debug("Found " << tuple.length << " documents for term " << t_args->arr_t[t]);
            
            t_args->arr_d[t] = new int[tuple.length];
            t_args->arr_f[t] = new int[tuple.length];
            t_args->len_arr_d[t] = tuple.length;
            
            for (int i = 0; i < tuple.length; ++i)
            {
                t_args->arr_d[t][i] = dt1_docs[i + tuple.row_id];
                t_args->arr_f[t][i] = dt2_freqs[i + tuple.row_id];  // should be dt1 but we don't care here for performance benchmarks

            }
        }
        
        return NULL;
    }
    
    void q2_omc_final_bench()
    {
        int* input_docs = exact_docs_ooo[5];
        
        show_info("Running Q2 with 10 repetitions and " << NUM_THREADS << " threads...");
        
        output::start_timer("run/q2_omc_bench");
        
        for (int r = 0; r < 10; ++r)
        {
            output::start_timer("run/current_rep");
            
            int doc = input_docs[r];
            
            // binary search to find RLE-encoded tuple (DT_2)
            int start = 0;
            int end = input::D_PM;
            int middle = (start+end) / 2;
            while (dt2_docs[middle].id != doc)
            {
                middle = (start + end)/2;
                if (dt2_docs[middle].id < doc)
                {
                    // continue on right side
                    start = middle;
                }
                else
                {
                    end = middle;
                }
            }
            
            // build arr_t
            rle_tuple<int, int> tuple = dt2_docs[middle];
            int* arr_t = new int[tuple.length];
            
            for (int i = 0; i < tuple.length; ++i)
            {
                arr_t[i] = dt2_terms[i + tuple.row_id];
            }
            
            debug("Found " << tuple.length << " tuples for document " << doc);
            
            // run in parallel
            pthread_t** threads = new pthread_t*[NUM_THREADS];
            thread_args_omc_p2** args = new thread_args_omc_p2*[NUM_THREADS];
            
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                threads[t] = new pthread_t();
                args[t] = new thread_args_omc_p2();
                
                int num_terms = tuple.length / NUM_THREADS;
                args[t]->arr_t = new int[num_terms];
                args[t]->len_arr_t = num_terms;
                for (int i = 0; i < num_terms; ++i)
                {
                    args[t]->arr_t[i] = arr_t[i + t * num_terms];
                }
                
                int result = pthread_create(threads[t], NULL, q2_omc_final_pthread, (void*) args[t]);
                
                if (result)
                {
                    error("Creating thread failed with error code " << result << ".");
                }
            }
            
            debug("All threads finished.");
            unordered_map<int, int>* result = new unordered_map<int, int>();
            
            for (int t = 0; t < NUM_THREADS; ++t)
            {
                pthread_join(*threads[t], NULL);
                int ctr = 0;
                
                for (int term = 0; term < args[t]->len_arr_t; ++term)
                {
                    //debug("Aggregating " << args[t]->len_arr_d[term] << " documents...");
                    for (int i = 0; i < args[t]->len_arr_d[term]; ++i)
                    {
                        (*result)[args[t]->arr_d[term][i]] += args[t]->arr_f[term][i];
                        ctr++;
                    }
                    
                    delete[] args[t]->arr_d[term];
                    delete[] args[t]->arr_f[term];
                }
                
                debug("Thread aggregated " << ctr << " documents.");
                
                delete threads[t];
                
                if (args[t]->len_arr_t > 0)
                {
                    delete[] args[t]->len_arr_d;
                    delete[] args[t]->arr_d;
                    delete[] args[t]->arr_f;
                }
                
                delete[] args[t]->arr_t;
            }
            
            output::stop_timer("run/current_rep");
            output::show_stats();
            
        }
        
        output::stop_timer("run/q2_omc_bench");
        output::show_stats();
    }

    
    void generate_tuples()
    {
        exact_docs_ooo = input::docs_bench_items(); // check at index 5
        
        show_info("Generating random data for DT1...");
        
        // DT1
        /*
        dt1_terms = new rle_tuple<int, int>[input::T_PM];
        debug("ALLOC terms check.");
        dt1_docs = new int[input::NUM_TUPLES];
        debug("ALLOC docs check.");
        
        show_info("Generating docs...");
        // generate random dt1_docs
        int row_counter = 0;
        for (int d = 0; d < input::D_PM; ++d)
        {
            for (int i = 0; i < pubmed::get_group_by_doc(d); ++i)
            {
                dt1_docs[row_counter++] = d;
            }
        }
        
        show_info("Shuffle...");
        shuffle(dt1_docs, dt1_docs + input::NUM_TUPLES, default_random_engine(42));
        
        row_counter = 0;
        for (int t = 0; t < input::T_PM; ++t)
        {
            dt1_terms[t].id = t;
            dt1_terms[t].length = pubmed::get_group_by_term(t);
            dt1_terms[t].row_id = row_counter;
            
            row_counter += pubmed::get_group_by_term(t);
        }
        
        exact_terms = input::terms_bench_items();
        
        show_info("Generating random data for DT2...");
        */
        
        // DT2
        show_info("Query modified for Q3_2.");
        
        dt2_docs = new rle_tuple<int, int>[input::D_PM];
        debug("ALLOC docs check.");
        dt2_terms = new int[input::NUM_TUPLES_DA];
        debug("ALLOC terms check.");
        
        show_info("Generating terms...");
        int row_counter = 0;
        for (int t = 0; t < input::A_PM; ++t)
        {
            for (int i = 0; i < pubmed::get_DA_group_by_author(t); ++i)
            {
                dt2_terms[row_counter++] = t;
            }
        }
        
        show_info("Shuffle...");
        shuffle(dt2_terms, dt2_terms + input::NUM_TUPLES_DA, default_random_engine(42));
        
        row_counter = 0;
        for (int d = 0; d < input::D_PM; ++d)
        {
            dt2_docs[d].id = d;
            dt2_docs[d].length = pubmed::get_DA_group_by_doc(d);
            dt2_docs[d].row_id = row_counter;
            
            for (int t = 0; t < pubmed::get_group_by_doc(d); ++t)
            {
                row_counter++;
                //dt2_freqs[row_counter++] = rand() % 256;
            }
        }
        
        exact_docs = input::docs_bench_items();
        
        show_info("DONE.");
    }
    
    template<typename dtype>
    struct thread_args
    {
        int p;
        int start;
        int end;
        union
        {
            vector<dtype>* intersection;
            map_aggregation* aggr;
        } result;
    };
    
    void* pthread_phase1(void* args)
    {
        int p = ((thread_args<int>*)args)->p;
        int start = ((thread_args<int>*)args)->start;
        int end = ((thread_args<int>*)args)->end;
        vector<int>* intersection = ((thread_args<int>*)args)->result.intersection;
        
        //debug("Starting binary search...");
        int num_terms = end - start;
        vector<int>* temp_docs = new vector<int>[num_terms]();
        
        // build columns
        for (int t = 0; t < num_terms; ++t)
        {
            int term_id = exact_terms[p][t];
            int l = 0;
            int r = input::T_PM;
            
            while (r - l > 1)
            {
                int mid = (l+r) / 2;
                //debug("[BS] mid = " << mid);
                
                if (dt1_terms[mid].id == term_id)
                {
                    l = mid;
                    break;
                }
                else if (dt1_terms[mid].id < term_id)
                {
                    l = mid + 1;
                }
                else if (dt1_terms[mid].id > term_id)
                {
                    r = mid;
                }
            }
            
            if (dt1_terms[l].id != term_id)
            {
                error("binary search failed in phase 1!");
            }
            else
            {
                //debug("[BS] found!");
            }
            
            for (int d = dt1_terms[l].row_id; d < dt1_terms[l].row_id + dt1_terms[l].length; ++d)
            {
                //debug("Add to temp docs column: " << dt1_docs[d]);
                temp_docs[t].push_back(dt1_docs[d]);
            }
            
            //debug("Added docs!");
        }
        
        //debug("Starting intersect...");
        // intersect columns
        for (int i = 0; i < temp_docs[0].size(); ++i)
        {
            bool not_found = false;
            int doc_id = temp_docs[0].at(i);
            
            for (int l = 1; l < num_terms; ++l)
            {
                if (find(temp_docs[l].begin(), temp_docs[l].end(), doc_id) == temp_docs[l].end())
                {
                    not_found = true;
                    break;
                }
            }
            
            if (!not_found)
                intersection->push_back(doc_id);
        }
        
        delete[] temp_docs;
        
        return NULL;
    }
    
    void bench_omc_phase1()
    {
        int num_terms_a[6] = {5, 10, 100, 1000, 10000, 25000};
        
        for (int p = 0; p < 6; ++p)
        {
            int num_terms = num_terms_a[p];
            
            show_info("Running for " << num_terms << " terms using 20 repititions.");
            output::start_timer("run/phase1_omc_final");
            
            for (int r = 0; r < 20; ++r)
            {
                vector<int>* temp_docs = new vector<int>[NUM_THREADS]();
                
                pthread_t** threads = new pthread_t*[NUM_THREADS];
                thread_args<int>** args = new thread_args<int>*[NUM_THREADS];
                
                for (int thread = 0; thread < NUM_THREADS; ++thread)
                {
                    //debug("Spawing thread " << thread << ".");
                    
                    args[thread] = new thread_args<int>;
                    threads[thread] = new pthread_t;
                    args[thread]->p = p;
                    args[thread]->start = thread * num_terms / NUM_THREADS;
                    args[thread]->end = (thread+1) * num_terms / NUM_THREADS;
                    args[thread]->result.intersection = &temp_docs[thread];
                    
                    int result = pthread_create(threads[thread], NULL, pthread_phase1, (void*) args[thread]);
                    
                    if (result)
                    {
                        error("Creating thread failed with error code " << result << ".");
                    }
                }

                for (int thread = 0; thread < NUM_THREADS; ++thread)
                {
                    pthread_join(*threads[thread], NULL);
                    
                    //debug("Joined thread " << thread << ".");
                }
                
                vector<int> intersection;
                
                for (int i = 0; i < temp_docs[0].size(); ++i)
                {
                    bool not_found = false;
                    int doc_id = temp_docs[0].at(i);
                    
                    for (int l = 1; l < NUM_THREADS; ++l)
                    {
                        if (find(temp_docs[l].begin(), temp_docs[l].end(), doc_id) == temp_docs[l].end())
                        {
                            not_found = true;
                            break;
                        }
                    }
                    
                    if (!not_found)
                        intersection.push_back(doc_id);
                }
                
                //debug("Delete...");
                delete[] temp_docs;
                //debug("There are " << intersection.size() << " elements in the intersection.");
            }
            
            output::stop_timer("run/phase1_omc_final");
            output::show_stats();
        }
    }
    
    void* pthread_phase2(void* args)
    {
        int p = ((thread_args<int>*)args)->p;
        int start = ((thread_args<int>*)args)->start;
        int end = ((thread_args<int>*)args)->end;
        map_aggregation* aggr = ((thread_args<int>*)args)->result.aggr;
        
        int num_docs = end - start;
        
        vector<int>* temp_terms = new vector<int>[num_docs]();
        vector<unsigned char>* temp_freqs = new vector<unsigned char>[num_docs]();
        
        // build columns
        for (int t = 0; t < num_docs; ++t)
        {
            int doc_id = exact_docs[p][t];
            int l = 0;
            int r = input::D_PM;
            
            while (r - l > 1)
            {
                int mid = (l+r) / 2;
                
                if (dt2_docs[mid].id == doc_id)
                {
                    l = mid;
                    break;
                }
                else if (dt2_docs[mid].id < doc_id)
                {
                    l = mid + 1;
                }
                else if (dt2_docs[mid].id > doc_id)
                {
                    r = mid;
                }
            }
            
            if (dt2_docs[l].id != doc_id)
            {
                error("binary search failed in phase 2!");
            }
            
            for (int d = dt2_docs[l].row_id; d < dt2_docs[l].row_id + dt2_docs[l].length; ++d)
            {
                temp_terms[t].push_back(dt2_terms[d]);
                //temp_freqs[t].push_back(dt2_freqs[d]);
            }
        }
        
        // aggregate
        for (int d = 0; d < num_docs; ++d)
        {
            for (int t = 0; t < temp_terms[d].size(); ++t)
            {
                aggr->add(temp_terms[d].at(t), 1);
            }
        }
        
        delete[] temp_freqs;
        delete[] temp_terms;
        
        return NULL;
    }
    
    void bench_omc_phase2()
    {
        int num_docs_a[6] = {10, 100, 1000, 10000, 100000, 1000000};
        
        for (int p = 0; p < 6; ++p)
        {
            int num_docs = num_docs_a[p];
            
            show_info("Running for " << num_docs << " documents using 20 repititions.");
            output::start_timer("run/phase2_omc_final MODIFIED FOR Q3_2");
            
            for (int r = 0; r < 20; ++r)
            {
                map_aggregation** temp_aggrs = new map_aggregation*[NUM_THREADS];
                
                vector<int>* temp_docs = new vector<int>[NUM_THREADS]();
                
                pthread_t** threads = new pthread_t*[NUM_THREADS];
                thread_args<int>** args = new thread_args<int>*[NUM_THREADS];
                
                for (int thread = 0; thread < NUM_THREADS; ++thread)
                {
                    //debug("Spawing thread " << thread << ".");
                    
                    args[thread] = new thread_args<int>;
                    threads[thread] = new pthread_t;
                    args[thread]->p = p;
                    args[thread]->start = thread * num_docs / NUM_THREADS;
                    args[thread]->end = (thread+1) * num_docs / NUM_THREADS;
                    temp_aggrs[thread] = new map_aggregation();
                    args[thread]->result.aggr = temp_aggrs[thread];
                    
                    int result = pthread_create(threads[thread], NULL, pthread_phase2, (void*) args[thread]);
                    
                    if (result)
                    {
                        error("Creating thread failed with error code " << result << ".");
                    }
                }
                
                // global aggregate
                map_aggregation global_aggr;
                
                for (int thread = 0; thread < NUM_THREADS; ++thread)
                {
                    pthread_join(*threads[thread], NULL);
                    
                    for (auto it = args[thread]->result.aggr->data.begin(); it != args[thread]->result.aggr->data.end(); ++it)
                    {
                        global_aggr.add(it->first, it->second);
                    }
                    //debug("Joined thread " << thread << ".");
                    
                    delete args[thread]->result.aggr;
                }

                delete[] temp_aggrs;
                //debug("There are " << intersection.size() << " elements in the intersection.");
            }
            
            output::stop_timer("run/phase2_omc_final MODIFIED FOR Q3_2");
            output::show_stats();
        }
    }
}