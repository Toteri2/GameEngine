/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** InputState - Centralized input state for polling
*/

#pragma once

#include <unordered_map>
#include <cstdint>
#include <string>

namespace GameEngine
{

    /**
     * @struct ActionState
     * @brief Stores the state of a digital action (button/key)
     *
     * Contains per-frame flags to allow both polling and edge-detection:
     * - `down` indicates whether the action is currently held
     * - `pressedThisFrame` and `releasedThisFrame` indicate edges within the frame
     */
    struct ActionState {
        bool down = false;              ///< Currently pressed
        bool pressedThisFrame = false;  ///< Just pressed this frame
        bool releasedThisFrame = false; ///< Just released this frame
        std::uint64_t timestamp = 0;    ///< Time of last change (ms since epoch)
        std::uint32_t playerId = 0;     ///< Which player triggered the action

        /**
         * @brief Update action state
         * @param newDown New down state
         * @param newTimestamp Timestamp of the change (ms)
         * @param player Player id that caused the change
         */
        void update(bool newDown, std::uint64_t newTimestamp, std::uint32_t player)
        {
            pressedThisFrame = !down && newDown;
            releasedThisFrame = down && !newDown;
            down = newDown;
            timestamp = newTimestamp;
            playerId = player;
        }

        /**
         * @brief Clear per-frame edge flags
         * Called once per frame after the frame has been processed.
         */
        void clearFrameFlags()
        {
            pressedThisFrame = false;
            releasedThisFrame = false;
        }
    };

    /**
     * @struct AxisState
     * @brief Stores the state of an analog axis (thumbstick, trigger, virtual axis)
     *
     * Tracks the current and previous values and whether the axis changed this frame.
     */
    struct AxisState {
        float value = 0.0f;            ///< Current value in [-1.0, 1.0]
        float previousValue = 0.0f;    ///< Value in previous frame
        bool changedThisFrame = false; ///< Whether the value changed this frame
        std::uint64_t timestamp = 0;   ///< Time of last change (ms since epoch)
        std::uint32_t playerId = 0;    ///< Which player controls this axis

        /**
         * @brief Update the axis state
         * @param newValue New axis value
         * @param newTimestamp Timestamp of the change (ms)
         * @param player Player id that produced the change
         */
        void update(float newValue, std::uint64_t newTimestamp, std::uint32_t player)
        {
            changedThisFrame = (std::abs(value - newValue) > 0.001f);
            previousValue = value;
            value = newValue;
            if (changedThisFrame) {
                timestamp = newTimestamp;
                playerId = player;
            }
        }

        /**
         * @brief Clear per-frame change flag
         */
        void clearFrameFlags() { changedThisFrame = false; }
    };

    /**
     * @class InputState
     * @brief Central storage for the current input state used by systems to poll actions/axes
     *
     * Provides methods to update actions/axes and to clear per-frame flags after processing.
     */
    class InputState {
      public:
        InputState() = default;
        ~InputState() = default;

        /**
         * @brief Get the current value of an axis
         * @param axisName Name of the axis
         * @return Current axis value (0.0 if not present)
         */
        float getAxisValue(const std::string &axisName) const;

        /**
         * @brief Update or create an action state
         * @param actionName Name of the action
         * @param down Whether the action is currently down
         * @param timestamp Timestamp of the update (ms)
         * @param playerId Player id responsible for the update
         */
        void updateAction(const std::string &actionName, bool down, std::uint64_t timestamp,
            std::uint32_t playerId);

        /**
         * @brief Update or create an axis state
         * @param axisName Name of the axis
         * @param value New axis value
         * @param timestamp Timestamp of the update (ms)
         * @param playerId Player id responsible for the update
         */
        void updateAxis(const std::string &axisName, float value, std::uint64_t timestamp,
            std::uint32_t playerId);

        /**
         * @brief Clear per-frame flags for all actions and axes
         * Should be called at the end of each frame.
         */
        void clearFrameFlags();

      private:
        std::unordered_map<std::string, ActionState> _actions; ///< Map action name -> state
        std::unordered_map<std::string, AxisState> _axes;       ///< Map axis name -> state
    };

} // namespace GameEngine
