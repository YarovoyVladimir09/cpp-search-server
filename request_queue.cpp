#include "request_queue.h"

int RequestQueue::GetNoResultRequests() const {
    return std::count_if(requests_.begin(), requests_.end(), [](QueryResult step)
        {
            return step.result == 0;
        });
}
 std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return RequestQueue::AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

 std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    return RequestQueue::AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}