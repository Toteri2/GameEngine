/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** Logger implementation
*/

#include "Logger.hpp"
#include <iostream>

namespace GameEngine
{
    namespace Logger
    {
        LoggerManager::LoggerManager()
        {
            std::cout << "Logger created" << std::endl;
        }

        auto LoggerManager::log(const LogLevel level, const std::string &message) -> void
        {
            std::cout << "[" << getLevelString(level) << "] " << message << std::endl;
        }

        auto LoggerManager::info(const std::string &message) -> void
        {
            log(LogLevel::INFO, message);
        }

        auto LoggerManager::warning(const std::string &message) -> void
        {
            log(LogLevel::WARNING, message);
        }

        auto LoggerManager::error(const std::string &message) -> void
        {
            log(LogLevel::ERROR_LEVEL, message);
        }

        auto LoggerManager::getLevelString(const LogLevel level) -> std::string
        {
            switch (level) {
                case LogLevel::INFO:
                    return "INFO";
                case LogLevel::WARNING:
                    return "WARNING";
                case LogLevel::ERROR_LEVEL:
                    return "ERROR";
                default:
                    return "UNKNOWN";
            }
        }

        static LoggerManager& getInstance() {
            static LoggerManager instance;
            return instance;
        }

        void info(const std::string &message) {
            getInstance().info(message);
        }

        void warning(const std::string &message) {
            getInstance().warning(message);
        }

        void error(const std::string &message) {
            getInstance().error(message);
        }

        void debug(const std::string &message) {
            getInstance().info("[DEBUG] " + message);
        }
    } // namespace Logger
} // namespace GameEngine
