#include "context.h"
#include "storage.h"
#include "bit_storage.h"
#include "list_storage.h"
#include "input.h"

storage* storage::new_from_file(string name, long offset, long length, int type)
{
    storage *s;
    
    switch (type)
    {
        case STORAGE_TYPE_BITVECTOR:
            s = new bit_storage();
            break;
        case STORAGE_TYPE_LIST:
            s = new list_storage();
            break;
        default:
            error("Invalid storage type " << type << ".");
    }
    
    s->load_from_file(name, offset, length);
    return s;
}

storage* storage::load(string name, DOMAIN_TYPE id)
{
    return new_from_file(name, offset::offset_begin(name, id), offset::offset_length(name, id), input::storage_type);
}

storage* storage::new_instance()
{
    switch (input::storage_type)
    {
        case STORAGE_TYPE_BITVECTOR:
            return new bit_storage();
            break;
        case STORAGE_TYPE_LIST:
            return new list_storage();
            break;
        default:
            error("Invalid storage type " << input::storage_type << ".");
    }
}

void storage::print_elements()
{
    vector<DOMAIN_TYPE> *v = this->elements();
    
    for (auto iter = v->begin(); iter != v->end(); ++iter)
    {
        cout << *iter << " ";
    }
    
    cout << "\n";
}