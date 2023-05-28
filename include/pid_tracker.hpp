#pragma once
#include <iostream>
#include <deque>

class PidHistoryTracker {
 public:
    PidHistoryTracker(size_t max_size) { max_size_ = max_size; }

    void push(uint32_t pid) {
        if (!pids_.size()) {
            pids_.push_back(pid);
        } else {
            if (pids_.back() == pid) return;
            if (max_size_ == size()) { pids_.pop_front(); }
            if (!contains(pid)) pids_.push_back(pid);
        }
    }
    bool contains(uint32_t pid) {
        for (auto it : pids_) {
            if (it == pid) { return true; }
        }
        return false;
    }
    void dump() {
        for (auto it : pids_) {
            std::cout << it << " ";
        }
        std::cout << "\n";
    }

    size_t size() { return pids_.size(); }

 private:
    size_t max_size_;
    std::deque<uint32_t> pids_;
};
