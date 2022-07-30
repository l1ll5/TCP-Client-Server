#include "Timer.hpp"

size_t Timer::getShortest() noexcept
{
    run();
    if (empty())
        return 1000;
    struct timeval now, timeout;
    gettimeofday(&now, nullptr);
    auto it = *timerSet.begin();
    timersub(&it->timeout, &now, &timeout);
    size_t ret = (timeout.tv_sec * 1000 + timeout.tv_usec / 1000) + 1;
    return ret > 0 ? ret : 0;
}
size_t Timer::insert(size_t sec, size_t usec, std::function<int(int)> task, int data, bool persistent) noexcept
{
    size_t id = getId();
    std::shared_ptr<timer> newTimer(new timer);
    idMap[id] = newTimer;

    timerclear(&newTimer->interval);
    newTimer->interval.tv_sec = sec;
    newTimer->interval.tv_usec = usec;
    newTimer->data = data;
    newTimer->task = task;

    struct timeval now;
    gettimeofday(&now, nullptr);

    timeradd(&now, &newTimer->interval, &newTimer->timeout);
    newTimer->persistent = persistent;

    timerSet.insert(newTimer);
    return id;
}
bool Timer::set_persistent(size_t id) noexcept
{
    if (!idMap.count(id))
        return 0;
    return idMap[id]->persistent = 1;
}
bool Timer::erase(size_t id) noexcept
{
    if (!idMap.count(id))
        return 0;
    auto it = idMap[id];
    timerSet.erase(it), idMap.erase(id);
    return 1;
}

size_t Timer::getId()
{
    return maxid++;
}

void Timer::run() noexcept
{
    //run to now
    if (empty())
        return;
    struct timeval now;
    gettimeofday(&now, nullptr);

    std::vector<std::shared_ptr<timer>> insertList;
    while (!timerSet.empty())
    {
        auto it = timerSet.begin();
        auto curTimer = *it;
        if (timercmp(&curTimer->timeout, &now, >))
            break;
        int ret = curTimer->task(curTimer->data);
        timerSet.erase(timerSet.find(curTimer));

        if (curTimer->persistent == false or ret == 1)
        {
            idMap.erase(curTimer->id);
        }
        else
        {
            timeradd(&now, &curTimer->interval, &curTimer->timeout);
            insertList.emplace_back(std::move(curTimer));
        }
    }
    for (auto &tmp : insertList)
        timerSet.insert(tmp);
}