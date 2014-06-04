#ifndef __context__top_k_tf_dual_list_query__
#define __context__top_k_tf_dual_list_query__

#include <iostream>
#include "context.h"
#include "storage.h"

namespace top_k_tf_dual_list_query
{
    vector<DOMAIN_TYPE> *top_k_tf_in_context(vector<DOMAIN_TYPE> *context, int k);
    vector<DOMAIN_TYPE> *top_k_tf_in_documents(vector<DOMAIN_TYPE> *documents, int k);
}


#endif
