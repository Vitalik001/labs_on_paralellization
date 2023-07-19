#include "DiskReader.h"

enum RETURN_CODES {
    SUCCESS = 0,
    INVALID_NUMBER_OF_ARGUMENTS = 1,
    ERROR_OPENING_CONFIG = 3,
    ERROR_OPENING_OUTPUT_FILE = 4,
    INCORRECT_CONFIG_FILE = 5,
    ERROR_WRITING_TO_FILE = 6,
    INDIR_DOES_NOT_EXIST = 26,
    OUTPUT_FILE_ERROR = 27
};

fs::path DiskReader::collect_disk_entries(oneapi::tbb::flow_control& fc) {
    if (iterator != end_iterator) {
        auto file_path = iterator->path();
        iterator++;
        if (is_file(file_path) || is_archive(file_path)) return file_path;
    } else {
        fc.stop();
    }

    return "";
}

se_pair DiskReader::save_disk_entries(fs::path& file_path) {
    if (file_path == "") return {"", ""};
    std::ifstream raw_file(file_path, std::ios::binary);

    auto buffer = static_cast<std::ostringstream&>(
            (std::ostringstream &) (std::ostringstream {} << raw_file.rdbuf())).str();

    return {std::move(buffer), file_path};
}

void DiskReader::process_disk_entries(se_pair& entry) {
    if (is_file(entry.second)) {
        try {
            perform_indexing(entry.first, entry.second);
        } catch (std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
        }
    } else if (is_archive(entry.second)) {
        try {
            process_archive(entry.first, entry.second);
        } catch (std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
        }
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

void DiskReader::process_archive_entry(archive_entry *archive_e, archive *a) {
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

    perform_indexing(buffer, archive_entry_pathname(archive_e));
}

void DiskReader::process_archive(const std::string& archive, fs::path& path) {
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
            process_archive_entry(archive_e, a);
        } catch (std::runtime_error& err) {
            std::cerr << "Couldn't index this file: " << err.what() << " in archive " << path << std::endl;
        }

    }

    archive_read_close(a);
    archive_read_free(a);
}

void DiskReader::perform_indexing(const std::string& data, const fs::path& path) {
    try {
        auto processed_file = bl::fold_case(bl::normalize(data, bl::norm_nfc));

        bl::boundary::ssegment_index map(
                bl::boundary::word,
                processed_file.begin(),
                processed_file.end());

        map.rule(boost::locale::boundary::word_letters);

        for (bl::boundary::ssegment_index::iterator it = map.begin(), e = map.end(); it != e; ++it) {
            if(word_map.contains(*it)){
                word_map[*it] += 1;
            }
            else{
                word_map[*it] = 1;
            }
        }
    }
    catch (std::runtime_error &err) {
        std::cerr << "Error happened while processing this file: " << path << std::endl;
    }
}

bool compare_num_entries(const std::pair<std::string, size_t> &a, const std::pair<std::string, size_t> &b) {
    return a.second > b.second;
}

bool compare_alphabet(const std::pair<std::string, size_t> &a, const std::pair<std::string, size_t> &b) {
    return a.first < b.first;
}

void DiskReader::record_results() {
    try {
    std::ofstream progress_file_n(out_by_n);

    std::vector<std::pair<std::string, size_t>> sorted_word_count(word_map.begin(), word_map.end());
    std::sort(sorted_word_count.begin(), sorted_word_count.end(), compare_num_entries);

    for (const auto &[word, count]: sorted_word_count) {
        progress_file_n << word << " " << count << std::endl;
    }

    progress_file_n.close();

    std::ofstream progress_file_a(out_by_a);


    std::sort(sorted_word_count.begin(), sorted_word_count.end(), compare_alphabet);

    for (const auto &[word, count]: sorted_word_count) {
        progress_file_a << word << " " << count << std::endl;
    }

    progress_file_a.close();
}  catch (std::ios_base::failure& e) {
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