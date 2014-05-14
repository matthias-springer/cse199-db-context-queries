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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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
    
    input::input_file = stdin;
    input::storage_type = STORAGE_TYPE_BITVECTOR;
    input::debug = 0;
    
    char *help = "usage: %s [-h] [-r] [-p] [-f file] [-s] [-i id] [-n name] [-q id] [-t type] [-d] \n\n"
        "-h\tShow list of parameters.\n"
        "-r\tRead data for name from stdin.\n"
        "-p\tPrint storage to stdout.\n"
        "-f file\tRead input from file instead of stdin.\n"
        "-s\tShow runtime statistics at the end of the execution.\n"
        "-i id\tSpecifies an id. Required for some actions.\n"
        "-n name\tSpecifies a name. Required for some actions.\n"
        "-q id\tRuns query id (see below).\n"
        "-t type\tSpecifies the storage type (see below).\n"
        "-d\tEnables debug mode.\n\n"
        "List of queries/benchmarks:\n"
        "[1]\tCount number of documents in context.\n"
        "[2]\tRetrieve top-k terms in documents.\n"
        "[3]\tRetrieve top-k terms in context.\n"
        "[4]\tSelf test\n"
        "[5]\tRetrieve top-k terms in documents with term frequency.\n"
        "[6]\tRetrieve top-k terms in context with term frequency.\n"
        "[7]\tBenchmark for query 1.\n\n"
        "List of storage types:\n"
        "[0]\tBit vector\n"
        "[1]\tVector (array)\n";
    
    while ((c = getopt(argc, argv, "hrf:sn:i:pq:t:d")) != -1)
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
        }
    }
    
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
            }
            break;
    }
    
    // shutdown code
    offset::close_offset_files();
    
    output::show_stats();
    
    return 0;
}

