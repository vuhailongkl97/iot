#include "lib.h"

void show_console_result(std::vector<obj_t> const result_vec,
                         std::vector<std::string> const obj_names,
                         int frame_id)
{
    if (frame_id >= 0) std::cout << " Frame: " << frame_id << std::endl;
    for (auto& i : result_vec) {
        if (obj_names.size() > i.obj_id)
            std::cout << obj_names[i.obj_id] << " - ";
        std::cout << "obj_id = " << i.obj_id << std::setprecision(3)
                  << ", prob = " << i.prob << std::endl;
    }
}

void consoleNotifier::doNotify(const char* pathToFrame, float avg_prob)
{
    // fix-me
}
