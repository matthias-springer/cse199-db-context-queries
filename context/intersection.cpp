#include "intersection.h"
#include "output.h"
#include <unordered_set>
#include <ibis.h>

#define SET_SIZE 100000

using namespace std;

namespace benchmark
{
    unordered_set<int> ib_list1;
    unordered_set<int> ib_list2;

    ibis::bitvector ib_vector1;
    ibis::bitvector ib_vector2;
    
    void ib_generate_data()
    {
        for (int i = 0; i < SET_SIZE; ++i)
        {
            ib_list1.insert(rand() % 10000000);
            ib_list2.insert(rand() % 10000000);
        }
        
        for (auto it = ib_list1.begin(); it != ib_list1.end(); ++it)
        {
            ib_vector1.setBit(*it, 1);
        }
        
        for (auto it = ib_list2.begin(); it != ib_list2.end(); ++it)
        {
            ib_vector2.setBit(*it, 1);
        }
    }
    
    void ib_uncompressed()
    {
        show_info("Running intersection with 10000 repetitions for uncompressed...");
     
        output::start_timer("ib/uncompressed");
        
        for (int r = 0; r < 10000; ++r)
        {
            unordered_set<int> result;
            
            for (auto it = ib_list1.begin(); it != ib_list1.end(); ++it)
            {
                if (ib_list2.find(*it) != ib_list2.end())
                {
                    result.insert(*it);
                }
            }
        }
        
        output::stop_timer("ib/uncompressed");
        output::show_stats();
    }
    
    void ib_bitvector()
    {
        show_info("Running intersection with 10000 repetitions for bit vector...");
        
        output::start_timer("ib/bitvector");
        
        for (int r = 0; r < 10000; ++r)
        {
            ibis::bitvector base(ib_vector1);
            base &= ib_vector2;
        }
        
        output::stop_timer("ib/bitvector");
        output::show_stats();
    }
}