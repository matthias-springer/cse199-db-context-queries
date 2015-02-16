#include <iostream>
#include "context.h"
#include "input.h"
#include "list_storage.h"
#include "file.h"
#include "count_context_query.h"
#include "top_k_query.h"
#include "top_k_tf_query.h"
#include "self_test.h"
#include "count_context_benchmark.h"
#include "top_k_query_benchmark.h"
#include "top_k_query_tf_benchmark.h"
#include "top_k_query_tf_dual_list_benchmark.h"
#include "top_k_tf_column_db_query_benchmark.h"
#include "huffman_query_benchmark.h"
#include "pubmed.h"
#include "huffman_benchmark.h"
#include "final_phase1.h"
#include "column_db_bitvector_space_benchmark.h"
#include "final_omc.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "q1_bench.h"
#include "q2_bench.h"
#include "huffman_self_test.h"
#include "compression.h"
#include "intersection.h"
#include "q3_phase2.h"
#include "q5_bench_fastr.h"
#include "q5_bench_omc.h"

int main(int argc, char ** argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    
    bool show_stats = false;
    char c;
    char action = 0;
    char *name = NULL;
    int query = 0;
    DOMAIN_TYPE id = -1;
    
    input::omit_io = false;
    input::input_file = stdin;
    input::storage_type = STORAGE_TYPE_BITVECTOR;
    input::debug = 0;
    input::no_generate_benchmark_data = false;
    storage_path = "/Users/matthias/Documents/Important/uni/cse199/cse199-db-context-queries/fastbit_index/database";
    input::STATS_FILE_T = "pubmed/stats_terms.csv";
    input::STATS_FILE_D = "pubmed/stats_docs.csv";
    input::STATS_FILE_DA_A = "pubmed/DA_authors.csv";
    input::STATS_FILE_DA_D = "pubmed/DA_docs.csv";
    
    char *help = "usage: %s [-hrpsdzo] [-f file] [-i id] [-n name] [-q id] [-t type] [-w path] \n\n"
        "-h\tShow list of parameters.\n"
        "-r\tRead data for name from stdin.\n"
        "-p\tPrint storage to stdout.\n"
        "-f file\tRead input from file instead of stdin.\n"
        "-s\tShow runtime statistics at the end of the execution.\n"
        "-i id\tSpecifies an id. Required for some actions.\n"
        "-n name\tSpecifies a name. Required for some actions.\n"
        "-q id\tRuns query id (see below).\n"
        "-t type\tSpecifies the storage type (see below).\n"
        "-z\tDo not generate benchmark data, reuse existing data.\n"
        "-w path\tUse path as the working directory containing all data.\n"
        "-k file\tOverride STATS_FILE_D (terms per document file)\n"
        "-l file\tOverride STATS_FILE_T (documents per term file)\n"
        "-o\tOmit IO in benchmarks.\n"
        "-d\tEnables debug mode.\n\n"
        "List of queries/benchmarks:\n"
        "[1]\tCount number of documents in context.\n"
        "[2]\tRetrieve top-k terms in documents.\n"
        "[3]\tRetrieve top-k terms in context.\n"
        "[4]\tSelf test\n"
        "[5]\tRetrieve top-k terms in documents with term frequency.\n"
        "[6]\tRetrieve top-k terms in context with term frequency.\n"
        "[7]\tBenchmark for query 1.\n"
        "[8]\tBenchmark for query 2.\n"
        "[9]\tBenchmark for query 5.\n"
        "[10]\tBenchmark for query 5 with dual lists.\n"
        "[11]\tBenchmark for query 6 with column database.\n"
        "[12]\tBenchmark for query 5 with column database.\n"
        "[13]\tBenchmark for query 1 with column database.\n"
        "[14]\tHuffman Benchmark.\n"
        "[15]\tHuffman query benchmark.\n"
        "[16]\tFinal Phase 1 benchmark.\n"
        "[17]\tColumn DB bit vector compression space benchmark.\n"
        "[18]\tFinal OMC for Phase 1.\n"
        "[19]\tFinal OMC for Phase 2.\n"
        "[20]\tQ1 benchmark.\n"
        "[21]\tHuffman self test.\n"
        "[22]\tQ2 benchmark.\n"
        "[23]\tCompression with duplicates.\n"
        "[24]\tIntersection benchmark.\n"
        "[25]\tQ1 OMC benchmark.\n"
        "[26]\tQ2 OMC benchmark.\n"
        "[27]\tQ3 Phase 2 benchmark.\n"
        "[28]\tQ5 FastR benchmark.\n"
        "[29]\tQ5 OMC benchmark.\n"
        "List of storage types:\n"
        "[0]\tBit vector\n"
        "[1]\tVector (array)\n";
    
    while ((c = getopt(argc, argv, "hrf:sn:i:pq:t:dzw:ok:l:")) != -1)
    {
        switch (c)
        {
            case 'h':
                printf(help, (strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0]));
                exit(0);
                break;
            case 'r':
                action = 'r';
                break;
            case 'n':
                name = new char[strlen(optarg) + 1];
                strcpy(name, optarg);
                break;
            case 'i':
                if (sizeof(DOMAIN_TYPE) == 4)
                {
                    id = atoi(optarg);
                }
                else if (sizeof(DOMAIN_TYPE) == 8)
                {
                    id = atol(optarg);
                }
                break;
            case 'f':
                input::input_file = fopen(optarg, "r");
                
                if (input::input_file == NULL)
                {
                    error("Unable to read from input file " << optarg << ".");
                }
                break;
            case 's':
                input::stats_visible = true;
                break;
            case 'p':
                action = 'p';
                break;
            case 'q':
                action = 'q';
                query = atoi(optarg);
                break;
            case 't':
                input::storage_type = atoi(optarg);
                break;
            case 'd':
                input::debug = 1;
                break;
            case 'z':
                input::no_generate_benchmark_data = true;
                break;
            case 'w':
                storage_path = optarg;
                break;
            case 'k':
                input::STATS_FILE_D = optarg;
                break;
            case 'l':
                input::STATS_FILE_T = optarg;
                break;
            case 'o':
                input::omit_io = true;
                break;
        }
    }
    
    pubmed::load_docs_per_term();
    pubmed::load_terms_per_doc();
    pubmed::load_authors_per_doc();
    pubmed::load_docs_per_author();

    show_info("Using these statistics:");
    input::print_stats();
        
    show_info("Using working directory: " << storage_base_path());
    
    switch (action)
    {
        case 0:
            error("Invalid command line arguments. Use -h for help.");
            break;
        case 'r':
            if (name == NULL || id == -1) error("Parameters -n and -i are required.");
            input::store(name, id);
            break;
        case 'p':
            if (name == NULL || id == -1) error("Parameters -n and -i are required.");
            storage::load(name, id)->print_elements();
            break;
        case 'q':
            switch (query)
            {
                case 1:
                    count_context_query::stdin_documents_in_context();
                    break;
                case 2:
                    top_k_query::stdin_top_k_in_documents();
                    break;
                case 3:
                    top_k_query::stdin_top_k_in_context();
                    break;
                case 4:
                    benchmark::run_self_test();
                    break;
                case 5:
                    top_k_tf_query::stdin_top_k_tf_in_documents();
                    break;
                case 6:
                    top_k_tf_query::stdin_top_k_tf_in_context();
                    break;
                case 7:
                    benchmark::run_count_context();
                    break;
                case 8:
                    benchmark::run_top_k();
                    break;
                case 9:
                    benchmark::run_top_k_tf();
                    break;
                case 10:
                    benchmark::run_top_k_tf_dual_list();
                    break;
                case 11:
                    benchmark::run_top_k_tf_column_db_query();
                    break;
                case 12:
                    benchmark::run_top_k_tf_in_docs_column_db_query();
                    break;
                case 13:
                    benchmark::run_docs_in_context_column_db_query();
                    break;
                case 14:
                    calculate_huffman_encoded_size();
                    break;
                case 15:
                    benchmark::huffman_query_generate_lists();
                    benchmark::huffman_query_run_benchmark();
                    break;
                case 16:
                    benchmark::generate_bit_vectors();
                    benchmark::run_phase1_bench_final();
                    break;
                case 17:
                    benchmark::column_db_bitvector_output_compression_ratio();
                    break;
                case 18:
                    benchmark::generate_tuples();
                    benchmark::bench_omc_phase1();
                    break;
                case 19:
                    benchmark::generate_tuples();
                    benchmark::bench_omc_phase2();
                    break;
                case 20:
                    benchmark::generate_data_q1();
                    benchmark::q1_final_bench();
                    break;
                case 21:
                    huffman_self_test();
                    break;
                case 22:
                    benchmark::generate_data_q2();
                    benchmark::q2_final_bench();
                    break;
                case 23:
                    benchmark::cb_generate_tuples();
                    benchmark::cb_uncompressed();
                    benchmark::cb_huffman();
                    benchmark::cb_rle();
                    benchmark::cb_bitvector();
                    break;
                case 24:
                    benchmark::ib_generate_data();
                    benchmark::ib_uncompressed();
                    benchmark::ib_bitvector();
                    break;
                case 25:
                    benchmark::generate_tuples();
                    benchmark::q1_omc_final_bench();
                    break;
                case 26:
                    benchmark::generate_tuples();
                    benchmark::q2_omc_final_bench();
                    break;
                case 27:
                    benchmark_q3::huffman_query_generate_lists();
                    benchmark_q3::huffman_query_run_benchmark();
                    break;
                case 28:
                    benchmark_q5::run_bench_q5_fastr();
                    break;
                case 29:
                    benchmark_q5_omc::run_bench_q5_omc();
                    break;
            }
            break;
    }
    
    // shutdown code
    offset::close_offset_files();
    
    output::show_stats();
    
    return 0;
}

