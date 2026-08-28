
#pragma once
#include "http.h"
#include <filesystem>
#include <fstream>

// all file paths must be treated with parent as the origin
class FileManager {
    using path_t = std::filesystem::path;
    const path_t parent;

  public:
    FileManager(path_t parent_dir) : parent(std::move(parent_dir)) {}

    template <http::ReadableView packet>
    void write(const path_t &location, packet view) {
        path_t path = get_full_path(location);
        std::ofstream write_stream{path, std::ios::binary};
        write_stream << view;
    }

    path_t get_full_path(const path_t &location) {
        // truncate '/' from the filepath
        return parent / std::string_view(location.native())
                            .substr(1, location.native().size());
    }

    bool exist_within_subfilesystem(const path_t &path) {
        path_t full_path = get_full_path(path);
        return std::filesystem::exists(full_path) &&
               std::filesystem::is_regular_file(full_path) &&
               !std::filesystem::canonical(full_path)
                    .lexically_relative(
                        std::filesystem::canonical(parent))
                    .generic_string()
                    .starts_with("..");
    }

    std::string hierarchy_display() {
        std::stringstream stream;
        for (auto &object :
             std::filesystem::recursive_directory_iterator(parent)) {
            std::string fullname = object.path().native();
            stream << fullname.erase(0, parent.native().size()) << RN;
        }
        return stream.str();
    }
};
