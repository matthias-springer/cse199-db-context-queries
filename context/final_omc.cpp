#include "final_omc.h"
#include "pubmed.h"
#include "output.h"
#include "input.h"
#include <unordered_map>
#include <pthread.h>

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
    rle_tuple<short, int>* dt1_terms;
    short** exact_terms;
    
    // DT_2
    rle_tuple<int, short>* dt2_docs;
    short* dt2_terms;
    unsigned char* dt2_freqs;
    int** exact_docs;
    
    void generate_tuples()
    {
        show_info("Generating random data for DT1...");
        
        // DT1
        dt1_terms = new rle_tuple<short, int>[input::T_PM];
        debug("ALLOC terms check.");
        dt1_docs = new int[input::NUM_TUPLES];
        debug("ALLOC docs check.");
        
        int row_counter = 0;
        for (int t = 0; t < input::T_PM; ++t)
        {
            dt1_terms[t].id = t;
            dt1_terms[t].length = pubmed::get_group_by_term(t);
            dt1_terms[t].row_id = row_counter;
            
            for (int d = 0; d < pubmed::get_group_by_term(t); ++d)
            {
                dt1_docs[row_counter++] = rand() % input::D_PM;
            }
        }
        
        exact_terms = input::terms_bench_items();
        
        show_info("Generating random data for DT2...");
        
        // DT2
        dt2_docs = new rle_tuple<int, short>[input::D_PM];
        debug("ALLOC docs check.");
        dt2_freqs = new unsigned char[input::NUM_TUPLES];
        debug("ALLOC freqs check.");
        dt2_terms = new short[input::NUM_TUPLES];
        debug("ALLOC terms check.");
        
        row_counter = 0;
        for (int d = 0; d < input::D_PM; ++d)
        {
            dt2_docs[d].id = d;
            dt2_docs[d].length = pubmed::get_group_by_doc(d);
            dt2_docs[d].row_id = row_counter;
            
            for (int t = 0; t < pubmed::get_group_by_doc(d); ++t)
            {
                dt2_terms[row_counter] = rand() % input::T_PM;
                dt2_freqs[row_counter++] = rand() % 256;
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
            unordered_map<dtype, int>* aggr;
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
            short term_id = exact_terms[p][t];
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
        int p = ((thread_args<short>*)args)->p;
        int start = ((thread_args<short>*)args)->start;
        int end = ((thread_args<short>*)args)->end;
        unordered_map<short, int>* aggr = ((thread_args<short>*)args)->result.aggr;
        
        int num_docs = end - start;
        
        vector<short>* temp_terms = new vector<short>[num_docs]();
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
                temp_freqs[t].push_back(dt2_freqs[d]);
            }
        }
        
        // aggregate
        for (int d = 0; d < num_docs; ++d)
        {
            for (int t = 0; t < temp_terms[d].size(); ++t)
            {
                (*aggr)[temp_terms[d].at(t)] += temp_freqs[d].at(t);
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
            output::start_timer("run/phase2_omc_final");
            
            for (int r = 0; r < 20; ++r)
            {
                unordered_map<short, int>* temp_aggrs = new unordered_map<short, int>[NUM_THREADS];
                
                vector<int>* temp_docs = new vector<int>[NUM_THREADS]();
                
                pthread_t** threads = new pthread_t*[NUM_THREADS];
                thread_args<short>** args = new thread_args<short>*[NUM_THREADS];
                
                for (int thread = 0; thread < NUM_THREADS; ++thread)
                {
                    //debug("Spawing thread " << thread << ".");
                    
                    args[thread] = new thread_args<short>;
                    threads[thread] = new pthread_t;
                    args[thread]->p = p;
                    args[thread]->start = thread * num_docs / NUM_THREADS;
                    args[thread]->end = (thread+1) * num_docs / NUM_THREADS;
                    args[thread]->result.aggr = &temp_aggrs[thread];
                    
                    int result = pthread_create(threads[thread], NULL, pthread_phase2, (void*) args[thread]);
                    
                    if (result)
                    {
                        error("Creating thread failed with error code " << result << ".");
                    }
                }
                
                // global aggregate
                unordered_map<short, int> global_aggr;
                
                for (int thread = 0; thread < NUM_THREADS; ++thread)
                {
                    pthread_join(*threads[thread], NULL);
                    
                    for (auto it = args[thread]->result.aggr->begin(); it != args[thread]->result.aggr->end(); ++it)
                    {
                        global_aggr[it->first] += it->second;
                    }
                    //debug("Joined thread " << thread << ".");
                }

                delete[] temp_aggrs;
                //debug("There are " << intersection.size() << " elements in the intersection.");
            }
            
            output::stop_timer("run/phase2_omc_final");
            output::show_stats();
        }
    }
}