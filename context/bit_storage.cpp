#include "bit_storage.h"
#include "list_storage.h"

void bit_storage::generate_randomly(DOMAIN_TYPE count, DOMAIN_TYPE max_value)
{
    output::start_timer("io/generate_bitstorage");
    
    for (DOMAIN_TYPE i = 0; i < count; ++i)
    {
        data->setBit(rand() % max_value, 1);
    }
    
    output::stop_timer("io/generate_bitstorage");
}

void bit_storage::load_from_file(string name, long offset, long length)
{
    output::start_timer("io/load_bitstorage");
 
    int fdes = open(storage_data_file_name(name).c_str(), O_RDONLY);
    ibis::array_t<word_t> arr = ibis::array_t<word_t>(fdes, offset, offset + length);
    
    delete data;
    data = new ibis::bitvector(arr);
    close(fdes);
    
    output::stop_timer("io/load_bitstorage");
}

long bit_storage::save_to_file(string name, pair<long, long> &position)
{
    output::start_timer("io/save_bitstorage");
    
    data->compress();
    
    long write_pos = offset::next_offset_for(name);
    FILE *file = fopen(storage_data_file_name(name).c_str(), "a");
    
    fseek(file, write_pos, 0);
    ibis::array_t<word_t> arr = ibis::array_t<word_t>();
    data->write(arr);
    arr.write(file);
    
    long bytes_written = ftell(file) - write_pos;
    offset::handle_bytes_written(name, bytes_written);
    
    fclose(file);
    
    position = make_pair(write_pos, bytes_written);
    output::stop_timer("io/save_bitstorage");
    output::increment_stat("io-bytes/bitvector-written-uncompressed", data->size()/8);
    output::increment_stat("io-bytes/bitvector-written-compressed", data->getSerialSize());
    
    return bytes_written;
}

bit_storage::bit_storage()
{
    type = STORAGE_TYPE_BITVECTOR;
    data = new ibis::bitvector();
}

bit_storage::~bit_storage()
{
    delete data;
}

void bit_storage::intersect(storage &another_storage)
{
    output::start_timer("run/intersect_bitstorage");
    
    if (another_storage.type == STORAGE_TYPE_BITVECTOR)
    {
        *data &= *static_cast<bit_storage*>(&another_storage)->data;
    }
    else if (another_storage.type == STORAGE_TYPE_LIST)
    {
        ibis::bitvector *other_bv = static_cast<list_storage*>(&another_storage)->to_bit_vector();
        *data &= *other_bv;
        delete other_bv;
    }
    
    output::stop_timer("run/intersect_bitstorage");
}

long bit_storage::count()
{
    output::start_timer("run/count_bitstorage");
    
    return data->cnt();
    
    output::stop_timer("run/count_bitstorage");
}

vector<DOMAIN_TYPE> *bit_storage::elements()
{
    output::start_timer("run/elements_bitstorage");
    
    vector<DOMAIN_TYPE> *result = new vector<DOMAIN_TYPE>();
    ibis::bitvector::indexSet ones = data->firstIndexSet();
    bool is_range = ones.isRange();
    
    while (ones.nIndices() != 0)
    {
        for (int i = 0; i < ones.nIndices(); ++i)
        {
            if (is_range)
            {
                result->push_back(ones.indices()[0] + i);
            }
            else
            {
                result->push_back(ones.indices()[i]);
            }
        }
        
        ++ones;
    }
    
    output::stop_timer("run/elements_bitstorage");
    return result;
}

void bit_storage::add(DOMAIN_TYPE item)
{
    data->setBit(item, 1);
}

