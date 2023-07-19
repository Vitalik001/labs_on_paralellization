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
#include <oneapi/tbb/concurrent_unordered_map.h>
#include <oneapi/tbb/concurrent_queue.h>
#include <thread>
#include <fstream>
#include <atomic>

#define fs std::filesystem
#define bl boost::locale

typedef tbb::concurrent_unordered_map<std::string, std::atomic<size_t>> word_count_map;

using se_pair = std::pair<std::string, fs::path>;
using names_queue_type =  tbb::concurrent_bounded_queue<fs::path>;
using string_queue_type =  tbb::concurrent_bounded_queue<se_pair>;


class DiskReader {
    std::string input_directory;
    size_t file_max_size;
    std::set<std::string> files_extensions;
    std::set<std::string> archives_extensions;
    names_queue_type filenames_queue;
    string_queue_type string_queue;
    word_count_map word_map;
    std::string out_by_n;
    std::string out_by_a;
public:
    explicit DiskReader(
            boost::program_options::variables_map& options
    ):
            input_directory(options["indir"].as<std::string>()),
            file_max_size(options["max_file_size"].as<int>()),
            out_by_a(options["out_by_a"].as<std::string>()),
            out_by_n(options["out_by_n"].as<std::string>())
    {
        auto indexing_ext = options["indexing_extensions"].as<std::vector<std::wstring>>();
        auto archives_ext = options["archives_extensions"].as<std::vector<std::wstring>>();

        string_queue.set_capacity(options["raw_files_queue_size"].as<int>());
        filenames_queue.set_capacity(options["filenames_queue_size"].as<int>());

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

    void process_archive(const std::string& archive, fs::path& path);

    void process_archive_entry(archive_entry *archive_e, archive *a);

    void perform_indexing(const std::string& data, const fs::path& path);

    bool is_archive(const fs::path& entry_path);

    bool is_file(const fs::path& entry_path);

    bool validate_file(archive_entry* entry);

    void record_results();
};

#endif //COUNTWORDS_PAR_DISKREADER_H
