#ifndef __context__storage__
#define __context__storage__

#include <iostream>
#include <string.h>
#include "context.h"

#define STORAGE_TYPE_BITVECTOR      0
#define STORAGE_TYPE_LIST           1
#define STORAGE_TYPE_MAP_COLUMN     2
#define STORAGE_TYPE_COLUMN_LIST    3

using namespace std;

class storage
{
private:
    int value;
    
public:
    int type;
    
    virtual void intersect(storage& another_storage) = 0;
    virtual void load_from_file(string name, long offset, long length) = 0;
    virtual void generate_randomly(DOMAIN_TYPE count, DOMAIN_TYPE max_value) = 0;
    virtual long save_to_file(string name, pair<long, long> &position) = 0;
    virtual long count() = 0;
    virtual vector<DOMAIN_TYPE> *elements() = 0;
    virtual void add(DOMAIN_TYPE item) = 0;
    virtual storage* copy();
    void print_elements();
    virtual ~storage();
    
    static storage* new_from_file(string name, long offset, long length, int type);
    static storage* load(string name, DOMAIN_TYPE id);
    static storage* load(string name, DOMAIN_TYPE id, int type);
    static storage* new_instance();
};

#endif
