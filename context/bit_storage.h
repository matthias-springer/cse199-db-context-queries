#ifndef __context__bit_storage__
#define __context__bit_storage__

#include <iostream>
#include "context.h"
#include "storage.h"
#include <bitvector.h>
#include <ibis.h>

typedef uint32_t word_t;

class bit_storage : public storage
{
public:
    ibis::bitvector *data;
    
    bit_storage();
    virtual ~bit_storage();
    virtual void intersect(storage& another_storage);
    virtual void load_from_file(string name, long offset, long length);
    virtual long save_to_file(string name, pair<long, long> &position);
    virtual long count();
    virtual vector<DOMAIN_TYPE> *elements();
    virtual void add(DOMAIN_TYPE item) ;
};

#endif
