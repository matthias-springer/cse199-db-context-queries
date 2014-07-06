#ifndef __context__count_context_query__
#define __context__count_context_query__

#include <iostream>
#include "context.h"
#include "storage.h"

namespace count_context_query
{
    long size_of_context(vector<DOMAIN_TYPE> *context);
    vector<DOMAIN_TYPE> *documents_in_context(vector<DOMAIN_TYPE> *context);
    void stdin_documents_in_context();
    void randomly_generate_vectors();
}

#endif
