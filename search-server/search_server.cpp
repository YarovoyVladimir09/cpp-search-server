#include "search_server.h"

using std::string_literals::operator""s;

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
    if ((document_id < 0) || (SearchServer::documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    const auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        SearchServer::word_to_document_freqs_[word][document_id] += inv_word_count;
        SearchServer::words_freq_in_doc_[document_id][word] += inv_word_count;
    }
    SearchServer::documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    SearchServer::document_ids_.insert(document_id);
}

int SearchServer::GetDocumentCount() const {
    return SearchServer::documents_.size();
}

const std::set<int>::const_iterator SearchServer::begin() const
{
    return SearchServer::document_ids_.cbegin();
}

const std::set<int>::const_iterator SearchServer::end() const
{
    return SearchServer::document_ids_.cend();
}

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const{
    //static std::map < std::string, double > result = SearchServer::words_freq_in_doc_.at(document_id);
    return SearchServer::words_freq_in_doc_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {
    
    SearchServer::documents_.erase(document_id);
    SearchServer::document_ids_.erase(document_id);
    for (auto step : SearchServer::word_to_document_freqs_)
    {
        if (step.second.count(document_id))
            step.second.erase(document_id);
    }
    SearchServer::words_freq_in_doc_.erase(document_id);

}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query);

    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (SearchServer::word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (SearchServer::word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (SearchServer::word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (SearchServer::word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return { matched_words, SearchServer::documents_.at(document_id).status };
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

 bool SearchServer::IsValidWord(const std::string& word) {
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word "s + word + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.cbegin(), ratings.cend(), 0);
    return (rating_sum / static_cast<int>(ratings.size()));
}

    SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const {
        if (text.empty()) {
            throw std::invalid_argument("Query word is empty"s);
        }
        std::string word = text;
        bool is_minus = false;
        if (word[0] == '-') {
            is_minus = true;
            word = word.substr(1);
        }
        if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
            throw std::invalid_argument("Query word "s + text + " is invalid"s);
        }

        return { word, is_minus, IsStopWord(word) };
    }

    SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
        Query result;
    for (const std::string& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            }
            else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

 std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return SearchServer::FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

 std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}