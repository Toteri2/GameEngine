/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** ScriptingHelpers - Helper macros/functions for registering components to Lua
*/

#pragma once

#include <cstdint>

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#include "../../src/Core/EcsManager.hpp"
#include "../../src/Core/managers/EntityManager.hpp"
#include "../../src/Scripting/ScriptManager.hpp"
#include "../../src/Systems/ScriptingSystem/ScriptingSystem.hpp"
#include <LuaBridge/LuaBridge.h>

namespace GameEngine::Scripting
{

    template<typename ComponentType, typename PropertyFunc>
    void registerComponent(ScriptManager &scriptManager, const std::string &className,
        const std::string &getterName, const std::string &hasName, PropertyFunc &&propertyFunc)
    {
        EcsManager &ecs = scriptManager.getEcs();
        lua_State *L = scriptManager.getLuaState();

        scriptManager.registerComponentInternal(className, getterName, hasName, [&]() {
            auto classBuilder =
                luabridge::getGlobalNamespace(L).beginClass<ComponentType>(className.c_str());

            propertyFunc(classBuilder);
            classBuilder.endClass();

            luabridge::getGlobalNamespace(L).addFunction(
                getterName.c_str(), [&ecs](const EntityId entity) -> ComponentType * {
                    if (ecs.hasComponent<ComponentType>(entity)) {
                        return &ecs.getComponent<ComponentType>(entity);
                    }
                    return nullptr;
                });

            luabridge::getGlobalNamespace(L).addFunction(
                hasName.c_str(), [&ecs](const EntityId entity) -> bool {
                    return ecs.hasComponent<ComponentType>(entity);
                });
        });
    }

} // namespace GameEngine::Scripting

// ============================================================================
// Macro helpers for easy component registration
// ============================================================================

// MSVC workaround for __VA_ARGS__ expansion
#define SCRIPT_EXPAND(x) x

#define SCRIPT_GET_MACRO(_1, _2, _3, _4, _5, NAME, ...) NAME

#define SCRIPT_ADD_PROP_1(builder, Type, p1) builder.addProperty(#p1, &Type::p1)

#define SCRIPT_ADD_PROP_2(builder, Type, p1, p2) \
    SCRIPT_ADD_PROP_1(builder, Type, p1).addProperty(#p2, &Type::p2)

#define SCRIPT_ADD_PROP_3(builder, Type, p1, p2, p3) \
    SCRIPT_ADD_PROP_2(builder, Type, p1, p2).addProperty(#p3, &Type::p3)

#define SCRIPT_ADD_PROP_4(builder, Type, p1, p2, p3, p4) \
    SCRIPT_ADD_PROP_3(builder, Type, p1, p2, p3).addProperty(#p4, &Type::p4)

#define SCRIPT_ADD_PROP_5(builder, Type, p1, p2, p3, p4, p5) \
    SCRIPT_ADD_PROP_4(builder, Type, p1, p2, p3, p4).addProperty(#p5, &Type::p5)

#define SCRIPT_ADD_PROPERTIES(builder, Type, ...)                                              \
    SCRIPT_EXPAND(                                                                             \
        SCRIPT_GET_MACRO(__VA_ARGS__, SCRIPT_ADD_PROP_5, SCRIPT_ADD_PROP_4, SCRIPT_ADD_PROP_3, \
            SCRIPT_ADD_PROP_2, SCRIPT_ADD_PROP_1, dummy)(builder, Type, __VA_ARGS__))

// Usage: REGISTER_LUA_COMPONENT(scriptManager, Position, Rtype::PositionComponent, x, y)
#define REGISTER_LUA_COMPONENT(scriptMgr, LuaName, CppType, ...)                           \
    GameEngine::Scripting::registerComponent<CppType>(scriptMgr, #LuaName, "get" #LuaName, \
        "has" #LuaName,                                                                    \
        [](auto &builder) { SCRIPT_ADD_PROPERTIES(builder, CppType, __VA_ARGS__); })
