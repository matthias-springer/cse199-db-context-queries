#ifndef __context__column_list_storage__
#define __context__column_list_storage__

#include <iostream>
#include "storage.h"

template<class TAttribute>
class column_list_storage : public storage
{
private:
    vector<TAttribute> data;
    
public:
    column_list_storage()
    {
        type = STORAGE_TYPE_COLUMN_LIST;
    }
};

#endif
