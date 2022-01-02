#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
    std::map<std::set<std::string>, std::stack<int>> for_duplicates;
    std::stack<int> duplicates;
    for (const int document_id : search_server) {
        std::set<std::string> all_words_in_doc;
        auto words_and_freq_on_doc = search_server.GetWordFrequencies(document_id);
        for (auto [words, freq] : words_and_freq_on_doc)
        {
            all_words_in_doc.insert(words);
        }
        if (for_duplicates[all_words_in_doc].size() > 0)
        {
            std::cout << "Found duplicate document id " << document_id << std::endl;
            duplicates.push(document_id);
        }
        else {
            for_duplicates[all_words_in_doc].push(document_id);
        }
    }
    while (!duplicates.empty())
    {
        search_server.RemoveDocument(duplicates.top());
        duplicates.pop();

    }
}