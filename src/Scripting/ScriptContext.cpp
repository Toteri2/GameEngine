/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** ScriptContext
*/

#include "ScriptContext.hpp"

#include <utility>

namespace GameEngine::Scripting
{

    ScriptContext::ScriptContext(lua_State *L, const EntityId entity, const ScriptID scriptId)
        : _L(L), _entity(entity), _scriptId(scriptId), _onInit(L), _onUpdate(L), _onDestroy(L),
          _initialized(false)
    {}

    ScriptContext::~ScriptContext() {}

    auto ScriptContext::initialize() -> bool
    {
        if (_initialized) {
            return true;
        }

        if (_onInit.isFunction()) {
            luabridge::LuaResult const result = _onInit(_entity);
            if (!result.wasOk()) {
                fprintf(stderr, "Error in onInit for entity %u: %s\n", _entity,
                    result.errorMessage().c_str());
                return false;
            }
            _initialized = true;
            return true;
        }

        _initialized = true;
        return true;
    }

    void ScriptContext::update(float deltaTime)
    {
        if (!_initialized) {
            return;
        }

        if (_onUpdate.isFunction()) {
            luabridge::LuaResult const result = _onUpdate(_entity, deltaTime);
            if (!result.wasOk()) {
                fprintf(stderr, "Error in onUpdate for entity %u: %s\n", _entity,
                    result.errorMessage().c_str());
            }
        }
    }

    void ScriptContext::destroy()
    {
        if (!_initialized) {
            return;
        }

        if (_onDestroy.isFunction()) {
            luabridge::LuaResult const result = _onDestroy(_entity);
            if (!result.wasOk()) {
                fprintf(stderr, "Error in onDestroy for entity %u: %s\n", _entity,
                    result.errorMessage().c_str());
            }
        }

        _initialized = false;
    }

    auto ScriptContext::setCallbacks(
        luabridge::LuaRef onInit, luabridge::LuaRef onUpdate, luabridge::LuaRef onDestroy) -> void
    {
        _onInit = std::move(onInit);
        _onUpdate = std::move(onUpdate);
        _onDestroy = std::move(onDestroy);
    }

    auto ScriptContext::getEntity() const -> EntityId
    {
        return _entity;
    }

    auto ScriptContext::getScriptId() const -> ScriptID
    {
        return _scriptId;
    }

    auto ScriptContext::isInitialized() const -> bool
    {
        return _initialized;
    }

} // namespace GameEngine::Scripting
