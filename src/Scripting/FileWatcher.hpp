/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** FileWatcher - Monitor script files for changes and auto-reload
*/

#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace GameEngine::Scripting
{
    struct FileInfo {
        std::filesystem::file_time_type lastModified;
        bool exists{};
    };

    class FileWatcher {
      public:
        FileWatcher() = default;
        ~FileWatcher() = default;

        auto addWatch(const std::string &filepath) -> void;
        auto removeWatch(const std::string &filepath) -> void;
        auto hasChanged(const std::string &filepath) -> bool;
        auto getChangedFiles() -> std::vector<std::string>;
        auto clearAll() -> void;
        auto getWatchCount() const -> size_t;

      private:
        std::unordered_map<std::string, FileInfo> _watchedFiles;

        auto updateFileInfo(const std::string &filepath) -> FileInfo;
    };

} // namespace GameEngine::Scripting
