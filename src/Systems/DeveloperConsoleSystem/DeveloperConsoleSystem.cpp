/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** DeveloperConsoleSystem implementation
*/

#include "DeveloperConsoleSystem.hpp"
#include <GameEngine/ConsoleComponents.hpp>
#include <GameEngine/EventTypes.hpp>
#include <GameEngine/RenderComponents.hpp>
#include <GameEngine/RenderEventTypes.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>

namespace GameEngine
{
    DeveloperConsoleSystem::DeveloperConsoleSystem(EcsManager &ecsManager)
        : _ecsManager(ecsManager)
    {
        ecsManager.registerComponent<DeveloperConsoleComponent>();
        ecsManager.registerComponent<ConsoleCommandComponent>();

        ecsManager.subscribeEvent<RegisterConsoleCommandEvent>(
            [this](const RegisterConsoleCommandEvent &event) { onRegisterCommand(event); });

        ecsManager.subscribeEvent<ToggleConsoleEvent>(
            [this](const ToggleConsoleEvent &event) { onToggleConsole(event); });

        ecsManager.subscribeEvent<ExecuteConsoleCommandEvent>(
            [this](const ExecuteConsoleCommandEvent &event) { onExecuteCommand(event); });

        ecsManager.subscribeEvent<TextInputEvent>(
            [this](const TextInputEvent &event) { onTextInput(event); });

        ecsManager.subscribeEvent<InputActionEvent>(
            [this](const InputActionEvent &event) { onInputAction(event); });

        createConsoleUI();

        _commands["help"] = [this](const std::vector<std::string> &args, std::function<void(const std::string&)> output) {
            (void) args;
            output("Available commands:");
            for (const auto &pair : _commands) {
                output("  " + pair.first);
            }
        };

        std::cout << "[DeveloperConsoleSystem] Initialized" << std::endl;
    }

    void DeveloperConsoleSystem::update(EcsManager &world, float deltaTime)
    {
        if (_consoleEntity == 0) {
            return;
        }

        auto &console = world.getComponent<DeveloperConsoleComponent>(_consoleEntity);
        updateConsoleVisuals(deltaTime);
    }

    void DeveloperConsoleSystem::onRegisterCommand(const RegisterConsoleCommandEvent &event)
    {
        _commands[event.command] = event.function;
        std::cout << "[DeveloperConsoleSystem] Registered command: " << event.command << std::endl;
    }

    void DeveloperConsoleSystem::onToggleConsole(const ToggleConsoleEvent &event)
    {
        (void) event;
        toggleConsole();
    }

    void DeveloperConsoleSystem::onExecuteCommand(const ExecuteConsoleCommandEvent &event)
    {
        executeCommand(event.commandLine);
    }

    void DeveloperConsoleSystem::onTextInput(const TextInputEvent &event)
    {
        if (_consoleEntity == 0) {
            return;
        }

        auto &console = _ecsManager.getComponent<DeveloperConsoleComponent>(_consoleEntity);
        if (console.isVisible && console.isActive) {
            console.currentInput += event.text;
        }
    }

    void DeveloperConsoleSystem::onInputAction(const InputActionEvent &event)
    {
        if (_consoleEntity == 0) {
            return;
        }

        auto &console = _ecsManager.getComponent<DeveloperConsoleComponent>(_consoleEntity);

        if (event.actionName == "ToggleConsole" && event.pressed) {
            toggleConsole();
            return;
        }
        if (console.isVisible && console.isActive && event.pressed) {
            if (event.actionName == "Enter" || event.actionName == "Return") {
                if (!console.currentInput.empty()) {
                    console.commandHistory.push_back(console.currentInput);
                    if (console.commandHistory.size() > console.maxHistorySize) {
                        console.commandHistory.erase(console.commandHistory.begin());
                    }
                    console.historyIndex = console.commandHistory.size();

                    executeCommand(console.currentInput);
                    console.currentInput.clear();
                }
            } else if (event.actionName == "Backspace") {
                if (!console.currentInput.empty()) {
                    console.currentInput.pop_back();
                }
            }
        }
    }

    void DeveloperConsoleSystem::executeCommand(const std::string &commandLine)
    {
        if (_consoleEntity == 0) {
            return;
        }

        auto &console = _ecsManager.getComponent<DeveloperConsoleComponent>(_consoleEntity);

        addOutput("> " + commandLine);

        std::vector<std::string> tokens = parseCommand(commandLine);
        if (tokens.empty()) {
            return;
        }

        std::string command = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        auto it = _commands.find(command);
        if (it != _commands.end()) {
            try {
                auto outputCallback = [this](const std::string &msg) { addOutput(msg); };
                it->second(args, outputCallback);
            } catch (const std::exception &e) {
                addOutput("Error executing command: " + std::string(e.what()));
            }
        } else {
            addOutput("Unknown command: " + command);
            addOutput("Available commands:");
            for (const auto &pair : _commands) {
                addOutput("  " + pair.first);
            }
        }
    }

    std::vector<std::string> DeveloperConsoleSystem::parseCommand(const std::string &commandLine)
    {
        std::vector<std::string> tokens;
        std::istringstream iss(commandLine);
        std::string token;

        while (iss >> token) {
            tokens.push_back(token);
        }

        return tokens;
    }

    void DeveloperConsoleSystem::addOutput(const std::string &output)
    {
        if (_consoleEntity == 0) {
            return;
        }

        auto &console = _ecsManager.getComponent<DeveloperConsoleComponent>(_consoleEntity);
        console.outputHistory.push_back(output);

        if (console.outputHistory.size() > console.maxOutputLines) {
            console.outputHistory.erase(console.outputHistory.begin());
        }
    }

    void DeveloperConsoleSystem::createConsoleUI()
    {
        _consoleEntity = _ecsManager.createEntity();
        DeveloperConsoleComponent consoleComp;
        consoleComp.isVisible = false;
        consoleComp.isActive = false;
        _ecsManager.addComponent(_consoleEntity, consoleComp);

        TransformComponent transform;
        transform.x = 0;
        transform.y = 0;
        transform.scaleX = 1280;
        transform.scaleY = 300;
        _ecsManager.addComponent(_consoleEntity, transform);

        RectangleComponent background;
        background.width = 1280;
        background.height = 300;
        background.colorR = 0;
        background.colorG = 0;
        background.colorB = 0;
        background.alpha = 0;
        background.filled = true;
        background.layer = 100;
        _ecsManager.addComponent(_consoleEntity, background);

        _inputTextEntity = _ecsManager.createEntity();
        TransformComponent inputTransform;
        inputTransform.x = 10;
        inputTransform.y = 260;
        _ecsManager.addComponent(_inputTextEntity, inputTransform);

        TextComponent inputText;
        inputText.fontId = "ui_font_24";
        inputText.text = "> ";
        inputText.colorR = 255;
        inputText.colorG = 255;
        inputText.colorB = 255;
        inputText.alpha = 0;
        inputText.layer = 1000;
        inputText.needsUpdate = true;
        _ecsManager.addComponent(_inputTextEntity, inputText);

        const size_t maxOutputLines = 8;
        _outputTextEntities.clear();
        _outputTextEntities.reserve(maxOutputLines);

        for (size_t i = 0; i < maxOutputLines; ++i) {
            EntityId outputEntity = _ecsManager.createEntity();
            TransformComponent outputTransform;
            outputTransform.x = 10;
            outputTransform.y = 10 + (i * 25);
            _ecsManager.addComponent(outputEntity, outputTransform);

            TextComponent outputText;
            outputText.fontId = "ui_font_24";
            outputText.text = "";
            outputText.colorR = 255;
            outputText.colorG = 255;
            outputText.colorB = 255;
            outputText.alpha = 0;
            outputText.layer = 1000;
            outputText.needsUpdate = true;
            _ecsManager.addComponent(outputEntity, outputText);

            _outputTextEntities.push_back(outputEntity);
        }

        std::cout << "[DeveloperConsoleSystem] Console UI created" << std::endl;
    }

    void DeveloperConsoleSystem::updateConsoleVisuals(float deltaTime)
    {
        if (_consoleEntity == 0 || _inputTextEntity == 0 || _outputTextEntities.empty()) {
            return;
        }

        auto &console = _ecsManager.getComponent<DeveloperConsoleComponent>(_consoleEntity);

        if (console.isVisible) {
            console.cursorBlinkTimer += deltaTime;
            if (console.cursorBlinkTimer >= 0.5f) {
                console.showCursor = !console.showCursor;
                console.cursorBlinkTimer = 0.0f;
            }
        }

        if (_ecsManager.hasComponent<RectangleComponent>(_consoleEntity)) {
            auto &background = _ecsManager.getComponent<RectangleComponent>(_consoleEntity);
            background.alpha = console.isVisible ? 200 : 0;
        }

        if (_ecsManager.hasComponent<TextComponent>(_inputTextEntity)) {
            auto &inputText = _ecsManager.getComponent<TextComponent>(_inputTextEntity);
            inputText.alpha = console.isVisible ? 255 : 0;

            std::string displayText = "> " + console.currentInput;
            if (console.showCursor) {
                displayText += "_";
            }

            if (inputText.text != displayText) {
                inputText.text = displayText;
                inputText.needsUpdate = true;
            }
        }

        const size_t maxLines = _outputTextEntities.size();
        size_t startLine =
            console.outputHistory.size() > maxLines ? console.outputHistory.size() - maxLines : 0;

        for (size_t i = 0; i < _outputTextEntities.size(); ++i) {
            EntityId outputEntity = _outputTextEntities[i];
            if (_ecsManager.hasComponent<TextComponent>(outputEntity)) {
                auto &outputText = _ecsManager.getComponent<TextComponent>(outputEntity);
                outputText.alpha = console.isVisible ? 255 : 0;

                size_t historyIndex = startLine + i;
                std::string displayText;
                if (historyIndex < console.outputHistory.size()) {
                    displayText = console.outputHistory[historyIndex];
                } else {
                    displayText = "";
                }

                if (outputText.text != displayText) {
                    outputText.text = displayText;
                    outputText.needsUpdate = true;
                }
            }
        }
    }

    void DeveloperConsoleSystem::toggleConsole()
    {
        if (_consoleEntity == 0) {
            return;
        }

        auto &console = _ecsManager.getComponent<DeveloperConsoleComponent>(_consoleEntity);
        console.isVisible = !console.isVisible;
        console.isActive = console.isVisible;

        if (console.isVisible) {
            console.currentInput.clear();
            if (!console.firstTimeOpened) {
                addOutput("Developer Console");
                addOutput("Type 'help' for available commands");
                console.firstTimeOpened = true;
            }
        } else {
            if (_ecsManager.hasComponent<RectangleComponent>(_consoleEntity)) {
                auto &background = _ecsManager.getComponent<RectangleComponent>(_consoleEntity);
                background.alpha = 0;
            }
        }

        std::cout << "[DeveloperConsoleSystem] Console "
                  << (console.isVisible ? "opened" : "closed") << std::endl;
    }
} // namespace GameEngine
