#ifndef __context__map_aggregation__
#define __context__map_aggregation__

#include <iostream>
#include <unordered_map>
#include "aggregation.h"

// unordered_map implementation: hash table
// map implementation: LLRB (left leaning red black tree)

using namespace std;

class map_aggregation : public aggregation
{   
public:
    unordered_map<DOMAIN_TYPE, DOMAIN_TYPE> data;
    
    map_aggregation();
    virtual void add(DOMAIN_TYPE key, DOMAIN_TYPE increment);
    virtual vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *top_k(int k);
};

#endif
