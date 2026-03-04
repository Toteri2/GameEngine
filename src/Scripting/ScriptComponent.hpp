/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** ScriptComponent - Attach scripts to entities
*/

#pragma once

#include "../Core/component/IComponent.hpp"
#include <cstdint>
#include <optional>
#include <string>

namespace GameEngine::Scripting
{
    using ScriptID = uint32_t;
    constexpr ScriptID INVALID_SCRIPT_ID = 0;

    struct ScriptComponent final : IComponent {
        ScriptID scriptId = INVALID_SCRIPT_ID;
        std::optional<std::string> path;
        bool autoInit = true;

        ScriptComponent() = default;

        explicit ScriptComponent(const ScriptID id, const bool autoInitialize = true)
            : scriptId(id), autoInit(autoInitialize)
        {}

        explicit ScriptComponent(const std::string &path, const bool autoInitialize = true)
            : path(path), autoInit(autoInitialize)
        {}
    };

} // namespace GameEngine::Scripting
