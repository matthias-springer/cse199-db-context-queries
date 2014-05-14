#include "self_test.h"
#include "input.h"
#include "storage.h"
#include "offset.h"
#include "count_context_query.h"
#include "top_k_query.h"

namespace benchmark
{
    void run_self_test()
    {
        pair<long, long> position;
        
        storage *d = storage::new_instance();
        d->add(1);
        d->add(5);
        d->add(7);
        d->add(10);
        d->add(1005);
        d->save_to_file("document", position);
        offset::set_offsets_for("document", 1, position.first, position.second);
        
        d = storage::new_instance();
        d->add(2);
        d->add(7);
        d->add(8);
        d->add(10);
        d->add(2000);
        d->save_to_file("document", position);
        offset::set_offsets_for("document", 2, position.first, position.second);
        
        d = storage::new_instance();
        d->add(2);
        d->add(5);
        d->add(7);
        d->add(9);
        d->add(3000);
        d->save_to_file("document", position);
        offset::set_offsets_for("document", 3, position.first, position.second);
        
        d = storage::new_instance();
        d->add(2);
        d->add(7);
        d->add(9);
        d->add(1000);
        d->add(5000);
        d->save_to_file("document", position);
        offset::set_offsets_for("document", 4, position.first, position.second);
        
        d = storage::new_instance();
        d->add(3);
        d->add(4);
        d->add(7);
        d->add(2000);
        d->save_to_file("document", position);
        offset::set_offsets_for("document", 5, position.first, position.second);
        
        storage *t = storage::new_instance();
        t->add(1);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 1, position.first, position.second);
        
        t = storage::new_instance();
        t->add(2);
        t->add(3);
        t->add(4);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 2, position.first, position.second);
        
        t = storage::new_instance();
        t->add(5);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 3, position.first, position.second);
        
        t = storage::new_instance();
        t->add(5);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 4, position.first, position.second);
        
        t = storage::new_instance();
        t->add(1);
        t->add(3);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 5, position.first, position.second);
        
        t = storage::new_instance();
        t->add(1);
        t->add(2);
        t->add(3);
        t->add(4);
        t->add(5);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 7, position.first, position.second);
        
        t = storage::new_instance();
        t->add(2);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 8, position.first, position.second);
        
        t = storage::new_instance();
        t->add(3);
        t->add(4);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 9, position.first, position.second);
        
        t = storage::new_instance();
        t->add(1);
        t->add(2);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 10, position.first, position.second);
        
        t = storage::new_instance();
        t->add(4);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 1000, position.first, position.second);
        
        t = storage::new_instance();
        t->add(1);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 1005, position.first, position.second);
        
        t = storage::new_instance();
        t->add(2);
        t->add(5);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 2000, position.first, position.second);
        
        t = storage::new_instance();
        t->add(3);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 3000, position.first, position.second);
        
        t = storage::new_instance();
        t->add(4);
        t->save_to_file("term", position);
        offset::set_offsets_for("term", 5000, position.first, position.second);
        
        offset::close_offset_files();
        
        vector<DOMAIN_TYPE> context;
        context.push_back(9);
        context.push_back(7);
        context.push_back(2);
        
        vector<DOMAIN_TYPE> *result = count_context_query::documents_in_context(&context);
        
        if (result->size() == 2 && (result->at(0) == 3 || result->at(1) == 3) && (result->at(0) == 4 || result->at(1) == 4))
        {
            show_info("Passed count_context_query.");
        }
        else
        {
            error("Failed count_context_query.");
        }
    }
}