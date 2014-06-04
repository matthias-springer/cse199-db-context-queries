#include "map_aggregation.h"

#define min(a,b) (a < b ? a : b)

void map_aggregation::add(DOMAIN_TYPE key, DOMAIN_TYPE increment)
{
    (*data)[key] = (*data)[key] + increment;
}

map_aggregation::map_aggregation()
{
    data = new unordered_map<DOMAIN_TYPE, DOMAIN_TYPE>();
}

vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *map_aggregation::top_k(int k)
{
    vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> pairs;
    for (auto iter = data->begin(); iter != data->end(); ++iter)
    {
        pairs.push_back(*iter);
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