#include "final_omc.h"
#include "pubmed.h"
#include "output.h"
#include "input.h"
#include <unordered_map>

namespace benchmark
{
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
    
    // DT_2
    rle_tuple<int, short>* dt2_docs;
    short* dt2_terms;
    unsigned char* dt2_freqs;
    
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
        
        show_info("DONE.");
    }
    
    void bench_omc_phase1()
    {
        int num_terms_a[6] = {5, 10, 100, 1000, 10000, 25000};
        
        for (int p = 0; p < 6; ++p)
        {
            int num_terms = num_terms_a[p];
            
            show_info("Running for " << num_terms << " terms using 200 repititions.");
            output::start_timer("run/phase1_omc_final");
            
            for (int r = 0; r < 200; ++r)
            {
                
                int* input_terms = new int[num_terms];
                for (int i = 0; i < num_terms; ++i)
                    input_terms[i] = rand() % input::T_PM;
                
                vector<int>* temp_docs = new vector<int>[num_terms]();
                
                //debug("Starting binary search...");
                
                // build columns
                for (int t = 0; t < num_terms; ++t)
                {
                    int term_id = input_terms[t];
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
                
                debug("Starting intersect...");
                // intersect columns
                vector<int> intersection;
                for (int i = 0; i < temp_docs[0].size(); ++i)
                {
                    int doc_id = temp_docs[0].at(i);
                    
                    for (int l = 1; l < num_terms; ++l)
                    {
                        if (find(temp_docs[l].begin(), temp_docs[l].end(), doc_id) == temp_docs[l].end())
                        {
                            continue;
                        }
                    }
                    
                    intersection.push_back(doc_id);
                }
                
                debug("Delete...");
                delete[] temp_docs;
                //debug("There are " << intersection.size() << " elements in the intersection.");
            }
            
            output::stop_timer("run/phase1_omc_final");
            output::show_stats();
        }
    }
    
    void bench_omc_phase2()
    {
        int num_docs_a[6] = {10, 100, 1000, 10000, 100000, 1000000};
        
        for (int p = 0; p < 6; ++p)
        {
            int num_docs = num_docs_a[p];
            
            show_info("Running for " << num_docs << " documents using 50 repititions.");
            output::start_timer("run/phase2_omc_final");
            
            for (int r = 0; r < 50; ++r)
            {
                
                int* input_docs;
                input_docs = new int[num_docs];
                for (int i = 0; i < num_docs; ++i)
                    input_docs[i] = rand() % input::D_PM;
                
                vector<short>* temp_terms = new vector<short>[num_docs]();
                vector<unsigned char>* temp_freqs = new vector<unsigned char>[num_docs]();
                
                // build columns
                for (int t = 0; t < num_docs; ++t)
                {
                    int doc_id = input_docs[t];
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
                unordered_map<short, int> aggr;
                
                for (int d = 0; d < num_docs; ++d)
                {
                    for (int t = 0; t < temp_terms[d].size(); ++t)
                    {
                        aggr[temp_terms[d].at(t)] += temp_freqs[d].at(t);
                    }
                }
                
                delete[] temp_freqs;
                delete[] temp_terms;
                //debug("There are " << intersection.size() << " elements in the intersection.");
            }
            
            output::stop_timer("run/phase2_omc_final");
            output::show_stats();
        }
    }
}