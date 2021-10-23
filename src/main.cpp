#include "appwindow.h"

#include <future>
#include <iostream>

int main(int argc, char **argv)
{
    auto ui = AppWindow::create();

    ui->on_request_increase_value([&ui]{
        std::async([&ui]{
            ui->set_progress(0.f);
            for(int i = 1; i <= 100; ++i)
            {
                std::cout << "at i = " << i << std::endl; 
                std::this_thread::sleep_for(std::chrono::milliseconds{100});
                ui->set_progress(float(i));
            }
        }).get();

        ui->set_counter(ui->get_counter() + 1);
        ui->set_progress(-1.);
    });
    
    ui->run();
    return 0;
}
