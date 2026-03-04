/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** InputBindingManager - Implementation
*/

#include <GameEngine/EventTypes.hpp>
#include "InputBindingManager.hpp"

#include <SDL3/SDL.h>
#include <algorithm>
#include <iostream>

namespace GameEngine
{

    void InputBindingManager::addContext(const InputContext &context)
    {
        removeContext(context.name);

        const auto insertPos = std::upper_bound(_contexts.begin(), _contexts.end(), context,
            [](const InputContext &a, const InputContext &b) { return a.priority > b.priority; });

        _contexts.insert(insertPos, context);

        std::cout << "[InputBindingManager] Added context '" << context.name << "' with priority "
                  << context.priority << std::endl;
    }

    void InputBindingManager::removeContext(const std::string &contextName)
    {
        auto it = std::find_if(_contexts.begin(), _contexts.end(),
            [&contextName](const InputContext &ctx) { return ctx.name == contextName; });

        if (it != _contexts.end()) {
            _contexts.erase(it);
            std::cout << "[InputBindingManager] Removed context '" << contextName << "'"
                      << std::endl;
        }
    }

    void InputBindingManager::setContextActive(const std::string &contextName, bool active)
    {
        int idx = findContextIndex(contextName);
        if (idx >= 0) {
            _contexts[idx].active = active;
            std::cout << "[InputBindingManager] Context '" << contextName << "' set to "
                      << (active ? "active" : "inactive") << std::endl;
        }
    }

    bool InputBindingManager::isContextActive(const std::string &contextName) const
    {
        int idx = findContextIndex(contextName);
        return idx >= 0 && _contexts[idx].active;
    }

    void InputBindingManager::addBinding(
        const std::string &contextName, const InputBinding &binding)
    {
        int idx = findContextIndex(contextName);
        if (idx < 0) {
            addContext(InputContext(contextName, 0));
            idx = findContextIndex(contextName);
        }

        if (idx >= 0) {
            _contexts[idx].bindings.push_back(binding);
            std::cout << "[InputBindingManager] Added binding in context '" << contextName
                      << "' for " << binding.targetName << std::endl;
        }
    }

    void InputBindingManager::removeBinding(
        const std::string &contextName, const std::string &targetName, DeviceType deviceType)
    {
        int idx = findContextIndex(contextName);
        if (idx < 0) {
            return;
        }

        auto &bindings = _contexts[idx].bindings;
        auto newEnd = std::remove_if(
            bindings.begin(), bindings.end(), [&targetName, deviceType](const InputBinding &binding) {
                return binding.targetName == targetName && binding.deviceType == deviceType;
            });

        bindings.erase(newEnd, bindings.end());
    }

    void InputBindingManager::clearBindings(const std::string &contextName)
    {
        int idx = findContextIndex(contextName);
        if (idx >= 0) {
            _contexts[idx].bindings.clear();
            std::cout << "[InputBindingManager] Cleared all bindings in context '" << contextName
                      << "'" << std::endl;
        }
    }

    std::vector<InputBinding> InputBindingManager::getBindingsForInput(
        DeviceType deviceType, std::uint32_t deviceId, std::uint32_t code) const
    {
        std::vector<InputBinding> matchingBindings;

        for (const auto &context : _contexts) {
            if (!context.active) {
                continue;
            }

            for (const auto &binding : context.bindings) {
                bool deviceMatches = (binding.deviceType == deviceType);
                bool deviceIdMatches = (binding.deviceId == 0 || binding.deviceId == deviceId);
                bool codeMatches = (binding.code == code);

                if (deviceMatches && deviceIdMatches && codeMatches) {
                    matchingBindings.push_back(binding);
                }
            }
        }

        return matchingBindings;
    }

    std::vector<InputBinding> InputBindingManager::getBindingsForTarget(
        const std::string &targetName) const
    {
        std::vector<InputBinding> matchingBindings;

        for (const auto &context : _contexts) {
            if (!context.active) {
                continue;
            }

            for (const auto &binding : context.bindings) {
                if (binding.targetName == targetName) {
                    matchingBindings.push_back(binding);
                }
            }
        }

        return matchingBindings;
    }

    std::vector<InputBinding> InputBindingManager::getBindingsForContext(
        const std::string &contextName) const
    {
        std::vector<InputBinding> matchingBindings;

        int idx = findContextIndex(contextName);
        if (idx >= 0) {
            matchingBindings = _contexts[idx].bindings;
        }

        return matchingBindings;
    }

    int InputBindingManager::findContextIndex(const std::string &name) const
    {
        auto it = std::find_if(_contexts.begin(), _contexts.end(),
            [&name](const InputContext &ctx) { return ctx.name == name; });

        if (it == _contexts.end()) {
            return -1;
        }

        return static_cast<int>(std::distance(_contexts.begin(), it));
    }

    InputBinding InputBindingManager::createActionBinding(DeviceType deviceType,
        std::uint32_t deviceId, std::uint32_t code, const std::string &actionName, const std::string &key)
    {
        return InputBinding(deviceType, deviceId, code, actionName, key, false);
    }

    InputBinding InputBindingManager::createAxisBinding(DeviceType deviceType,
        std::uint32_t deviceId, std::uint32_t code, const std::string &axisName, const std::string &key, float deadzone,
        float sensitivity, bool invert)
    {
        InputBinding binding(deviceType, deviceId, code, axisName, key, true);
        binding.deadzone = deadzone;
        binding.sensitivity = sensitivity;
        binding.invert = invert;
        return binding;
    }

    InputBinding InputBindingManager::createCompositeAxisBinding(DeviceType deviceType,
        std::uint32_t deviceId, std::uint32_t code, const std::string &axisName, const std::string &key, float value,
        float threshold)
    {
        InputBinding binding(deviceType, deviceId, code, axisName, key, true);
        binding.isComposite = true;
        binding.compositeValue = value;
        binding.threshold = threshold;
        return binding;
    }

    std::string InputBindingManager::getKeyNameFromCode(std::uint32_t code) const
    {
        SDL_Scancode scancode = static_cast<SDL_Scancode>(code);
        SDL_Keycode keycode = SDL_GetKeyFromScancode(scancode, SDL_KMOD_NONE, false);
        
        const char *name = SDL_GetKeyName(keycode);
        if (name && name[0] != '\0') {
            return std::string(name);
        }
        return "Unknown";
    }

} // namespace GameEngine
