#include "list_storage.h"
#include "bit_storage.h"

void list_storage::load_from_file(string name, long offset, long length)
{
    long cnt_elements = length / sizeof(DOMAIN_TYPE);
    DOMAIN_TYPE *read_buffer = new DOMAIN_TYPE[cnt_elements];
    
    FILE *data_file = fopen(storage_data_file_name(name).c_str(), "r");
    fseek(data_file, offset, 0);
    fread(read_buffer, sizeof(DOMAIN_TYPE), cnt_elements, data_file);
    
    data = new vector<DOMAIN_TYPE>(read_buffer, read_buffer + cnt_elements);
    
    fclose(data_file);
}

long list_storage::save_to_file(string name, pair<long, long> &position)
{
    long write_pos = offset::next_offset_for(name);
    FILE *file = fopen(storage_data_file_name(name).c_str(), "a");
    
    fseek(file, write_pos, 0);
    fwrite(&(*data)[0], sizeof(DOMAIN_TYPE), data->size(), file);
    
    long bytes_written = ftell(file) - write_pos;
    offset::handle_bytes_written(name, bytes_written);
    
    fclose(file);
    
    position = make_pair(write_pos, bytes_written);
    return bytes_written;
}

list_storage::list_storage()
{
    type = STORAGE_TYPE_LIST;
    data = new vector<DOMAIN_TYPE>;
}

ibis::bitvector *list_storage::to_bit_vector()
{
    ibis::bitvector *v = new ibis::bitvector();
    
    for (vector<DOMAIN_TYPE>::iterator iter = data->begin(); iter != data->end(); ++iter)
    {
        v->setBit(*iter, 1);
    }
    
    return v;
}

void list_storage::intersect(storage &another_storage)
{
    if (another_storage.type == STORAGE_TYPE_BITVECTOR)
    {
        ibis::bitvector *v = static_cast<bit_storage*>(&another_storage)->data;
        
        for (vector<DOMAIN_TYPE>::iterator iter = data->begin(); iter != data->end(); ++iter)
        {
            if (!v->getBit(*iter))
            {
                iter = data->erase(iter);
            }
        }
    }
    else if (another_storage.type == STORAGE_TYPE_LIST)
    {
        vector<DOMAIN_TYPE> *another_vector = static_cast<list_storage*>(&another_storage)->data;
        
        for (vector<DOMAIN_TYPE>::iterator iter = data->begin(); iter != data->end(); ++iter)
        {
            if (find(another_vector->begin(), another_vector->end(), *iter) == another_vector->end())
            {
                iter = data->erase(iter);
            }
        }
    }
}

long list_storage::count()
{
    return data->size();
}

vector<DOMAIN_TYPE> *list_storage::elements()
{
    return data;
}

void list_storage::add(DOMAIN_TYPE item)
{
    data->push_back(item);
}

