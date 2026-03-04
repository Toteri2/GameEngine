/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** ConsoleComponents
*/

#pragma once

#include <functional>
#include <string>
#include <vector>

namespace GameEngine
{
    struct DeveloperConsoleComponent {
        bool isVisible = false;
        bool isActive = false;
        std::string currentInput;
        std::vector<std::string> commandHistory;
        std::vector<std::string> outputHistory;
        std::size_t historyIndex = 0;
        std::size_t maxHistorySize = 50;
        std::size_t maxOutputLines = 20;
        float cursorBlinkTimer = 0.0f;
        bool showCursor = true;
        bool ignoreNextToggleChar = false;
        bool firstTimeOpened = false;
    };

    struct ConsoleCommandComponent {
        std::string command;
        std::function<void(const std::vector<std::string> &)> function;
    };
} // namespace GameEngine
