/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** FileWatcher
*/

#include "FileWatcher.hpp"
#include <iostream>

namespace GameEngine::Scripting
{

    auto FileWatcher::addWatch(const std::string &filepath) -> void
    {
        if (!std::filesystem::exists(filepath)) {
            fprintf(stderr, "[FileWatcher] Warning: File '%s' does not exist\n", filepath.c_str());
            _watchedFiles[filepath] = {std::filesystem::file_time_type::min(), false};
            return;
        }

        const auto lastWrite = std::filesystem::last_write_time(filepath);
        _watchedFiles[filepath] = {lastWrite, true};

        fprintf(stdout, "[FileWatcher] Now watching '%s'\n", filepath.c_str());
    }

    auto FileWatcher::removeWatch(const std::string &filepath) -> void
    {
        const auto it = _watchedFiles.find(filepath);
        if (it != _watchedFiles.end()) {
            _watchedFiles.erase(it);
            fprintf(stdout, "[FileWatcher] Stopped watching '%s'\n", filepath.c_str());
        }
    }

    auto FileWatcher::hasChanged(const std::string &filepath) -> bool
    {
        const auto it = _watchedFiles.find(filepath);
        if (it == _watchedFiles.end()) {
            return false;
        }

        FileInfo const newInfo = updateFileInfo(filepath);
        FileInfo &oldInfo = it->second;

        if (!oldInfo.exists && newInfo.exists) {
            oldInfo = newInfo;
            return true;
        }

        if (oldInfo.exists && !newInfo.exists) {
            oldInfo = newInfo;
            return false;
        }

        if (oldInfo.exists && newInfo.exists && newInfo.lastModified > oldInfo.lastModified) {
            oldInfo = newInfo;
            return true;
        }

        return false;
    }

    auto FileWatcher::getChangedFiles() -> std::vector<std::string>
    {
        std::vector<std::string> changedFiles;

        for (auto &[filepath, oldInfo] : _watchedFiles) {
            FileInfo const newInfo = updateFileInfo(filepath);

            bool changed = false;

            if (!oldInfo.exists && newInfo.exists) {
                changed = true;
            } else if (oldInfo.exists && newInfo.exists &&
                       newInfo.lastModified > oldInfo.lastModified) {
                changed = true;
            }

            if (changed) {
                changedFiles.push_back(filepath);
                oldInfo = newInfo;
            }
        }

        return changedFiles;
    }

    auto FileWatcher::clearAll() -> void
    {
        _watchedFiles.clear();
        fprintf(stdout, "[FileWatcher] Cleared all watches\n");
    }

    auto FileWatcher::getWatchCount() const -> size_t
    {
        return _watchedFiles.size();
    }

    auto FileWatcher::updateFileInfo(const std::string &filepath) -> FileInfo
    {
        if (!std::filesystem::exists(filepath)) {
            return {std::filesystem::file_time_type::min(), false};
        }

        auto lastWrite = std::filesystem::last_write_time(filepath);
        return {lastWrite, true};
    }

} // namespace GameEngine::Scripting
