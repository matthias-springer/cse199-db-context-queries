#include "file.h"

using namespace std;

string storage_base_path()
{
    return "/Users/matthias/Documents/Important/uni/cse199/cse199-db-context-queries/fastbit_index/database";
}

string storage_data_file_name(string name)
{
    stringstream stream;
    stream << storage_base_path() << "/" << name << ".data";
    
    return stream.str();
}

string storage_offset_file_name(string name)
{
    stringstream stream;
    stream << storage_base_path() << "/" << name << ".offset";
    
    return stream.str();
}