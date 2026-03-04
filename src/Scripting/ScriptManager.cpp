/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** ScriptManager
*/

#include "ScriptManager.hpp"
#include <iostream>

namespace GameEngine::Scripting
{

    ScriptManager::ScriptManager(EcsManager &ecs) : _lua(nullptr), _ecs(ecs), _nextScriptId(1), _autoReloadEnabled(false)
    {
        _lua = luaL_newstate();
        if (!_lua) {
            throw std::runtime_error("Failed to create Lua state");
        }

        luaL_openlibs(_lua);
        registerAPI();
    }

    ScriptManager::~ScriptManager()
    {
        _entityContexts.clear();
        _scripts.clear();

        if (_lua) {
            lua_close(_lua);
            _lua = nullptr;
        }
    }

    auto ScriptManager::loadScript(const std::string &filepath) -> ScriptID
    {
        const auto it = _filepathToScriptId.find(filepath);
        if (it != _filepathToScriptId.end()) {
            return it->second;
        }

        if (!std::filesystem::exists(filepath)) {
            fprintf(stderr, "Script file not found: '%s'\n", filepath.c_str());
            return INVALID_SCRIPT_ID;
        }

        if (luaL_dofile(_lua, filepath.c_str()) != LUA_OK) {
            const char *error = lua_tostring(_lua, -1);
            lua_pop(_lua, 1);
            fprintf(stderr, "Failed to load script '%s': %s\n", filepath.c_str(), error);
            return INVALID_SCRIPT_ID;
        }

        LoadedScript script(_lua);
        script.id = _nextScriptId++;
        script.filepath = filepath;
        script.valid = true;

        lua_getglobal(_lua, "onInit");
        if (lua_isfunction(_lua, -1)) {
            script.onInit = luabridge::LuaRef::fromStack(_lua, -1);
        }
        lua_pop(_lua, 1);

        lua_getglobal(_lua, "onUpdate");
        if (lua_isfunction(_lua, -1)) {
            script.onUpdate = luabridge::LuaRef::fromStack(_lua, -1);
        }
        lua_pop(_lua, 1);

        lua_getglobal(_lua, "onDestroy");
        if (lua_isfunction(_lua, -1)) {
            script.onDestroy = luabridge::LuaRef::fromStack(_lua, -1);
        }
        lua_pop(_lua, 1);

        ScriptID const id = script.id;
        _scripts.push_back(std::move(script));
        _filepathToScriptId[filepath] = id;
        _scriptIdToScript[id] = &_scripts.back();

        if (_autoReloadEnabled) {
            _fileWatcher.addWatch(filepath);
        }

        return id;
    }

    auto ScriptManager::reloadScript(const std::string &filepath) -> ScriptID
    {
        const auto it = _filepathToScriptId.find(filepath);
        if (it == _filepathToScriptId.end()) {
            fprintf(stdout, "[ScriptManager] Script '%s' not loaded, loading for first time\n", filepath.c_str());
            return loadScript(filepath);
        }

        ScriptID const oldId = it->second;
        fprintf(stdout, "[ScriptManager] Reloading script '%s' (ID: %u)\n", filepath.c_str(), oldId);

        std::vector<EntityId> affectedEntities;
        for (const auto& [entity, context] : _entityContexts) {
            if (context->getScriptId() == oldId) {
                affectedEntities.push_back(entity);
            }
        }

        fprintf(stdout, "[ScriptManager] Found %zu entities using this script\n", affectedEntities.size());

        for (EntityId const entity : affectedEntities) {
            destroyEntity(entity);  // Calls onDestroy
            detachScript(entity);
        }

        const auto scriptIt = _scriptIdToScript.find(oldId);
        if (scriptIt != _scriptIdToScript.end()) {
            scriptIt->second->valid = false;
        }

        _filepathToScriptId.erase(filepath);
        _scriptIdToScript.erase(oldId);
        ScriptID const newId = loadScript(filepath);

        if (newId == INVALID_SCRIPT_ID) {
            fprintf(stderr, "[ScriptManager] ERROR: Failed to reload script '%s'\n", filepath.c_str());
            return INVALID_SCRIPT_ID;
        }

        fprintf(stdout, "[ScriptManager] Successfully loaded new version (ID: %u)\n", newId);

        for (EntityId const entity : affectedEntities) {
            if (attachScript(entity, newId)) {
                initializeEntity(entity);
                fprintf(stdout, "[ScriptManager] Re-attached script to entity %u\n", entity);
            } else {
                fprintf(stderr, "[ScriptManager] WARNING: Failed to re-attach script to entity %u\n", entity);
            }
        }

        fprintf(stdout, "[ScriptManager] Hot reload complete for '%s'\n", filepath.c_str());
        return newId;
    }

    auto ScriptManager::unloadScript(ScriptID scriptId) -> bool
    {
        for (const auto& [entity, context] : _entityContexts) {
            if (context->getScriptId() == scriptId) {
                fprintf(stderr, "[ScriptManager] Cannot unload script %u: still attached to entity %u\n",
                        scriptId, entity);
                return false;
            }
        }

        const auto it = _scriptIdToScript.find(scriptId);
        if (it == _scriptIdToScript.end()) {
            return false;
        }

        LoadedScript* scriptPtr = it->second;
        std::string const filepath = scriptPtr->filepath;

        scriptPtr->valid = false;
        scriptPtr->onInit = luabridge::LuaRef(_lua);
        scriptPtr->onUpdate = luabridge::LuaRef(_lua);
        scriptPtr->onDestroy = luabridge::LuaRef(_lua);

        _scriptIdToScript.erase(it);
        _filepathToScriptId.erase(filepath);

        fprintf(stdout, "[ScriptManager] Unloaded script %u ('%s')\n", scriptId, filepath.c_str());
        return true;
    }

    auto ScriptManager::attachScript(EntityId entity, ScriptID scriptId) -> bool
    {
        const auto it = _scriptIdToScript.find(scriptId);
        if (it == _scriptIdToScript.end()) {
            return false;
        }

        LoadedScript const *script = it->second;
        if (!script || !script->valid) {
            return false;
        }

        auto context = std::make_unique<ScriptContext>(_lua, entity, scriptId);
        context->setCallbacks(script->onInit, script->onUpdate, script->onDestroy);
        _entityContexts[entity] = std::move(context);
        return true;
    }

    auto ScriptManager::detachScript(const EntityId entity) -> void
    {
        const auto it = _entityContexts.find(entity);
        if (it != _entityContexts.end()) {
            _entityContexts.erase(it);
        }
    }

    auto ScriptManager::hasScript(const EntityId entity) const -> bool
    {
        return _entityContexts.contains(entity);
    }

    auto ScriptManager::initializeEntity(const EntityId entity) -> void
    {
        const auto it = _entityContexts.find(entity);
        if (it != _entityContexts.end() && !it->second->isInitialized()) {
            it->second->initialize();
        }
    }

    auto ScriptManager::updateEntity(const EntityId entity, const float deltaTime) -> void
    {
        const auto it = _entityContexts.find(entity);
        if (it != _entityContexts.end() && it->second->isInitialized()) {
            it->second->update(deltaTime);
        }
    }

    auto ScriptManager::destroyEntity(const EntityId entity) -> void
    {
        const auto it = _entityContexts.find(entity);
        if (it != _entityContexts.end()) {
            it->second->destroy();
        }
    }

    auto ScriptManager::updateAll(const float deltaTime) -> void
    {
        for (const auto &context : _entityContexts | std::views::values) {
            if (context->isInitialized()) {
                context->update(deltaTime);
            }
        }
    }

    auto ScriptManager::getLuaState() const -> lua_State *
    {
        return _lua;
    }

    auto ScriptManager::getEcs() -> EcsManager &
    {
        return _ecs;
    }

    auto ScriptManager::getEcs() const -> const EcsManager &
    {
        return _ecs;
    }

    auto ScriptManager::registerAPI() const -> void
    {
        lua_getglobal(_lua, "print");
        if (lua_isfunction(_lua, -1)) {
            lua_pushstring(_lua,
                "GameEngine Scripting API loaded! Customize registerAPI() to expose your "
                "components.");
            if (lua_pcall(_lua, 1, 0, 0) != LUA_OK) {
                const char *error = lua_tostring(_lua, -1);
                fprintf(
                    stderr, "Error calling print in registerAPI: %s\n", error ? error : "unknown");
                lua_pop(_lua, 1);
            }
        }
    }

    auto ScriptManager::registerComponentInternal(const std::string &className,
        const std::string &getterName, const std::string &hasName,
        const std::function<void()> &registerClassCallback) const -> void
    {
        if (!_lua) {
            fprintf(stderr, "ScriptManager not initialized!\n");
            return;
        }

        registerClassCallback();
        std::cout << "[Scripting] Registered component class: " << className << "\n";
        std::cout << "[Scripting] Registered functions: " << getterName << "(), " << hasName
                  << "()\n";
    }

    auto ScriptManager::enableAutoReload(bool enable) -> void
    {
        if (_autoReloadEnabled == enable) {
            return;
        }

        _autoReloadEnabled = enable;

        if (enable) {
            for (const auto &filepath : _filepathToScriptId | std::views::keys) {
                _fileWatcher.addWatch(filepath);
            }
            fprintf(stdout, "[ScriptManager] Auto-reload ENABLED (%zu scripts watched)\n",
                    _fileWatcher.getWatchCount());
        } else {
            _fileWatcher.clearAll();
            fprintf(stdout, "[ScriptManager] Auto-reload DISABLED\n");
        }
    }

    auto ScriptManager::isAutoReloadEnabled() const -> bool
    {
        return _autoReloadEnabled;
    }

    auto ScriptManager::checkForChanges() -> void
    {
        if (!_autoReloadEnabled) {
            return;
        }

        const auto changedFiles = _fileWatcher.getChangedFiles();

        if (changedFiles.empty()) {
            return;
        }

        fprintf(stdout, "[ScriptManager] Detected %zu changed script(s), auto-reloading...\n",
                changedFiles.size());

        for (const auto& filepath : changedFiles) {
            fprintf(stdout, "[ScriptManager] Auto-reloading: %s\n", filepath.c_str());
            ScriptID const newId = reloadScript(filepath);

            if (newId != INVALID_SCRIPT_ID) {
                fprintf(stdout, "[ScriptManager] Successfully auto-reloaded: %s\n", filepath.c_str());
            } else {
                fprintf(stderr, "[ScriptManager] Failed to auto-reload: %s\n", filepath.c_str());
            }
        }
    }

} // namespace GameEngine::Scripting
