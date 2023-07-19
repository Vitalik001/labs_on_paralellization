#include "DiskReader.h"

enum RETURN_CODES {
    SUCCESS = 0,
    INVALID_NUMBER_OF_ARGUMENTS = 1,
    ERROR_OPENING_CONFIG = 3,
    ERROR_OPENING_OUTPUT_FILE = 4,
    INCORRECT_CONFIG_FILE = 5,
    ERROR_WRITING_TO_FILE = 6,
    INDIR_DOES_NOT_EXIST = 26,
    OUTPUT_FILE_ERROR = 28
};

void DiskReader::collect_disk_entries() {
    try {
    if (!fs::exists(input_directory)) {
        throw std::invalid_argument("Input directory does not exist!");
    } else if (!fs::is_directory(input_directory)) {
        throw std::invalid_argument("Input directory is not a directory!");
    } else if (fs::is_empty(input_directory)) {
        throw std::invalid_argument("Input directory is empty!");
    } else if (fs::status(input_directory).permissions() == fs::perms::none) {
        throw std::invalid_argument("No permissions to read input directory!");
    } else if (fs::status(input_directory).permissions() == fs::perms::unknown) {
        throw std::invalid_argument("Unknown permissions to read input directory!");
    } else if (fs::status(input_directory).permissions() == fs::perms::mask) {
        throw std::invalid_argument("Mask permissions to read input directory!");
    }
    for (auto &entry: fs::recursive_directory_iterator(input_directory)) {
        if (!is_file(entry.path()) && !is_archive(entry.path())) continue;

        filenames_queue.enqueue(entry.path());
    }

    filenames_queue.enqueue("\0");
} catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    exit(INDIR_DOES_NOT_EXIST);
}
}

bool DiskReader::validate_file(archive_entry* entry) {
    fs::path archive_entry_path = archive_entry_pathname(entry);

    return
    files_extensions.count(archive_entry_path.filename().extension())
    &&
    (archive_entry_size(entry) < file_max_size);
}

bool DiskReader::is_file(const fs::path &entry_path) {
    return files_extensions.find(entry_path.filename().extension()) != files_extensions.end();
}

bool DiskReader::is_archive(const fs::path &entry_path) {
    return archives_extensions.find(entry_path.filename().extension()) != archives_extensions.end();
}

void DiskReader::save_disk_entries() {
    while (true) {
        auto queue_entry = filenames_queue.dequeue();

        if (queue_entry == "\0") {
            return string_queue.enqueue({ std::move(queue_entry), std::move(queue_entry) });
        }

        std::ifstream raw_file(queue_entry, std::ios::binary);

        auto buffer = static_cast<std::ostringstream&>(
                (std::ostringstream &) (std::ostringstream {} << raw_file.rdbuf())).str();

        string_queue.enqueue({std::move(buffer), std::move(queue_entry)});
    }
}

void DiskReader::process_archive_entry(archive_entry *archive_e, archive *a, word_count_map& word_map) {
    std::string buffer;
    la_int64_t offset = 0;
    size_t block_size = 0;

    const void *data;
    int r;

    while (true) {
        r = archive_read_data_block(a, &data, &block_size, &offset);
        if (r == ARCHIVE_EOF) break;
        if (r < 0) {
            throw std::runtime_error(archive_entry_pathname(archive_e));
        }

        buffer.append(static_cast<const char *>(data), block_size);
    }

    perform_indexing(buffer, archive_entry_pathname(archive_e), word_map);
}

void DiskReader::process_archive(const std::string& archive, fs::path& path, word_count_map& word_map) {
    struct archive *a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    if (archive_read_open_memory(a, archive.data(), archive.size()) != ARCHIVE_OK) {
        throw std::runtime_error(path);
    }

    struct archive_entry *archive_e;

    while (archive_read_next_header(a, &archive_e) == ARCHIVE_OK) {
        if (!validate_file(archive_e)) continue;

        try {
            process_archive_entry(archive_e, a, word_map);
        } catch (std::runtime_error& err) {
            std::cerr << "Couldn't index this file: " << err.what() << " in archive " << path << std::endl;
        }

    }

    archive_read_close(a);
    archive_read_free(a);
}

void DiskReader::perform_indexing(const std::string& data, const fs::path& path, word_count_map& word_map) {
    try {
        auto processed_file = bl::fold_case(bl::normalize(data, bl::norm_nfc));

        bl::boundary::ssegment_index map(
                bl::boundary::word,
                processed_file.begin(),
                processed_file.end());

        map.rule(boost::locale::boundary::word_letters);

        for (bl::boundary::ssegment_index::iterator it = map.begin(), e = map.end(); it != e; ++it) {
            ++word_map[*it];
        }
    }
    catch (std::runtime_error &err) {
        std::cerr << "Error happened while processing this file: " << path << std::endl;
    }
}

void DiskReader::process_disk_entries() {
    workers += 1;
    while (true) {
        word_count_map word_map;

        se_pair entry = string_queue.dequeue();

        if (entry.second == "\0") {
            workers -= 1;
            return string_queue.enqueue({ "\0", "\0" });
        }

        if (is_file(entry.second)) {
            try {
                perform_indexing(entry.first, entry.second, word_map);
            } catch (std::runtime_error& e) {
                std::cerr << e.what() << std::endl;
            }
        } else {
            try {
                process_archive(entry.first, entry.second, word_map);
            } catch (std::runtime_error& e) {
                std::cerr << e.what() << std::endl;
            }
        }

        if (!word_map.empty()) {
            counter += 1;
            dictionary_queue.enqueue(std::move(word_map));
        }
    }
}

void DiskReader::merge_dictionaries() {
    while (true) {
        if (workers == 0 && counter == 1) {
            word_count_map mp;
            mp["\0"] = -1;
            dictionary_queue.enqueue(std::move(mp));
            break;
        }

        map_pair dicts = dictionary_queue.dequeue_duo();

        if (dicts.first.find("\0") != dicts.first.end()) {
            word_count_map mp;
            dictionary_queue.enqueue(dicts.first);
            dictionary_queue.enqueue(dicts.second);
            break;
        } else if (dicts.second.find("\0") != dicts.second.end()) {
            dictionary_queue.enqueue(dicts.second);
            dictionary_queue.enqueue(dicts.first);
            break;
        }

        for (const auto &[word, count]: dicts.second) {
            dicts.first[word] += count;
        }

        dictionary_queue.enqueue(std::move(dicts.first));
        counter -= 1;
    }
}

bool compare(const std::pair<std::string, size_t> &a, const std::pair<std::string, size_t> &b) {
    return a.second > b.second;
}

void DiskReader::record_results() {
    try {
    std::ofstream progress_file_n(out_by_a);
    word_count_map words;
    auto pair = dictionary_queue.dequeue_duo();

    if (pair.first.find("\0") != pair.first.end()) {
        words = pair.second;
    } else  {
        words = pair.first;
    }

    for (const auto &[word, count]: words) {
        progress_file_n << word << " " << count << std::endl;
    }

    progress_file_n.close();

    std::ofstream progress_file_a(out_by_n);

    std::vector<std::pair<std::string, size_t>> sorted_word_count(words.begin(), words.end());
    std::sort(sorted_word_count.begin(), sorted_word_count.end(), compare);

    for (const auto &[word, count]: sorted_word_count) {
        progress_file_a << word << " " << count << std::endl;
    }

    progress_file_a.close();
} catch (std::ios_base::failure& e) {
        std::cerr << e.what() << std::endl;
        exit(ERROR_OPENING_OUTPUT_FILE);
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        exit(ERROR_WRITING_TO_FILE);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(ERROR_OPENING_OUTPUT_FILE);
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        exit(ERROR_OPENING_OUTPUT_FILE);
    }
}
