#include "array_aggregration.h"
#include "input.h"

#define min(a,b) (a < b ? a : b)

void array_aggregation::add(DOMAIN_TYPE key, DOMAIN_TYPE increment)
{
    if (key >= size)
    {
        error("Aggregation array not big enough for " << key << ".");
    }
    
    data[key] += increment;
}

array_aggregation::array_aggregation(DOMAIN_TYPE size)
{
    debug("Creating new array aggregation of size " << size << ".");
    data = new DOMAIN_TYPE[size]();
    this->size = size;
}

vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *array_aggregation::top_k(int k)
{
    vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> pairs;
    for (DOMAIN_TYPE i = 0; i < size; ++i)
    {
        pair<DOMAIN_TYPE, DOMAIN_TYPE> p;
        p.first = i;
        p.second = data[i];
        pairs.push_back(p);
    }
    
    sort(pairs.begin(), pairs.end(), [=](pair<DOMAIN_TYPE, DOMAIN_TYPE> &a, pair<DOMAIN_TYPE, DOMAIN_TYPE> &b)
         {
             return a.second > b.second;
         });
    
    vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *result = new vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>>();
    
    for (int i = 0; i < min(k, pairs.size()); ++i)
    {
        result->push_back(pairs[i]);
    }
    
    return result;
}
