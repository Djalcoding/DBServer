
#pragma once
#include "logger.h"
#include <cassert>
#include <filesystem>
#include <fstream>
class FileReader {
    using path_t = std::filesystem::path;
    path_t      filepath;
    std::string contents;

  public:
    void load() {
        if (!contents.empty())
            return;
        constexpr std::size_t read_size = 65536;
        std::ifstream         reader = std::ifstream(filepath);
        Logger::getInstance()->push({"FILE_READER", std::format("Opened {}", filepath.string())});
        reader.exceptions(std::ios_base::badbit);
        if (!reader)
            throw std::ios_base::failure("file doesn't exist");

        std::string buffer = std::string(read_size, '\0');
        while (reader.read(&buffer[0], buffer.size())) {
            Logger::getInstance()->push(
                {"FILE_READER", std::format("Read {} bytes from {}", reader.gcount(), filepath.string())});
            contents.append(buffer, 0, reader.gcount());
        }
        Logger::getInstance()->push(
            {"FILE_READER", std::format("Read {} bytes from {}", reader.gcount(), filepath.string())});
        contents.append(buffer, 0, reader.gcount());
        Logger::getInstance()->push(
            {"FILE_READER", std::format("Loaded a total of {} bytes from {}", contents.size(), filepath.string())});
    }

    FileReader(const path_t &path) : filepath(path) {
        assert(std::filesystem::exists(path));
        assert(std::filesystem::is_regular_file(filepath));
    }
    FileReader(const FileReader &) = delete;
    FileReader &operator=(const FileReader &) = delete;
    FileReader &operator=(FileReader &&moved) {
        this->filepath = std::move(moved.filepath);
        this->contents = std::move(moved.contents);
        return *this;
    };
    FileReader(FileReader &&moved) { *this = std::move(moved); };
    template <class Self> auto&& get_contents(this Self &&self) {
        self.load();
        return std::forward<Self>(self).contents;
    }
};
