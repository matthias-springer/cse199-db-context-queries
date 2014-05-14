#ifndef __context__aggregation__
#define __context__aggregation__

#include <iostream>
#include "compile_options.h"
#include "context.h"

class aggregation
{
public:
    virtual void add(DOMAIN_TYPE key, DOMAIN_TYPE increment) = 0;
    virtual vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *top_k(int k) = 0;
};

#endif
