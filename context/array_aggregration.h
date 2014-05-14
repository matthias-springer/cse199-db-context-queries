#ifndef __context__array_aggregration__
#define __context__array_aggregration__

#include <iostream>
#include "aggregation.h"

class array_aggregation : public aggregation
{
private:
    DOMAIN_TYPE *data;
    
public:
    array_aggregation(long size);
    virtual void add(DOMAIN_TYPE key, DOMAIN_TYPE increment);
    virtual vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *top_k(int k);
};

#endif
