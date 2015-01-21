#include "input.h"
#include "bit_storage.h"
#include "offset.h"
#include "pubmed.h"

namespace input
{
    FILE *input_file;
    int storage_type;
    int debug;
    bool stats_visible;
    bool no_generate_benchmark_data;
    bool omit_io;
        
    DOMAIN_TYPE b_DOCUMENTS_PER_TERM = 50000;
    DOMAIN_TYPE b_TERMS_PER_DOCUMENT = 100;
    DOMAIN_TYPE b_MAX_FREQUENCY = 40;
    DOMAIN_TYPE b_K = 5;
    
    char* STATS_FILE_T;
    char* STATS_FILE_D;
    
    char* STATS_FILE_DA_A;
    char* STATS_FILE_DA_D;
    
    // number of documents and terms, read from first line of CSV file
    long D_PM;
    long T_PM;
    long A_PM;
    long NUM_TUPLES = 0;
    long NUM_TUPLES_DA = 0;

    void print_stats()
    {
        show_info("# docs                | " << D_PM);
        show_info("# terms               | " << T_PM);
        show_info("# authors             | " << A_PM);
        show_info("# tuples DT           | " << NUM_TUPLES);
        show_info("# tuples DA           | " << NUM_TUPLES_DA);
    }

    void store(string name)
    {
        show_info("Input number id and number of values.");
        store(name, read_value());
    }
    
    void store(string name, DOMAIN_TYPE id)
    {
        show_info("Input data for " << name << ":" << id << ". Waiting for count.");
        
        DOMAIN_TYPE count = read_value();
        show_info("Waiting for " << count << " values.");
        
        storage *s = storage::new_instance();
        
        for (int i = 0; i < count; ++i)
        {
            s->add(read_value());
        }
        
        pair<long, long> position;
        s->save_to_file(name, position);
        debug("Saved storage to file.");
        
        offset::set_offsets_for(name, id, position.first, position.second);
        debug("Saved offsets to offset file.");
    }
    
    DOMAIN_TYPE read_value()
    {
        DOMAIN_TYPE value;
        
        if (sizeof(DOMAIN_TYPE) == 1)
        {
            fscanf(input_file, "%c", &value);
        }
        else if (sizeof(DOMAIN_TYPE) == 4)
        {
            fscanf(input_file, "%i", &value);
        }
        else if (sizeof(DOMAIN_TYPE) == 8)
        {
            fscanf(input_file, "%ld", &value);
        }
        
        return value;
    }
    
    short** terms_bench_items()
    {
        srand(123);
        
        int num_terms_a[6] = {5, 10, 100, 1000, 10000, 25000};
        short** terms = new short*[6];
        
        for (int i = 0; i < 6; ++i)
        {
            terms[i] = new short[num_terms_a[i]];
            
            for (int j = 0; j < num_terms_a[i]; ++j)
            {
                terms[i][j] = rand() % T_PM;
            }
        }
        
        return terms;
    }
    
    void dealloc_terms_bench_items(short** ptr)
    {
        for (int i = 0; i < 6; ++i)
        {
            delete[] ptr[i];
        }
        
        delete ptr;
    }
    
    int** docs_bench_items()
    {
        srand(456);
        
        int num_docs_a[6] = {10, 100, 1000, 10000, 100000, 1000000};
        int** docs = new int*[6];
        
        for (int i = 0; i < 6; ++i)
        {
            docs[i] = new int[num_docs_a[i]];
            
            for (int j = 0; j < num_docs_a[i]; ++j)
            {
                docs[i][j] = rand() % D_PM;
            }
        }
        
        return docs;
    }
    
    void dealloc_docs_bench_items(int** ptr)
    {
        for (int i = 0; i < 6; ++i)
        {
            delete[] ptr[i];
        }
        
        delete ptr;
    }
}
