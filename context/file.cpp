#include "file.h"

using namespace std;

string storage_path;

string storage_base_path()
{
    //return "/Volumes/4 TB External/context";
    return storage_path;
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