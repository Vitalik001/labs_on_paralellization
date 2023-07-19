//
// Created by Nazar Kononenko on 12.04.2023.
//

#ifndef COUNTWORDS_PAR_DISKREADER_H
#define COUNTWORDS_PAR_DISKREADER_H

#include <iostream>
#include "options_parser.h"
#include <archive.h>
#include <string>
#include <vector>
#include <archive_entry.h>
#include <filesystem>
#include <set>
#include <boost/locale.hpp>
#include <unordered_map>
#include <map>
#include "../threadsafe_queue/threadsafe_queue.h"
#include <thread>
#include <fstream>
#include <atomic>

#define fs std::filesystem
#define bl boost::locale
#define ORDERED

#ifdef ORDERED
typedef std::map<std::string, int> word_count_map;
#else
typedef std::unordered_map<std::string, int> word_count_map;
#endif

using se_pair = std::pair<std::string, fs::path>;
using map_pair = std::pair<word_count_map, word_count_map>;

class DiskReader {
    std::string input_directory;
    size_t file_max_size;
    std::set<std::string> files_extensions;
    std::set<std::string> archives_extensions;
    threadsafe_queue<fs::path> filenames_queue;
    threadsafe_queue<se_pair> string_queue;
    std::string out_by_n;
    std::string out_by_a;
    threadsafe_queue<word_count_map> dictionary_queue;
    std::atomic<size_t> counter = 0;
    std::atomic<size_t> workers = 0;
public:
    explicit DiskReader(
            boost::program_options::variables_map& options
            ):
            input_directory(options["indir"].as<std::string>()),
            file_max_size(options["max_file_size"].as<int>()),
            filenames_queue(options["filenames_queue_size"].as<int>()),
            string_queue(options["raw_files_queue_size"].as<int>()),
            dictionary_queue(options["dictionaries_queue_size"].as<int>()),
            out_by_a(options["out_by_a"].as<std::string>()),
            out_by_n(options["out_by_n"].as<std::string>())
            {
                auto indexing_ext = options["indexing_extensions"].as<std::vector<std::wstring>>();
                auto archives_ext = options["archives_extensions"].as<std::vector<std::wstring>>();

                for (auto &i: indexing_ext) {
                    files_extensions.insert(std::string(i.begin(), i.end()));
                }

                for (auto &i: archives_ext) {
                    archives_extensions.insert(std::string(i.begin(), i.end()));
                }
            }

    void collect_disk_entries();

    void save_disk_entries();

    void process_disk_entries();

    void process_archive(const std::string& archive, fs::path& path,  word_count_map& word_map);

    void process_archive_entry(archive_entry *archive_e, archive *a, word_count_map& word_map);

    void perform_indexing(const std::string& data, const fs::path& path, word_count_map& word_map);

    bool is_archive(const fs::path& entry_path);

    bool is_file(const fs::path& entry_path);

    bool validate_file(archive_entry* entry);

    void merge_dictionaries();

    void record_results();
};

#endif //COUNTWORDS_PAR_DISKREADER_H
