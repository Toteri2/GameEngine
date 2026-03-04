/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** SDLInputDevice - Implementation
*/

#include "SDLInputDevice.hpp"
#include <GameEngine/EventTypes.hpp>
#include <chrono>
#include <iostream>

namespace GameEngine
{

    SDLInputDevice::SDLInputDevice(EcsManager &ecsManager) : _ecsManager(ecsManager)
    {
        if (!SDL_WasInit(SDL_INIT_GAMEPAD)) {
            if (!SDL_Init(SDL_INIT_GAMEPAD)) {
                std::cerr << "[SDLInputDevice] Failed to init SDL gamepad: " << SDL_GetError()
                          << std::endl;
            }
        }

        if (!SDL_WasInit(SDL_INIT_VIDEO)) {
            if (!SDL_Init(SDL_INIT_VIDEO)) {
                std::cerr << "[SDLInputDevice] Failed to init SDL video: " << SDL_GetError()
                          << std::endl;
            }
        }

        scanForGamepads(ecsManager);

        std::cout << "[SDLInputDevice] Initialized" << std::endl;
    }

    void SDLInputDevice::enableTextInput(bool enable)
    {
        _textInputEnabled = enable;
        if (enable && _currentWindow) {
            SDL_StartTextInput(_currentWindow);
        } else if (!enable && _currentWindow) {
            SDL_StopTextInput(_currentWindow);
        }
    }

    void SDLInputDevice::update(EcsManager &ecsManager)
    {
        std::uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
                                        .count();

        if (currentTime - _lastGamepadScan > 1000) {
            scanForGamepads(ecsManager);
            _lastGamepadScan = currentTime;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            processSDLEvent(event, ecsManager);
        }
    }

    void SDLInputDevice::processSDLEvent(const SDL_Event &event, EcsManager &ecsManager)
    {
        SDL_Window *window = SDL_GetWindowFromEvent(&event);
        if (window && !_currentWindow) {
            _currentWindow = window;
        }
        if (!_textInputEnabled && _currentWindow) {
            SDL_StartTextInput(_currentWindow);
            _textInputEnabled = true;
        }

        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                processKeyboardEvent(event, ecsManager);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            case SDL_EVENT_MOUSE_MOTION:
            case SDL_EVENT_MOUSE_WHEEL:
                processMouseEvent(event, ecsManager);
                break;

            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP:
            case SDL_EVENT_GAMEPAD_AXIS_MOTION:
                processGamepadEvent(event, ecsManager);
                break;

            case SDL_EVENT_GAMEPAD_ADDED:
            case SDL_EVENT_GAMEPAD_REMOVED:
                processGamepadDeviceEvent(event, ecsManager);
                break;

            case SDL_EVENT_TEXT_INPUT:
                processTextInputEvent(event, ecsManager);
                break;
            case SDL_EVENT_JOYSTICK_HAT_MOTION:
                processJoystickHatEvent(event, ecsManager);
                break;

            case SDL_EVENT_QUIT:
                _ecsManager.publishEvent(ExitEvent());
                break;

            default:
                break;
        }
    }

    void SDLInputDevice::processKeyboardEvent(const SDL_Event &event, EcsManager &ecsManager)
    {
        if (event.key.repeat) {
            return;
        }

        std::uint32_t scancode = static_cast<std::uint32_t>(event.key.scancode);
        bool pressed = (event.type == SDL_EVENT_KEY_DOWN);

        bool wasPressed = _keyStates[scancode];
        _keyStates[scancode] = pressed;

        if (pressed != wasPressed) {
            InputEventType eventType =
                pressed ? InputEventType::BUTTON_PRESSED : InputEventType::BUTTON_RELEASED;
            publishLowLevelEvent(
                DeviceType::KEYBOARD, 0, eventType, scancode, pressed ? 1.0f : 0.0f, ecsManager);
            publishKeyboardTouchEvent(scancode,
                pressed ? getKeyNameFromCode(static_cast<SDL_Scancode>(scancode))
                        : std::string(""),
                eventType, ecsManager);
        }
    }

    std::string SDLInputDevice::getKeyNameFromCode(std::uint32_t code) const
    {
        SDL_Scancode scancode = static_cast<SDL_Scancode>(code);
        SDL_Keycode keycode = SDL_GetKeyFromScancode(scancode, SDL_KMOD_NONE, false);

        const char *name = SDL_GetKeyName(keycode);
        if (name && name[0] != '\0') {
            return std::string(name);
        }
        return "Unknown";
    }

    void SDLInputDevice::processTextInputEvent(const SDL_Event &event, EcsManager &ecsManager)
    {
        if (event.type != SDL_EVENT_TEXT_INPUT) {
            return;
        }

        std::uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
                                        .count();

        std::string inputText(event.text.text);
        if (!inputText.empty()) {
            TextInputEvent textEvent(inputText, currentTime);
            ecsManager.publishEvent(textEvent);
        }
    }

    void SDLInputDevice::processMouseEvent(const SDL_Event &event, EcsManager &ecsManager)
    {
        switch (event.type) {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                std::uint32_t button = static_cast<std::uint32_t>(event.button.button);
                bool pressed = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);

                bool wasPressed = _mouseButtonStates[button];
                _mouseButtonStates[button] = pressed;

                if (pressed != wasPressed) {
                    InputEventType eventType =
                        pressed ? InputEventType::BUTTON_PRESSED : InputEventType::BUTTON_RELEASED;
                    publishLowLevelEvent(DeviceType::MOUSE, 0, eventType, button,
                        pressed ? 1.0f : 0.0f, ecsManager);
                }
                break;
            }

            case SDL_EVENT_MOUSE_MOTION: {
                float renderX = event.motion.x;
                float renderY = event.motion.y;

                SDL_Window *window = SDL_GetWindowFromEvent(&event);
                if (window) {
                    SDL_Renderer *renderer = SDL_GetRenderer(window);
                    if (renderer) {
                        SDL_RenderCoordinatesFromWindow(
                            renderer, event.motion.x, event.motion.y, &renderX, &renderY);
                    }
                }

                MouseMotionEvent motionEvent(
                    renderX, renderY, event.motion.xrel, event.motion.yrel);
                ecsManager.publishEvent(motionEvent);

                publishLowLevelEvent(
                    DeviceType::MOUSE, 0, InputEventType::AXIS_MOTION, 1000, renderX, ecsManager);
                publishLowLevelEvent(
                    DeviceType::MOUSE, 0, InputEventType::AXIS_MOTION, 1001, renderY, ecsManager);
                publishLowLevelEvent(DeviceType::MOUSE, 0, InputEventType::AXIS_MOTION, 1002,
                    event.motion.xrel, ecsManager);
                publishLowLevelEvent(DeviceType::MOUSE, 0, InputEventType::AXIS_MOTION, 1003,
                    event.motion.yrel, ecsManager);
                break;
            }

            case SDL_EVENT_MOUSE_WHEEL: {
                publishLowLevelEvent(DeviceType::MOUSE, 0, InputEventType::AXIS_MOTION, 1004,
                    event.wheel.x, ecsManager);
                publishLowLevelEvent(DeviceType::MOUSE, 0, InputEventType::AXIS_MOTION, 1005,
                    event.wheel.y, ecsManager);
                break;
            }
        }
    }

    void SDLInputDevice::processGamepadEvent(const SDL_Event &event, EcsManager &ecsManager)
    {
        std::uint32_t gamepadId = static_cast<std::uint32_t>(event.gdevice.which);

        auto it = _connectedGamepads.find(gamepadId);
        if (it == _connectedGamepads.end() || !it->second) {
            return;
        }

        switch (event.type) {
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            case SDL_EVENT_GAMEPAD_BUTTON_UP: {
                std::uint32_t button = static_cast<std::uint32_t>(event.gbutton.button);
                bool pressed = (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);

                std::uint64_t key = makeGamepadKey(gamepadId, button);
                bool wasPressed = _gamepadButtonStates[key];
                _gamepadButtonStates[key] = pressed;

                if (pressed != wasPressed) {
                    InputEventType eventType =
                        pressed ? InputEventType::BUTTON_PRESSED : InputEventType::BUTTON_RELEASED;
                    publishLowLevelEvent(DeviceType::GAMEPAD, gamepadId, eventType, button,
                        pressed ? 1.0f : 0.0f, ecsManager);
                }
                break;
            }

            case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
                std::uint32_t axis = static_cast<std::uint32_t>(event.gaxis.axis);
                float value;

                if (axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER
                    || axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
                    value = normalizeGamepadTrigger(event.gaxis.value);
                } else {
                    value = normalizeGamepadAxis(event.gaxis.value);
                }

                std::uint64_t key = makeGamepadKey(gamepadId, axis);
                float oldValue = _gamepadAxisStates[key];
                _gamepadAxisStates[key] = value;

                if (std::abs(value - oldValue) > 0.01f) {
                    publishLowLevelEvent(DeviceType::GAMEPAD, gamepadId,
                        InputEventType::AXIS_MOTION, axis, value, ecsManager);
                }
                break;
            }
        }
    }

    void SDLInputDevice::processGamepadDeviceEvent(const SDL_Event &event, EcsManager &ecsManager)
    {
        std::uint32_t gamepadId = static_cast<std::uint32_t>(event.gdevice.which);

        switch (event.type) {
            case SDL_EVENT_GAMEPAD_ADDED:
                connectGamepad(gamepadId, ecsManager);
                break;

            case SDL_EVENT_GAMEPAD_REMOVED:
                disconnectGamepad(gamepadId, ecsManager);
                break;
        }
    }

    void SDLInputDevice::processJoystickHatEvent(const SDL_Event &event, EcsManager &ecsManager)
    {
        std::uint32_t joystickId = static_cast<std::uint32_t>(event.jhat.which);
        std::uint8_t hatValue = event.jhat.value;
        std::uint8_t oldHatValue = _joystickHatStates[joystickId];

        struct HatDirection {
            std::uint8_t mask;
            std::uint32_t buttonCode;
        };

        HatDirection directions[] = {{0x01, SDL_GAMEPAD_BUTTON_DPAD_UP},
            {0x02, SDL_GAMEPAD_BUTTON_DPAD_RIGHT}, {0x04, SDL_GAMEPAD_BUTTON_DPAD_DOWN},
            {0x08, SDL_GAMEPAD_BUTTON_DPAD_LEFT}};

        for (const auto &dir : directions) {
            bool wasPressed = (oldHatValue & dir.mask) != 0;
            bool isPressed = (hatValue & dir.mask) != 0;

            if (wasPressed != isPressed) {
                InputEventType eventType =
                    isPressed ? InputEventType::BUTTON_PRESSED : InputEventType::BUTTON_RELEASED;

                publishLowLevelEvent(DeviceType::GAMEPAD, joystickId, eventType, dir.buttonCode,
                    isPressed ? 1.0f : 0.0f, ecsManager);
            }
        }

        _joystickHatStates[joystickId] = hatValue;
    }

    void SDLInputDevice::scanForGamepads(EcsManager &ecsManager)
    {
        int numJoysticks = 0;
        SDL_JoystickID *joysticks = SDL_GetJoysticks(&numJoysticks);

        if (!joysticks) {
            return;
        }

        for (int i = 0; i < numJoysticks; ++i) {
            SDL_JoystickID joyId = joysticks[i];

            if (SDL_IsGamepad(joyId)) {
                std::uint32_t gamepadId = static_cast<std::uint32_t>(joyId);

                if (_connectedGamepads.find(gamepadId) == _connectedGamepads.end()) {
                    connectGamepad(gamepadId, ecsManager);
                }
            }
        }

        SDL_free(joysticks);
    }

    void SDLInputDevice::connectGamepad(std::uint32_t gamepadId, EcsManager &ecsManager)
    {
        if (_connectedGamepads.find(gamepadId) != _connectedGamepads.end()) {
            return;
        }

        SDL_Gamepad *gamepad = SDL_OpenGamepad(static_cast<SDL_JoystickID>(gamepadId));
        if (!gamepad) {
            std::cerr << "[SDLInputDevice] Failed to open gamepad " << gamepadId << ": "
                      << SDL_GetError() << std::endl;
            return;
        }

        _connectedGamepads[gamepadId] = gamepad;

        std::string name = SDL_GetGamepadName(gamepad);
        if (name.empty()) {
            name = "Unknown Gamepad";
        }

        ecsManager.publishEvent(InputDeviceEvent(DeviceType::GAMEPAD, gamepadId, true, name));

        std::cout << "[SDLInputDevice] Connected gamepad " << gamepadId << " (" << name << ")"
                  << std::endl;
    }

    void SDLInputDevice::disconnectGamepad(std::uint32_t gamepadId, EcsManager &ecsManager)
    {
        auto it = _connectedGamepads.find(gamepadId);
        if (it == _connectedGamepads.end()) {
            return;
        }

        std::string name = SDL_GetGamepadName(it->second);

        ecsManager.publishEvent(InputDeviceEvent(DeviceType::GAMEPAD, gamepadId, false, name));

        std::cout << "[SDLInputDevice] Disconnected gamepad " << gamepadId << " (" << name << ")"
                  << std::endl;

        SDL_CloseGamepad(it->second);
        _connectedGamepads.erase(it);

        for (auto stateIt = _gamepadButtonStates.begin(); stateIt != _gamepadButtonStates.end();) {
            if ((stateIt->first >> 32) == gamepadId) {
                stateIt = _gamepadButtonStates.erase(stateIt);
            } else {
                ++stateIt;
            }
        }

        for (auto stateIt = _gamepadAxisStates.begin(); stateIt != _gamepadAxisStates.end();) {
            if ((stateIt->first >> 32) == gamepadId) {
                stateIt = _gamepadAxisStates.erase(stateIt);
            } else {
                ++stateIt;
            }
        }
    }

    bool SDLInputDevice::isGamepadConnected(std::uint32_t gamepadId) const
    {
        auto it = _connectedGamepads.find(gamepadId);
        return it != _connectedGamepads.end() && it->second && SDL_GamepadConnected(it->second);
    }

    std::string SDLInputDevice::getGamepadName(std::uint32_t gamepadId) const
    {
        auto it = _connectedGamepads.find(gamepadId);
        if (it != _connectedGamepads.end() && it->second) {
            return SDL_GetGamepadName(it->second);
        }
        return "";
    }

    std::vector<std::uint32_t> SDLInputDevice::getConnectedGamepadIds() const
    {
        std::vector<std::uint32_t> ids;
        for (const auto &[id, gamepad] : _connectedGamepads) {
            if (gamepad && SDL_GamepadConnected(gamepad)) {
                ids.push_back(id);
            }
        }
        return ids;
    }

    void SDLInputDevice::publishLowLevelEvent(DeviceType deviceType, std::uint32_t deviceId,
        InputEventType eventType, std::uint32_t code, float value, EcsManager &ecsManager)
    {
        std::uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
                                      .count();

        LowLevelInputEvent event(deviceType, deviceId, eventType, code, value);
        ecsManager.publishEvent(
            LowLevelInputEvent(deviceType, deviceId, eventType, code, value, timestamp));
    }

    void SDLInputDevice::publishKeyboardTouchEvent(std::uint32_t scancode,
        const std::string &textKey, InputEventType eventType, EcsManager &ecsManager)
    {
        KeyboardTouchEvent event(scancode, textKey, eventType);
        ecsManager.publishEvent(event);
    }

    std::uint64_t SDLInputDevice::makeGamepadKey(std::uint32_t gamepadId, std::uint32_t code) const
    {
        return (static_cast<std::uint64_t>(gamepadId) << 32) | code;
    }

    float SDLInputDevice::normalizeGamepadAxis(std::int16_t rawValue) const
    {
        if (rawValue < 0) {
            return rawValue / 32768.0f;
        } else {
            return rawValue / 32767.0f;
        }
    }

    float SDLInputDevice::normalizeGamepadTrigger(std::int16_t rawValue) const
    {
        return std::max(0.0f, rawValue / 32767.0f);
    }

} // namespace GameEngine
