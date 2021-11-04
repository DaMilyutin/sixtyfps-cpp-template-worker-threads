#include "appwindow.h"

#include "Property.h"

#include <future>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <assert.h>

#include "../_lib/ThreadPool/ThreadPool.hpp"

int nextId()
{
    static int n = 0;
    return n = (n+1)%(1<<15);
}

struct IdMap
{
    std::unordered_map<int, int> id2row;
    std::vector<int>             row2id;

    void push(int id, int row)
    {
        row2id.push_back(id);
        id2row.emplace(id, int(row2id.size()-1));
    }

    void erase(int row)
    {
        assert(0 <=row && row < row2id.size());
        for(int r = row+1; r < row2id.size(); ++r)
            --id2row[row2id[r]];
        id2row.erase(row2id[row]);       
        row2id.erase(row2id.begin() + row);
    }
};

int main(int argc, char **argv)
{
    auto ui = AppWindow::create();

    auto task_data_model = std::make_shared<sixtyfps::VectorModel<ListItemData>>();
    auto task_id2index = std::make_shared<IdMap>();

    ui->set_task_data_model(task_data_model);

    auto counter = F60_PROPERTY(counter, int, ui);// make_property<int>([ui](){ return ui->get_counter();}, [ui](int i){ui->set_counter(i);});

    dbr::cc::ThreadPool pool;

    auto do_job = [&](int id, float latency){
            auto const period = std::chrono::milliseconds{int(10.*latency)};
            for(int i = 1; i <= 100; ++i)
            {
                std::this_thread::sleep_for(period);
                sixtyfps::blocking_invoke_from_event_loop([&]{
                    auto r = task_id2index->id2row[id];
                    task_data_model->set_row_data(r, ListItemData{id, float(i)});
                });
            }
            std::this_thread::sleep_for(std::chrono::seconds{1});
            // may call counter like this:
            // counter.blocking() += 1;
            sixtyfps::blocking_invoke_from_event_loop([&] {                
                counter += 1; // but this is more optimal here
                auto r = task_id2index->id2row[id];
                task_data_model->erase(r);
                task_id2index->erase(r);
            });
        };

    ui->on_request_increase_value([&]{
        int id = nextId();
        float latency = ui->get_latency();
        int r = task_data_model->row_count(); 
        task_data_model->push_back(ListItemData{id, 0.});
        task_id2index->push(id, r);
        pool.add(do_job, id, latency);
        //std::thread(do_job, id, latency).detach();
    });
    
    ui->run();
    pool.destroy_detach();
    return 0;
}
