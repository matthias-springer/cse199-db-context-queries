#ifndef __context__top_k_tf_column_db_query__
#define __context__top_k_tf_column_db_query__

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include "context.h"
#include "storage.h"
#include "input.h"
#include "map_aggregation.h"

namespace top_k_tf_column_db_query
{
    void generate_random_tuples();
    unordered_set<DOMAIN_TYPE> *documents_in_context(vector<DOMAIN_TYPE> *terms);
    vector<DOMAIN_TYPE> *top_k_tf_in_documents(unordered_set<DOMAIN_TYPE> *documents, int k);
    vector<DOMAIN_TYPE> *top_k_tf_in_context(vector<DOMAIN_TYPE> *context, int k);
}

#endif
