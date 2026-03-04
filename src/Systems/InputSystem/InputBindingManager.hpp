/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** InputBinding - Binding configuration and management
*/

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace GameEngine
{

    /**
     * @struct InputBinding
     * @brief Represents a single input binding that maps a physical input to a game action or axis
     * 
     * An InputBinding connects a physical input (key, button, axis) from a specific device
     * to a logical game action (like "Jump") or axis (like "MoveX"). It includes configuration
     * for how the input should be processed (deadzone, sensitivity, etc.).
     */
    struct InputBinding {
        DeviceType deviceType;      ///< Type of input device (KEYBOARD, MOUSE, GAMEPAD, TOUCH)
        std::uint32_t deviceId;     ///< Device identifier (0 = any device of this type)
        std::uint32_t code;         ///< Input code (scancode for keyboard, button ID, axis ID)
        std::string targetName;     ///< Name of the action or axis this binding targets
        bool isAxis;                ///< true = axis binding, false = action binding
        std::string keyName;        ///< Display name of the key/button for UI

        float deadzone = 0.1f;      ///< Minimum absolute value to register (ignores small movements)
        float sensitivity = 1.0f;   ///< Multiplier applied to axis values
        bool invert = false;        ///< If true, inverts the axis direction (positive becomes negative)
        bool positiveOnly = false;  ///< If true, only positive values are used (useful for triggers)
        float threshold = 0.5f;     ///< Threshold for converting axis input to action (button press)

        bool isComposite = false;    ///< If true, this button contributes to a composite axis
        float compositeValue = 1.0f; ///< Value to contribute to the composite axis (-1.0 or 1.0)

        InputBinding() = default;

        /**
         * @brief Construct an InputBinding
         * @param devType Type of device (KEYBOARD, MOUSE, GAMEPAD)
         * @param devId Device ID (0 for any device of this type)
         * @param inputCode Scancode, button ID, or axis ID
         * @param target Name of the action or axis to bind to
         * @param key Display name for UI
         * @param axisBinding true if this is an axis binding, false for action
         */
        InputBinding(DeviceType devType, std::uint32_t devId, std::uint32_t inputCode,
            const std::string &target, const std::string &key, bool axisBinding = false)
            : deviceType(devType), deviceId(devId), code(inputCode), targetName(target),
              keyName(key), isAxis(axisBinding)
        {}
    };

    /**
     * @struct InputContext
     * @brief Represents a collection of input bindings that can be enabled/disabled together
     * 
     * Input contexts allow you to have different sets of bindings active at different times.
     * For example, you might have a "Gameplay" context for in-game controls and a "Menu" context
     * for menu navigation. Contexts are stackable and processed in priority order.
     */
    struct InputContext {
        std::string name;                   ///< Unique name of the context
        bool active = true;                 ///< Whether this context is currently active
        int priority = 0;                   ///< Processing priority (higher = processed first)
        std::vector<InputBinding> bindings; ///< All bindings in this context

        InputContext(const std::string &contextName, int prio = 0)
            : name(contextName), priority(prio)
        {}
    };

    /**
     * @class InputBindingManager
     * @brief Manages input bindings and contexts for the input system
     * 
     * The InputBindingManager handles all input binding configuration, including:
     * - Creating and managing input contexts
     * - Mapping physical inputs to logical actions/axes
     * - Querying bindings for specific inputs or targets
     * - Loading/saving binding configurations
     * - Setting up default bindings for different device types
     */
    class InputBindingManager {
      public:
        InputBindingManager() = default;
        ~InputBindingManager() = default;

        // Context management
        /**
         * @brief Add a new input context
         * @param context The context to add
         */
        void addContext(const InputContext &context);
        
        /**
         * @brief Remove an input context by name
         * @param contextName Name of the context to remove
         */
        void removeContext(const std::string &contextName);
        
        /**
         * @brief Enable or disable a context
         * @param contextName Name of the context
         * @param active true to enable, false to disable
         */
        void setContextActive(const std::string &contextName, bool active);
        
        /**
         * @brief Check if a context is currently active
         * @param contextName Name of the context to check
         * @return true if the context exists and is active
         */
        bool isContextActive(const std::string &contextName) const;

        // Binding management
        /**
         * @brief Add a binding to a context
         * @param contextName Name of the context to add the binding to
         * @param binding The binding to add
         */
        void addBinding(const std::string &contextName, const InputBinding &binding);
        
        /**
         * @brief Remove a binding by target name from a context
         * @param contextName Name of the context
         * @param targetName Name of the action/axis to unbind
         */
        void removeBinding(const std::string &contextName, const std::string &targetName, DeviceType deviceType);
        
        /**
         * @brief Remove all bindings from a context
         * @param contextName Name of the context to clear
         */
        void clearBindings(const std::string &contextName);

        // Binding queries
        /**
         * @brief Get all bindings that match a specific physical input
         * @param deviceType Type of device (KEYBOARD, MOUSE, GAMEPAD)
         * @param deviceId ID of the device (0 for any)
         * @param code Input code (scancode, button ID, axis ID)
         * @return Vector of bindings that match this input
         */
        std::vector<InputBinding> getBindingsForInput(
            DeviceType deviceType, std::uint32_t deviceId, std::uint32_t code) const;
        
        /**
         * @brief Get all bindings for a specific action or axis name
         * @param targetName Name of the action or axis
         * @return Vector of bindings targeting this action/axis
         */
        std::vector<InputBinding> getBindingsForTarget(const std::string &targetName) const;
        
        /**
         * @brief Get all bindings in a specific context
         * @param contextName Name of the context
         * @return Vector of all bindings in this context
         */
        std::vector<InputBinding> getBindingsForContext(const std::string &contextName) const;

        /**
         * @brief Create an action binding (button press -> action)
         * @param deviceType Type of input device
         * @param deviceId Device identifier (0 for any device)
         * @param code Input code (scancode, button ID)
         * @param actionName Name of the action to trigger
         * @param key Display name for UI
         * @return Configured InputBinding for an action
         */
        InputBinding createActionBinding(DeviceType deviceType, std::uint32_t deviceId,
            std::uint32_t code, const std::string &actionName, const std::string &key);
        
        /**
         * @brief Create an axis binding (analog input -> axis value)
         * @param deviceType Type of input device
         * @param deviceId Device identifier (0 for any device)
         * @param code Axis ID
         * @param axisName Name of the axis
         * @param key Display name for UI
         * @param deadzone Minimum value to register (default: 0.1)
         * @param sensitivity Multiplier for axis values (default: 1.0)
         * @param invert If true, inverts the axis direction (default: false)
         * @return Configured InputBinding for an axis
         */
        InputBinding createAxisBinding(DeviceType deviceType, std::uint32_t deviceId,
            std::uint32_t code, const std::string &axisName, const std::string &key, float deadzone = 0.1f,
            float sensitivity = 1.0f, bool invert = false);
        
        /**
         * @brief Create a composite axis binding (button -> virtual axis)
         * @param deviceType Type of input device
         * @param deviceId Device identifier (0 for any device)
         * @param code Input code (scancode, button ID)
         * @param axisName Name of the virtual axis
         * @param key Display name for UI
         * @param value Value to contribute when pressed (-1.0 or 1.0)
         * @param threshold Threshold for button detection (default: 0.5)
         * @return Configured InputBinding for a composite axis
         * 
         * Composite axis bindings allow buttons to contribute to a virtual axis.
         * For example, W and S keys can create a vertical movement axis.
         */
        InputBinding createCompositeAxisBinding(DeviceType deviceType, std::uint32_t deviceId,
            std::uint32_t code, const std::string &axisName, const std::string &key, float value, float threshold = 0.5f);

      private:
        /**
         * @brief Get display name for an input code
         * @param code Input code (scancode, button ID, axis ID)
         * @return Human-readable name for the input
         */
        std::string getKeyNameFromCode(std::uint32_t code) const;

        std::vector<InputContext> _contexts; ///< All registered input contexts

        /**
         * @brief Find a context by name
         * @param name Name of the context
         * @return Index of the context, or -1 if not found
         */
        int findContextIndex(const std::string &name) const;
    };

} // namespace GameEngine
