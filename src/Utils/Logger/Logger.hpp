/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** Logger - Simple logging subsystem
*/

#pragma once

#include <string>

namespace GameEngine
{
    namespace Logger
    {
        enum class LogLevel { INFO, WARNING, ERROR_LEVEL };

        class LoggerManager {
          public:
            LoggerManager();
            ~LoggerManager() = default;

            auto log(LogLevel level, const std::string &message) -> void;
            auto info(const std::string &message) -> void;
            auto warning(const std::string &message) -> void;
            auto error(const std::string &message) -> void;

          private:
            auto getLevelString(LogLevel level) -> std::string;
        };

        void info(const std::string &message);
        void warning(const std::string &message);
        void error(const std::string &message);
        void debug(const std::string &message);
    } // namespace Logger
} // namespace GameEngine
