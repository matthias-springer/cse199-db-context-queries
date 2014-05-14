#ifndef __context__file__
#define __context__file__

#include <iostream>
#include <string.h>
#include <sstream>

using namespace std;

string storage_base_path();
string storage_data_file_name(string name);
string storage_offset_file_name(string name);

#endif
