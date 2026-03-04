/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** InputManager - Main input management system
*/

#pragma once

#include <GameEngine/EcsManager.hpp>
#include <GameEngine/EventTypes.hpp>
#include "InputBindingManager.hpp"
#include "InputState.hpp"
#include "SDLInputDevice.hpp"
#include <memory>
#include <unordered_map>

namespace GameEngine
{

    /**
     * @class InputManager
     * @brief High-level input management system that coordinates all input processing
     * 
     * The InputManager is the central component of the input system. It:
     * - Polls low-level input events from SDL via SDLInputDevice
     * - Maps physical inputs to logical actions/axes using InputBindingManager
     * - Maintains current input state via InputState
     * - Publishes high-level input events (InputActionEvent, InputAxisEvent)
     * - Handles input contexts (gameplay, menus, etc.)
     * - Processes composite axes (virtual axes from button combinations)
     * - Manages device connections/disconnections
     * 
     * @note This class should be registered as a system in the ECS
     */
    class InputManager {
      public:

        explicit InputManager(EcsManager &ecsManager);
        ~InputManager() = default;

        /**
         * @brief Update the input system (call every frame)
         * @param deltaTime Time elapsed since last frame (unused currently)
         * @param ecsManager Reference to the ECS manager
         * 
         * This method:
         * 1. Polls SDL events via the device layer
         * 2. Processes input bindings
         * 3. Updates input state
         * 4. Publishes high-level input events
         */
        void update(float deltaTime, EcsManager &ecsManager);
        /**
         * @brief Setup default bindings for all connected devices
         * 
         * Automatically detects and configures:
         * - Keyboard and mouse bindings
         * - All connected gamepad bindings
         * 
         * This creates "Gameplay" and "UI" contexts with standard controls.
         */
        void setupDefaultBindings();

        /**
         * @brief Add a new input context to the stack
         * @param contextName Name of the context to push
         * @param priority Priority level (higher = processed first)
         * 
         * Contexts allow different sets of bindings to be active at different times.
         * For example, pushing a "Menu" context can override "Gameplay" bindings.
         */
        void pushContext(const std::string &contextName, int priority = 0);
        
        /**
         * @brief Remove an input context from the stack
         * @param contextName Name of the context to remove
         */
        void popContext(const std::string &contextName);
        
        /**
         * @brief Enable or disable a specific context
         * @param contextName Name of the context
         * @param active true to enable, false to disable
         */
        void setContextActive(const std::string &contextName, bool active);

        /**
         * @brief Enable or disable text input mode
         * @param enable true to enable text input, false to disable
         * 
         * When enabled, SDL will generate text input events for typing.
         * Useful for text fields, chat, etc.
         */
        void enableTextInput(bool enable);
        
        /**
         * @brief Configure axis smoothing
         * @param enable true to enable smoothing, false to disable
         * @param smoothingFactor Interpolation factor (0.0 to 1.0, default: 0.1)
         * 
         * Axis smoothing applies interpolation to analog inputs to reduce jitter
         * and make movement feel smoother. Lower values = more smoothing.
         */
        void setAxisSmoothing(bool enable, float smoothingFactor = 0.1f);

        /**
         * @brief Access the InputBindingManager for binding configuration
         * @return Reference to the InputBindingManager
         */
        InputBindingManager &getBindingManager()
        {
            return _bindingManager;
        }

      private:
        EcsManager *_ecsManager;                       ///< Pointer to ECS manager for event publishing

        std::unique_ptr<SDLInputDevice> _deviceLayer;  ///< Low-level SDL input device layer
        InputState _inputState;                        ///< Current state of all inputs
        InputBindingManager _bindingManager;           ///< Manages input bindings and contexts

        bool _eventsSubscribed = false;                ///< Whether event subscriptions are active
        std::unordered_map<std::string, float>
            _compositeAxes;                            ///< Accumulated composite axis values

        bool _axisSmoothing = false;                   ///< Whether axis smoothing is enabled
        float _smoothingFactor = 0.1f;                 ///< Interpolation factor for smoothing

        /**
         * @brief Subscribe to all relevant input events
         * 
         * Subscribes to:
         * - LowLevelInputEvent (from SDL device layer)
         * - InputDeviceEvent (device connections/disconnections)
         * - ChangeBindingEvent (runtime binding changes)
         * - RecoverBindingsEvent (binding queries)
         */
        void subscribeToEvents();
        
        /**
         * @brief Handle low-level input events from SDL
         * @param event The low-level input event
         */
        void onLowLevelInput(const LowLevelInputEvent &event);
        
        /**
         * @brief Handle device connection/disconnection events
         * @param event The device event
         * 
         * Automatically sets up default bindings when a new device connects.
         */
        void onDeviceConnection(const InputDeviceEvent &event);
        
        /**
         * @brief Handle runtime binding change requests
         * @param event The binding change event
         */
        void onChangeBinding(const ChangeBindingEvent &event);
        
        /**
         * @brief Handle binding recovery requests (for UI/settings)
         * @param event The recovery request event
         */
        void onRecoverBinding(const RecoverBindingsEvent &event);

        /**
         * @brief Process a low-level input event and apply bindings
         * @param event The event to process
         */
        void processLowLevelEvent(const LowLevelInputEvent &event);
        
        /**
         * @brief Process button input (pressed/released) with bindings
         * @param event The input event
         * @param bindings Vector of bindings that match this input
         */
        void processButtonInput(
            const LowLevelInputEvent &event, const std::vector<InputBinding> &bindings);
        
        /**
         * @brief Process axis input (analog movement) with bindings
         * @param event The input event
         * @param bindings Vector of bindings that match this input
         */
        void processAxisInput(
            const LowLevelInputEvent &event, const std::vector<InputBinding> &bindings);

        /**
         * @brief Evaluate a button binding to determine if it's pressed
         * @param binding The binding to evaluate
         * @param event The input event
         * @return true if the button is pressed
         */
        bool evaluateButtonBinding(const InputBinding &binding, const LowLevelInputEvent &event);
        
        /**
         * @brief Evaluate an axis binding and apply processing
         * @param binding The binding with deadzone, sensitivity, etc.
         * @param event The input event with raw axis value
         * @return Processed axis value after applying deadzone, sensitivity, inversion
         */
        float evaluateAxisBinding(const InputBinding &binding, const LowLevelInputEvent &event);

        /**
         * @brief Update the state of an action
         * @param actionName Name of the action
         * @param pressed Whether the action is pressed
         */
        void updateActionState(const std::string &actionName, bool pressed);
        
        /**
         * @brief Update the state of an axis
         * @param axisName Name of the axis
         * @param value Axis value (-1.0 to 1.0)
         */
        void updateAxisState(const std::string &axisName, float value);
        
        /**
         * @brief Process accumulated composite axis values
         * 
         * Composite axes are virtual axes created from button combinations
         * (e.g., W+S keys creating a vertical movement axis)
         */
        void processCompositeAxes();

        /**
         * @brief Get current timestamp in milliseconds
         * @return Current time since epoch in milliseconds
         */
        std::uint64_t getCurrentTimestamp() const;
        
        /**
         * @brief Publish a high-level action event
         * @param actionName Name of the action
         * @param pressed Whether the action is pressed
         */
        void publishActionEvent(const std::string &actionName, bool pressed);
        
        /**
         * @brief Publish a high-level axis event
         * @param axisName Name of the axis
         * @param value Axis value (-1.0 to 1.0)
         */
        void publishAxisEvent(const std::string &axisName, float value);
    };

} // namespace GameEngine
