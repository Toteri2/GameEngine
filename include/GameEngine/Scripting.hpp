/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** Scripting - Public API
*/

#pragma once

#include "../../src/Scripting/ScriptComponent.hpp"

namespace GameEngine::Scripting
{
    class ScriptingSystem;
}

namespace GameEngine
{
    using ScriptID = Scripting::ScriptID;
    using ScriptComponent = Scripting::ScriptComponent;
    using ScriptingSystem = Scripting::ScriptingSystem;
    constexpr ScriptID INVALID_SCRIPT_ID = Scripting::INVALID_SCRIPT_ID;
} // namespace GameEngine
