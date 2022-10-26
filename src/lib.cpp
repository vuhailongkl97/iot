#include "lib.h"

void show_console_result(std::vector<obj_t> const result_vec)
{
    for (auto& i : result_vec) {
        std::cout << "obj_id = " << i.obj_id << std::setprecision(3)
                  << ", prob = " << i.prob << std::endl;
    }
}

void consoleNotifier::doWork(const filteredDataResult& d)
{
    show_console_result(d.objs);
}
