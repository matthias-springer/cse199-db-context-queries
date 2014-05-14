#include "array_aggregration.h"

void array_aggregation::add(DOMAIN_TYPE key, DOMAIN_TYPE increment)
{
    data[key] += increment;
}

array_aggregation::array_aggregation(long size)
{
    data = new DOMAIN_TYPE[size];
}

vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *array_aggregation::top_k(int k)
{
    return NULL;
}
