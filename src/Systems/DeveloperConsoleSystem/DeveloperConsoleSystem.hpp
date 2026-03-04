/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** DeveloperConsoleSystem
*/

#pragma once

#include <GameEngine/EcsManager.hpp>
#include <GameEngine/ISystem.hpp>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace GameEngine
{
    class DeveloperConsoleSystem final : public ISystem {
      public:
        explicit DeveloperConsoleSystem(EcsManager &ecsManager);
        ~DeveloperConsoleSystem() override = default;

        void update(EcsManager &world, float deltaTime) override;

      private:
        EcsManager &_ecsManager;
        EntityId _consoleEntity = 0;
        EntityId _inputTextEntity = 0;
        std::vector<EntityId> _outputTextEntities;
        std::map<std::string, std::function<void(const std::vector<std::string> &, std::function<void(const std::string&)>)>> _commands;

        // Event handlers
        void onRegisterCommand(const struct RegisterConsoleCommandEvent &event);
        void onToggleConsole(const struct ToggleConsoleEvent &event);
        void onExecuteCommand(const struct ExecuteConsoleCommandEvent &event);
        void onTextInput(const struct TextInputEvent &event);
        void onInputAction(const struct InputActionEvent &event);

        // Console logic
        void executeCommand(const std::string &commandLine);
        std::vector<std::string> parseCommand(const std::string &commandLine);
        void addOutput(const std::string &output);
        void createConsoleUI();
        void updateConsoleVisuals(float deltaTime);
        void toggleConsole();
    };
} // namespace GameEngine
