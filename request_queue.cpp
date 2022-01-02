#include "request_queue.h"

int RequestQueue::GetNoResultRequests() const {
    return std::count_if(requests_.begin(), requests_.end(), [](QueryResult step)
        {
            return step.result == 0;
        });
}