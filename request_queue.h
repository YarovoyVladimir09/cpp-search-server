#pragma once

#include <algorithm>
#include <vector>
#include <deque>
#include <string>
#include "search_server.h"
#include "document.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) :query_class(search_server) {
    }

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        bool result;
    };

    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& query_class;
};

template <typename DocumentPredicate>
inline std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {

    std::vector<Document> request = RequestQueue::query_class.FindTopDocuments(raw_query, document_predicate);
    RequestQueue::QueryResult answer;
    answer.result = (!request.empty());
    RequestQueue::requests_.push_back(answer);
    if (RequestQueue::requests_.size() > min_in_day_)
        RequestQueue::requests_.pop_front();
    return request;
}

inline std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    return RequestQueue::AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

inline std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    return RequestQueue::AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}