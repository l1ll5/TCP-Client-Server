#pragma once

#include <sys/time.h>
#include <functional>
#include <set>
#include <map>
#include <memory>

struct timer
{
    size_t id;
    struct timeval timeout;
    struct timeval interval;
    bool persistent;
    int data;
    std::function<int(int)> task;
};
using cmp = std::function<bool(std::shared_ptr<timer>, std::shared_ptr<timer>)>;

class Timer
{
public:
    Timer() {}
    std::set<std::shared_ptr<timer>, cmp> timerSet = std::set<std::shared_ptr<timer>, cmp>([](std::shared_ptr<timer> t1, std::shared_ptr<timer> t2)
                                                                                           { return timercmp(&t1->timeout, &t2->timeout, <); });
    std::map<size_t, std::shared_ptr<timer>> idMap;
    bool empty() { return timerSet.empty(); }
    size_t size() { return timerSet.size(); }
    size_t getShortest() noexcept;
    size_t insert(size_t sec, size_t usec, std::function<int(int)> task, int data, bool persistent = false) noexcept;
    bool set_persistent(size_t id) noexcept;
    bool erase(size_t id) noexcept;

private:
    int maxid = 0;
    size_t getId();
    void run() noexcept;
};