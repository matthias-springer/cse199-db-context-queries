#ifndef __context__list_storage__
#define __context__list_storage__

#include <iostream>
#include "storage.h"
#include "context.h"

using namespace std;

class list_storage : public storage
{
public:
    vector<DOMAIN_TYPE> *data;
    
    list_storage();
    virtual ~list_storage();
    virtual void intersect(storage& another_storage);
    virtual void load_from_file(string name, long offset, long length);
    virtual long save_to_file(string name, pair<long, long> &position);
    virtual long count();
    virtual vector<DOMAIN_TYPE> *elements();
    virtual void add(DOMAIN_TYPE item);
    
    ibis::bitvector *to_bit_vector();
};

#endif
