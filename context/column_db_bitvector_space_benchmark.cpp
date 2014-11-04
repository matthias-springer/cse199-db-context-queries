#include "column_db_bitvector_space_benchmark.h"

#include "pubmed.h"
#include "output.h"
#include "input.h"

namespace benchmark
{
    void column_db_bitvector_output_compression_ratio()
    {
        unsigned long terms_uncompressed = 0;
        unsigned long terms_compressed = 0;
        
        for (int i = 0; i < input::T_PM; ++i)
        {
            ibis::bitvector bv;
            
            for (int j = 0; j < pubmed::get_group_by_term(i); ++j)
            {
                bv.setBit(rand() % input::NUM_TUPLES, 1);
            }
            
            bv.compress();
            
            terms_uncompressed += 2 * pubmed::get_group_by_term(i);
            terms_compressed += bv.bytes();
            
            debug_n("  " << i*100.0/input::T_PM << " % complete.    ");
        }
        
        debug_n("  " << 100 << " % complete.    \n");
        
        show_info("terms uncompressed: " << terms_uncompressed);
        show_info("terms compressed: " << terms_compressed);
        
        
        unsigned long docs_uncompressed = 0;
        unsigned long docs_compressed = 0;
        
        for (int i = 0; i < input::D_PM; ++i)
        {
            ibis::bitvector bv;
            
            for (int j = 0; j < pubmed::get_group_by_doc(i); ++j)
            {
                bv.setBit(rand() % input::NUM_TUPLES, 1);
            }
            
            bv.compress();
            
            docs_uncompressed += 4 * pubmed::get_group_by_doc(i);
            docs_compressed += bv.bytes();
            
            debug_n("  " << i*100.0/input::D_PM << " % complete.    ");
        }
        
        debug_n("  " << 100 << " % complete.    \n");
        
        show_info("terms uncompressed: " << docs_uncompressed);
        show_info("terms compressed: " << docs_compressed);
    }
}
