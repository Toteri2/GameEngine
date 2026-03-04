/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** ScriptContext - Per-entity script instance
*/

#pragma once

#include <cstdint>

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include "../Core/managers/EntityManager.hpp"
#include "ScriptComponent.hpp"
#include <LuaBridge/LuaBridge.h>

namespace GameEngine::Scripting
{

    class ScriptContext {
      public:
        ScriptContext(lua_State *L, EntityId entity, ScriptID scriptId);
        ~ScriptContext();

        auto initialize() -> bool;
        auto update(float deltaTime) -> void;
        auto destroy() -> void;

        auto setCallbacks(luabridge::LuaRef onInit, luabridge::LuaRef onUpdate,
            luabridge::LuaRef onDestroy) -> void;

        [[nodiscard]] auto getEntity() const -> EntityId;
        [[nodiscard]] auto getScriptId() const -> ScriptID;
        [[nodiscard]] auto isInitialized() const -> bool;

      private:
        lua_State *_L;
        EntityId _entity;
        ScriptID _scriptId;
        luabridge::LuaRef _onInit;
        luabridge::LuaRef _onUpdate;
        luabridge::LuaRef _onDestroy;
        bool _initialized;
    };

} // namespace GameEngine::Scripting
