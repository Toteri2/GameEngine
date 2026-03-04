/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** SDLInputDevice - Low-level SDL input device layer
*/

#pragma once

#include <unordered_map>
#include <GameEngine/EcsManager.hpp>
#include <GameEngine/EventTypes.hpp>
#include <GameEngine/InputEventTypes.hpp>
#include <SDL3/SDL.h>
#include <string>
#include <utility>
#include <vector>

namespace GameEngine
{

    /**
     * @class SDLInputDevice
     * @brief Low-level SDL input device abstraction layer
     *
     * SDLInputDevice is the lowest layer of the input system. It:
     * - Polls SDL events via SDL_PollEvent()
     * - Normalizes SDL events into engine-agnostic LowLevelInputEvent
     * - Tracks device state (keyboard keys, mouse buttons, gamepad axes)
     * - Manages gamepad connection/disconnection
     * - Converts mouse coordinates from window space to render space
     * - Handles text input for UI text fields
     *
     * This class isolates all SDL-specific code from the rest of the engine.
     *
     * @note Only one instance should exist per application
     */
    class SDLInputDevice {
      public:
        /**
         * @brief Construct the SDL input device layer
         * @param ecsManager Reference to ECS manager for event publishing
         *
         * Initializes SDL gamepad and video subsystems if needed,
         * and performs initial gamepad scan.
         */
        explicit SDLInputDevice(EcsManager &ecsManager);
        ~SDLInputDevice() = default;

        /**
         * @brief Update the device layer (polls SDL events)
         * @param ecsManager Reference to ECS manager for event publishing
         *
         * Should be called every frame. This method:
         * 1. Polls all SDL events via SDL_PollEvent()
         * 2. Converts them to low-level engine events
         * 3. Publishes events to the ECS
         * 4. Periodically scans for new gamepads (every 1 second)
         */
        void update(EcsManager &ecsManager);

        /**
         * @brief Enable or disable SDL text input mode
         * @param enable true to enable text input, false to disable
         *
         * When enabled, SDL generates text input events for character typing.
         * Use this for text fields, chat boxes, etc.
         */
        void enableTextInput(bool enable);

        /**
         * @brief Manually scan for connected gamepads
         * @param ecsManager Reference to ECS manager for connection events
         *
         * Checks all joystick devices and connects any that are gamepads.
         * This is called automatically by update() every second.
         */
        void scanForGamepads(EcsManager &ecsManager);

        /**
         * @brief Check if a specific gamepad is connected
         * @param gamepadId ID of the gamepad to check
         * @return true if the gamepad is connected and valid
         */
        bool isGamepadConnected(std::uint32_t gamepadId) const;

        /**
         * @brief Get the name of a connected gamepad
         * @param gamepadId ID of the gamepad
         * @return Name of the gamepad, or empty string if not connected
         */
        std::string getGamepadName(std::uint32_t gamepadId) const;

        /**
         * @brief Get IDs of all connected gamepads
         * @return Vector of gamepad device IDs
         */
        std::vector<std::uint32_t> getConnectedGamepadIds() const;

        /**
         * @brief Convert an input code to a human-readable key name
         * @param code Scancode, button ID, or axis ID
         * @return Display name for the key/button (e.g., "W", "Space", "Left Mouse")
         *
         * Used for displaying key bindings in UI/settings menus.
         */
        std::string getKeyNameFromCode(std::uint32_t code) const;

      private:
        EcsManager &_ecsManager; ///< Reference to ECS manager for event publishing

        std::unordered_map<std::uint32_t, SDL_Gamepad *>
            _connectedGamepads; ///< Map of gamepad ID to SDL_Gamepad pointer

        std::unordered_map<std::uint32_t, bool>
            _keyStates; ///< Current state of keyboard keys (scancode -> pressed)
        std::unordered_map<std::uint32_t, bool>
            _mouseButtonStates; ///< Current state of mouse buttons (button ID -> pressed)
        std::unordered_map<std::uint64_t, bool>
            _gamepadButtonStates; ///< Current state of gamepad buttons (composite key -> pressed)
        std::unordered_map<std::uint64_t, float>
            _gamepadAxisStates; ///< Current state of gamepad axes (composite key -> value)

        bool _textInputEnabled = false;       ///< Whether SDL text input mode is active
        std::uint64_t _lastGamepadScan = 0;   ///< Timestamp of last gamepad scan (milliseconds)
        SDL_Window *_currentWindow = nullptr; ///< Current SDL window for text input
        std::unordered_map<std::uint32_t, std::uint8_t> _joystickHatStates;

        /**
         * @brief Process a single SDL event and dispatch to appropriate handler
         * @param event The SDL event to process
         * @param ecsManager Reference to ECS manager
         */
        void processSDLEvent(const SDL_Event &event, EcsManager &ecsManager);

        /**
         * @brief Process keyboard events (key press/release)
         * @param event The SDL keyboard event
         * @param ecsManager Reference to ECS manager
         *
         * Converts SDL_EVENT_KEY_DOWN/UP to LowLevelInputEvent with scancodes.
         * Ignores key repeat events.
         */
        void processKeyboardEvent(const SDL_Event &event, EcsManager &ecsManager);

        /**
         * @brief Process mouse events (button, motion, wheel)
         * @param event The SDL mouse event
         * @param ecsManager Reference to ECS manager
         *
         * Handles:
         * - Mouse button press/release
         * - Mouse motion (converts window coords to render coords)
         * - Mouse wheel scrolling
         */
        void processMouseEvent(const SDL_Event &event, EcsManager &ecsManager);

        /**
         * @brief Process text input events
         * @param event The SDL text input event
         * @param ecsManager Reference to ECS manager
         *
         * Handles SDL_EVENT_TEXT_INPUT for text entry in consoles or UI fields.
         */
        void processTextInputEvent(const SDL_Event &event, EcsManager &ecsManager);

        /**
         * @brief Process gamepad input events (button, axis)
         * @param event The SDL gamepad event
         * @param ecsManager Reference to ECS manager
         *
         * Handles button presses and analog stick/trigger movements.
         * Applies normalization and deadzone filtering.
         */
        void processGamepadEvent(const SDL_Event &event, EcsManager &ecsManager);

        /**
         * @brief Process gamepad device events (connection/disconnection)
         * @param event The SDL device event
         * @param ecsManager Reference to ECS manager
         */
        void processGamepadDeviceEvent(const SDL_Event &event, EcsManager &ecsManager);

        /**
         * @brief Process joystick HAT/DPAD motion events
         * @param event The SDL joystick HAT event
         * @param ecsManager Reference to ECS manager
         *
         * Converts HAT motion to DPAD button presses for compatibility with
         * controllers that don't expose DPAD as standard gamepad buttons.
         */
        void processJoystickHatEvent(const SDL_Event &event, EcsManager &ecsManager);

        /**
         * @brief Publish a low-level input event to the ECS
         * @param deviceType Type of device (KEYBOARD, MOUSE, GAMEPAD)
         * @param deviceId ID of the device
         * @param eventType Type of event (BUTTON_PRESSED, AXIS_MOTION, etc.)
         * @param code Input code (scancode, button ID, axis ID)
         * @param value Input value (1.0 for press, 0.0 for release, -1.0 to 1.0 for axis)
         * @param ecsManager Reference to ECS manager
         */
        void publishLowLevelEvent(DeviceType deviceType, std::uint32_t deviceId,
            InputEventType eventType, std::uint32_t code, float value, EcsManager &ecsManager);

        /**
         * @brief Publish a keyboard touch event (for text input)
         * @param scancode The key scancode
         * @param textKey Display name of the key
         * @param eventType Type of event (BUTTON_PRESSED, BUTTON_RELEASED)
         * @param ecsManager Reference to ECS manager
         */
        void publishKeyboardTouchEvent(std::uint32_t scancode, const std::string &textKey,
            InputEventType eventType, EcsManager &ecsManager);

        /**
         * @brief Create a composite key for gamepad state tracking
         * @param gamepadId ID of the gamepad
         * @param code Button or axis code
         * @return 64-bit key combining gamepad ID and code
         *
         * Allows tracking multiple gamepads independently.
         */
        std::uint64_t makeGamepadKey(std::uint32_t gamepadId, std::uint32_t code) const;

        /**
         * @brief Connect a gamepad and publish connection event
         * @param gamepadId ID of the gamepad to connect
         * @param ecsManager Reference to ECS manager
         */
        void connectGamepad(std::uint32_t gamepadId, EcsManager &ecsManager);

        /**
         * @brief Disconnect a gamepad and clean up state
         * @param gamepadId ID of the gamepad to disconnect
         * @param ecsManager Reference to ECS manager
         */
        void disconnectGamepad(std::uint32_t gamepadId, EcsManager &ecsManager);

        /**
         * @brief Normalize gamepad analog stick value
         * @param rawValue Raw SDL axis value (-32768 to 32767)
         * @return Normalized value (-1.0 to 1.0)
         */
        float normalizeGamepadAxis(std::int16_t rawValue) const;

        /**
         * @brief Normalize gamepad trigger value
         * @param rawValue Raw SDL trigger value (0 to 32767)
         * @return Normalized value (0.0 to 1.0)
         *
         * Triggers are positive-only axes.
         */
        float normalizeGamepadTrigger(std::int16_t rawValue) const;
    };

} // namespace GameEngine
