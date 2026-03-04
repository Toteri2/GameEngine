/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** InputManager - Implementation
*/

#include "InputManager.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

namespace GameEngine
{

    InputManager::InputManager(EcsManager &ecsManager)
        : _ecsManager(&ecsManager), _deviceLayer(std::make_unique<SDLInputDevice>(ecsManager))
    {
        subscribeToEvents();
        std::cout << "[InputManager] Initialized" << std::endl;
    }

    void InputManager::update(float deltaTime, EcsManager &ecsManager)
    {
        (void) deltaTime;

        _ecsManager = &ecsManager;

        _deviceLayer->update(ecsManager);

        processCompositeAxes();

        _inputState.clearFrameFlags();
    }

    void InputManager::pushContext(const std::string &contextName, int priority)
    {
        _bindingManager.addContext(InputContext(contextName, priority));
        std::cout << "[InputManager] Pushed context '" << contextName << "' with priority "
                  << priority << std::endl;
    }

    void InputManager::popContext(const std::string &contextName)
    {
        _bindingManager.removeContext(contextName);
        std::cout << "[InputManager] Popped context '" << contextName << "'" << std::endl;
    }

    void InputManager::setContextActive(const std::string &contextName, bool active)
    {
        _bindingManager.setContextActive(contextName, active);
    }

    void InputManager::enableTextInput(bool enable)
    {
        _deviceLayer->enableTextInput(enable);
    }

    void InputManager::setAxisSmoothing(bool enable, float smoothingFactor)
    {
        _axisSmoothing = enable;
        _smoothingFactor = std::clamp(smoothingFactor, 0.0f, 1.0f);
    }

    void InputManager::subscribeToEvents()
    {
        if (_eventsSubscribed) {
            return;
        }

        _ecsManager->subscribeEvent<RecoverBindingsEvent>(
            [this](const RecoverBindingsEvent &event) { onRecoverBinding(event); });

        _ecsManager->subscribeEvent<LowLevelInputEvent>(
            [this](const LowLevelInputEvent &event) { onLowLevelInput(event); });

        _ecsManager->subscribeEvent<InputDeviceEvent>(
            [this](const InputDeviceEvent &event) { onDeviceConnection(event); });

        _ecsManager->subscribeEvent<ChangeBindingEvent>(
            [this](const ChangeBindingEvent &event) { onChangeBinding(event); });

        _ecsManager->subscribeEvent<CreateContextEvent>(
            [this](const CreateContextEvent &event) {
                pushContext(event.contextName, event.priority);
            });

        _eventsSubscribed = true;
        std::cout << "[InputManager] Subscribed to input events" << std::endl;
    }

    void InputManager::onRecoverBinding(const RecoverBindingsEvent &event)
    {
        std::cout << "[InputManager] Recovering default bindings for context '"
                  << event.contextName << "'" << std::endl;
        std::vector<InputBinding> bindings =
            _bindingManager.getBindingsForContext(event.contextName);

        SharedBindingsEvent recoverEvent(event.contextName, bindings);
        _ecsManager->publishEvent<SharedBindingsEvent>(recoverEvent);
    }

    void InputManager::onLowLevelInput(const LowLevelInputEvent &event)
    {
        processLowLevelEvent(event);
    }

    void InputManager::onDeviceConnection(const InputDeviceEvent &event)
    {
        if (event.connected) {
            std::cout << "[InputManager] Device connected: " << event.deviceName
                      << " (type=" << static_cast<int>(event.deviceType)
                      << ", id=" << event.deviceId << ")" << std::endl;
            if (event.deviceType == DeviceType::GAMEPAD) {
                std::cout << "[InputManager] Setting up gamepad bindings for device ID: "
                          << event.deviceId << std::endl;
            }
        } else {
            std::cout << "[InputManager] Device disconnected: " << event.deviceName
                      << " (type=" << static_cast<int>(event.deviceType)
                      << ", id=" << event.deviceId << ")" << std::endl;
        }
    }

    void InputManager::onChangeBinding(const ChangeBindingEvent &event)
    {
        if (event.changeBinding) {
            DeviceType devType;

            if (event.deviceType == 0)
                devType = DeviceType::KEYBOARD;
            else if (event.deviceType == 1)
                devType = DeviceType::MOUSE;
            else
                devType = DeviceType::GAMEPAD;

            _bindingManager.removeBinding(event.contextName, event.actionName, devType);

            if (event.bindType == 0) {

                InputBinding binding = _bindingManager.createActionBinding(devType, 0, event.code,
                    event.actionName, _deviceLayer->getKeyNameFromCode(event.code));
                _bindingManager.addBinding(event.contextName, binding);

            } else if (event.bindType == 1) {

                InputBinding binding = _bindingManager.createCompositeAxisBinding(devType, 0,
                    event.code, event.actionName, _deviceLayer->getKeyNameFromCode(event.code),
                    event.value);
                _bindingManager.addBinding(event.contextName, binding);
            }
            std::cout << "[InputManager] Changed binding to context '" << event.contextName
                      << "' for target '" << event.actionName << "'" << std::endl;
        }
    }

    void InputManager::processLowLevelEvent(const LowLevelInputEvent &event)
    {
        std::vector<InputBinding> bindings =
            _bindingManager.getBindingsForInput(event.deviceType, event.deviceId, event.code);

        if (bindings.empty()) {
            return;
        }

        switch (event.eventType) {
            case InputEventType::BUTTON_PRESSED:
            case InputEventType::BUTTON_RELEASED:
                processButtonInput(event, bindings);
                break;

            case InputEventType::AXIS_MOTION:
                processAxisInput(event, bindings);
                break;

            default:
                break;
        }
    }

    void InputManager::processButtonInput(
        const LowLevelInputEvent &event, const std::vector<InputBinding> &bindings)
    {
        for (const auto &binding : bindings) {

            if (binding.isAxis && binding.isComposite) {
                float contribution =
                    evaluateButtonBinding(binding, event) ? binding.compositeValue : 0.0f;

                const std::string &key = binding.targetName;
                if (contribution == 0.0f) {
                    auto it = _compositeAxes.find(key);
                    if (it != _compositeAxes.end()) {
                        it->second -= binding.compositeValue;
                        if (std::abs(it->second) < 0.001f) {
                            _compositeAxes.erase(it);
                        }
                    }
                } else {
                    _compositeAxes[key] += contribution;
                }

            } else if (!binding.isAxis) {
                bool pressed = evaluateButtonBinding(binding, event);
                updateActionState(binding.targetName, pressed);
                publishActionEvent(binding.targetName, pressed);
            }
        }
    }

    void InputManager::processAxisInput(
        const LowLevelInputEvent &event, const std::vector<InputBinding> &bindings)
    {
        for (const auto &binding : bindings) {
            if (binding.isAxis && !binding.isComposite) {
                float value = evaluateAxisBinding(binding, event);
                updateAxisState(binding.targetName, value);
                publishAxisEvent(binding.targetName, value);

            } else if (!binding.isAxis) {
                bool pressed = (std::abs(event.value) >= binding.threshold);
                updateActionState(binding.targetName, pressed);
                publishActionEvent(binding.targetName, pressed);
            }
        }
    }

    bool InputManager::evaluateButtonBinding(
        const InputBinding &binding, const LowLevelInputEvent &event)
    {
        (void) binding;
        return (event.eventType == InputEventType::BUTTON_PRESSED);
    }

    float InputManager::evaluateAxisBinding(
        const InputBinding &binding, const LowLevelInputEvent &event)
    {
        float value = event.value;

        if (std::abs(value) < binding.deadzone) {
            value = 0.0f;
        } else {
            float sign = (value >= 0.0f) ? 1.0f : -1.0f;
            value = sign * (std::abs(value) - binding.deadzone) / (1.0f - binding.deadzone);
        }

        value *= binding.sensitivity;

        if (binding.invert) {
            value = -value;
        }

        if (binding.positiveOnly && value < 0.0f) {
            value = 0.0f;
        }

        return std::clamp(value, -1.0f, 1.0f);
    }

    void InputManager::updateActionState(const std::string &actionName, bool pressed)
    {
        std::uint64_t timestamp = getCurrentTimestamp();
        _inputState.updateAction(actionName, pressed, timestamp, 0);
    }

    void InputManager::updateAxisState(const std::string &axisName, float value)
    {
        if (_axisSmoothing) {
            float currentValue = _inputState.getAxisValue(axisName);
            value = currentValue + (value - currentValue) * _smoothingFactor;
        }

        std::uint64_t timestamp = getCurrentTimestamp();
        _inputState.updateAxis(axisName, value, timestamp, 0);
    }

    void InputManager::processCompositeAxes()
    {
        for (const auto &[axisName, value] : _compositeAxes) {
            float clampedValue = std::clamp(value, -1.0f, 1.0f);
            updateAxisState(axisName, clampedValue);
            publishAxisEvent(axisName, clampedValue);
        }

        _compositeAxes.clear();
    }

    std::uint64_t InputManager::getCurrentTimestamp() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count();
    }

    void InputManager::publishActionEvent(const std::string &actionName, bool pressed)
    {
        InputActionEvent event(actionName, pressed, 0);
        event.timestamp = getCurrentTimestamp();
        _ecsManager->publishEvent(event);
    }

    void InputManager::publishAxisEvent(const std::string &axisName, float value)
    {
        InputAxisEvent event(axisName, value, 0);
        event.timestamp = getCurrentTimestamp();
        _ecsManager->publishEvent(event);
    }
} // namespace GameEngine
