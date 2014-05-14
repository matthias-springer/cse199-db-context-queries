#ifndef __context__map_aggregation__
#define __context__map_aggregation__

#include <iostream>
#include <map>
#include "aggregation.h"

using namespace std;

class map_aggregation : public aggregation
{
private:
    map<DOMAIN_TYPE, DOMAIN_TYPE> *data;
    
public:
    map_aggregation();
    virtual void add(DOMAIN_TYPE key, DOMAIN_TYPE increment);
    virtual vector<pair<DOMAIN_TYPE, DOMAIN_TYPE>> *top_k(int k);
};

#endif
