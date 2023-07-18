#include "Notifier.hpp"

using std::chrono::duration;
using std::chrono::milliseconds;
using std::chrono::seconds;

NotifyState Notifier::work(const DataResult& d) {
    filteredDataResult f_d = filterPerson(d);
    NotifyState result = NotifyState::SKIPPING;
    if (!f_d.objs.empty()) result = doWork(f_d);
    //std::cout << "[notify] result " << static_cast<int>(result) << "\n";
    return result;
}

milliseconds Notifier::getCurrentTime() {
    std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto current =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return current;
}

filteredDataResult Notifier::filterPerson(const DataResult& d) {
    filteredDataResult filtered_d;
    for (auto& obj : d.objs) {
        if (d.names.size() > obj.obj_id) {
            if (d.names[obj.obj_id] == std::string("person")) {
                if (filtered_d.frame.empty()) filtered_d.frame = d.frame;
                filtered_d.objs.push_back(obj);
            }
        }
    }
    filtered_d.fps = d.fps;
    return filtered_d;
}
