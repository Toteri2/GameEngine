/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** ScriptManager - Core Lua integration
*/

#pragma once

#include <unordered_map>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <memory>
#include <string>

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include "../Core/EcsManager.hpp"
#include "../Core/managers/EntityManager.hpp"
#include "ScriptComponent.hpp"
#include "ScriptContext.hpp"
#include "FileWatcher.hpp"
#include <LuaBridge/LuaBridge.h>

namespace GameEngine::Scripting
{

    struct LoadedScript {
        ScriptID id;
        std::string filepath;
        luabridge::LuaRef onInit;
        luabridge::LuaRef onUpdate;
        luabridge::LuaRef onDestroy;
        bool valid;

        explicit LoadedScript(lua_State *L)
            : id(INVALID_SCRIPT_ID), onInit(L), onUpdate(L), onDestroy(L), valid(false)
        {}
    };

    class ScriptManager {
      public:
        explicit ScriptManager(EcsManager &ecs);
        ~ScriptManager();

        // Script loading
        ScriptID loadScript(const std::string &filepath);
        ScriptID reloadScript(const std::string &filepath);
        auto unloadScript(ScriptID scriptId) -> bool;

        // Entity management
        auto attachScript(EntityId entity, ScriptID scriptId) -> bool;
        auto detachScript(EntityId entity) -> void;
        auto hasScript(EntityId entity) const -> bool;

        // Execution
        auto initializeEntity(EntityId entity) -> void;
        auto updateEntity(EntityId entity, float deltaTime) -> void;
        auto destroyEntity(EntityId entity) -> void;
        auto updateAll(float deltaTime) -> void;

        // Access
        auto getLuaState() const -> lua_State *;
        auto getEcs() -> EcsManager &;
        auto getEcs() const -> const EcsManager &;

        // Component registration API
        auto registerComponentInternal(const std::string &className, const std::string &getterName,
            const std::string &hasName, const std::function<void()> &registerClassCallback) const
            -> void;

        // Hot reload / File watching
        auto enableAutoReload(bool enable) -> void;
        auto isAutoReloadEnabled() const -> bool;
        auto checkForChanges() -> void;

      private:
        lua_State *_lua;
        EcsManager &_ecs;

        std::deque<LoadedScript> _scripts;
        std::unordered_map<std::string, ScriptID> _filepathToScriptId;
        std::unordered_map<ScriptID, LoadedScript*> _scriptIdToScript;
        ScriptID _nextScriptId;

        std::unordered_map<EntityId, std::unique_ptr<ScriptContext>> _entityContexts;

        FileWatcher _fileWatcher;
        bool _autoReloadEnabled;

        void registerAPI() const;
    };

} // namespace GameEngine::Scripting
