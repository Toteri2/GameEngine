/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** InputState - Implementation
*/

#include "InputState.hpp"
#include <iostream>

namespace GameEngine
{

    float InputState::getAxisValue(const std::string &axisName) const
    {
        auto it = _axes.find(axisName);
        return it != _axes.end() ? it->second.value : 0.0f;
    }

    void InputState::updateAction(
        const std::string &actionName, bool down, std::uint64_t timestamp, std::uint32_t playerId)
    {
        _actions[actionName].update(down, timestamp, playerId);
    }

    void InputState::updateAxis(
        const std::string &axisName, float value, std::uint64_t timestamp, std::uint32_t playerId)
    {
        _axes[axisName].update(value, timestamp, playerId);
    }

    void InputState::clearFrameFlags()
    {
        for (auto &[name, action] : _actions) {
            action.clearFrameFlags();
        }

        for (auto &[name, axis] : _axes) {
            axis.clearFrameFlags();
        }
    }
} // namespace GameEngine
