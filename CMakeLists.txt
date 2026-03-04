cmake_minimum_required(VERSION 3.16)
project(GameEngine VERSION 1.0.0 LANGUAGES C CXX)

# --- CPM Boilerplate ---
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cmake)
file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.8/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH SHA256=78ba32abdf798bc616bab7c73aac32a17bbd7b06ad9e26a6add69de8f3ae4791
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
# -----------------------

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(game_engine_core INTERFACE)

target_include_directories(game_engine_core
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(GameEngine::core ALIAS game_engine_core)

# Time Manager (standalone utility library - no ECS dependency)
option(GAMEENGINE_BUILD_TIME "Build Time utilities" OFF)

if(GAMEENGINE_BUILD_TIME)
    add_library(game_engine_time STATIC
        src/Utils/Time/TimeManager.cpp
    )

    target_include_directories(game_engine_time
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    add_library(GameEngine::time ALIAS game_engine_time)

    target_compile_definitions(game_engine_core INTERFACE GAMEENGINE_HAS_TIME)

    list(APPEND INSTALL_TARGETS game_engine_time)

    message(STATUS "GameEngine: Time utilities enabled")
endif()

# Logger Manager (standalone library - no ECS dependency)
option(GAMEENGINE_BUILD_LOGGER "Build Logger subsystem" OFF)
# Logger System (ECS integration - requires both Core + Logger)
option(GAMEENGINE_BUILD_LOGGER_SYSTEM "Build Logger ECS System" OFF)

# Network Manager (standalone library - no ECS dependency)
option(GAMEENGINE_BUILD_NETWORK "Build Network subsystem" OFF)

# Network System (ECS integration - requires Core + Network)
option(GAMEENGINE_BUILD_NETWORK_SYSTEM "Build Network ECS System" OFF)

# Audio Manager (standalone library - no ECS dependency)
option(GAMEENGINE_BUILD_AUDIO "Build Audio subsystem" OFF)
# Audio System (ECS integration - requires both Core)
option(GAMEENGINE_BUILD_AUDIO_SYSTEM "Build Audio ECS System" OFF)

option(GAMEENGINE_BUILD_SCRIPTING "Build Scripting subsystem" OFF)
option(GAMEENGINE_BUILD_SCRIPTING_SYSTEM "Build Scripting ECS System" OFF)

# Physics System (ECS integration - requires Core + Box2D)
option(GAMEENGINE_BUILD_PHYSICS_SYSTEM "Build Physics ECS System" OFF)

# Developer Console System (ECS integration - requires Core + Render)
option(GAMEENGINE_BUILD_CONSOLE_SYSTEM "Build Developer Console ECS System" OFF)

set(INSTALL_TARGETS game_engine_core)

if(GAMEENGINE_BUILD_LOGGER)
    add_library(game_engine_logger STATIC
        src/Utils/Logger/Logger.cpp
    )

    target_include_directories(game_engine_logger
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    add_library(GameEngine::logger ALIAS game_engine_logger)

    target_compile_definitions(game_engine_core INTERFACE GAMEENGINE_HAS_LOGGER)

    list(APPEND INSTALL_TARGETS game_engine_logger)

    message(STATUS "GameEngine: Logger subsystem enabled")
endif()

if(GAMEENGINE_BUILD_LOGGER_SYSTEM)
    if(NOT GAMEENGINE_BUILD_LOGGER)
        message(FATAL_ERROR "GAMEENGINE_BUILD_LOGGER_SYSTEM requires GAMEENGINE_BUILD_LOGGER=ON")
    endif()

    add_library(game_engine_logger_system STATIC
        src/Systems/LoggerSystem/LoggerSystem.cpp
    )

    target_include_directories(game_engine_logger_system
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(game_engine_logger_system PUBLIC
        GameEngine::core
        GameEngine::logger
    )

    add_library(GameEngine::logger_system ALIAS game_engine_logger_system)

    target_compile_definitions(game_engine_logger_system PUBLIC GAMEENGINE_HAS_LOGGER_SYSTEM)

    list(APPEND INSTALL_TARGETS game_engine_logger_system)

    message(STATUS "GameEngine: Logger ECS System enabled")
endif()

# NETWORK SUBSYSTEM

if(GAMEENGINE_BUILD_NETWORK)
    CPMAddPackage(
        NAME Boost.Asio
        GITHUB_REPOSITORY boostorg/asio
        GIT_TAG boost-1.84.0
    )
    CPMAddPackage(
        NAME Boost.System
        GITHUB_REPOSITORY boostorg/system
        GIT_TAG boost-1.84.0
    )
    CPMAddPackage(
        NAME Boost.Config
        GITHUB_REPOSITORY boostorg/config
        GIT_TAG boost-1.84.0
    )
    CPMAddPackage(
        NAME Boost.Assert
        GITHUB_REPOSITORY boostorg/assert
        GIT_TAG boost-1.84.0
    )
    CPMAddPackage(
        NAME Boost.ThrowException
        GITHUB_REPOSITORY boostorg/throw_exception
        GIT_TAG boost-1.84.0
    )
    CPMAddPackage(
        NAME Boost.Mp11
        GITHUB_REPOSITORY boostorg/mp11
        GIT_TAG boost-1.84.0
    )
    CPMAddPackage(
        NAME Boost.Predef
        GITHUB_REPOSITORY boostorg/predef
        GIT_TAG boost-1.84.0
    )
    CPMAddPackage(
        NAME Boost.Align
        GITHUB_REPOSITORY boostorg/align
        GIT_TAG boost-1.84.0
    )
    CPMAddPackage(
        NAME Boost.WinAPI
        GITHUB_REPOSITORY boostorg/winapi
        GIT_TAG boost-1.84.0
    )
    CPMAddPackage(
        NAME Boost.Variant2
        GITHUB_REPOSITORY boostorg/variant2
        GIT_TAG boost-1.84.0
    )

    add_library(game_engine_network STATIC
        src/Utils/Network/NetworkManager.cpp
        src/Utils/Network/NetworkReceiver.cpp
    )

    target_include_directories(game_engine_network
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${Boost.Asio_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${Boost.System_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${Boost.Config_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${Boost.Assert_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${Boost.ThrowException_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${Boost.Variant2_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${Boost.Mp11_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${Boost.Predef_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${Boost.Align_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${Boost.WinAPI_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    add_library(GameEngine::network ALIAS game_engine_network)

    target_compile_definitions(game_engine_core INTERFACE GAMEENGINE_HAS_NETWORK)

    list(APPEND INSTALL_TARGETS game_engine_network)

    message(STATUS "GameEngine: Network subsystem enabled")
endif()

# Network System (ECS integration - requires both Core + Network)
if(GAMEENGINE_BUILD_NETWORK_SYSTEM)
    if(NOT GAMEENGINE_BUILD_NETWORK)
        message(FATAL_ERROR "GAMEENGINE_BUILD_NETWORK_SYSTEM requires GAMEENGINE_BUILD_NETWORK=ON")
    endif()

    add_library(game_engine_network_system STATIC
        src/Systems/NetworkSystem/NetworkSystem.cpp
    )

    target_include_directories(game_engine_network_system
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(game_engine_network_system PUBLIC
        GameEngine::core
        GameEngine::network
    )

    add_library(GameEngine::network_system ALIAS game_engine_network_system)

    target_compile_definitions(game_engine_network_system PUBLIC GAMEENGINE_HAS_NETWORK_SYSTEM)

    list(APPEND INSTALL_TARGETS game_engine_network_system)

    message(STATUS "GameEngine: Network ECS System enabled")
endif()

# ═══════════════════════════════════════════════════════════════════
# AUDIO SUBSYSTEM

if(GAMEENGINE_BUILD_AUDIO)
    # miniaudio - Single header audio library
    CPMAddPackage(
        NAME miniaudio
        GITHUB_REPOSITORY mackron/miniaudio
        GIT_TAG 0.11.21
    )

    # Create a wrapper source file for miniaudio implementation
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/miniaudio_impl.c"
        "#define MINIAUDIO_IMPLEMENTATION\n#include \"miniaudio.h\"\n")

    add_library(game_engine_audio STATIC
        src/Utils/Audio/AudioManager.cpp
        "${CMAKE_CURRENT_BINARY_DIR}/miniaudio_impl.c"
    )
    target_include_directories(game_engine_audio
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${miniaudio_SOURCE_DIR}>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    add_library(GameEngine::audio ALIAS game_engine_audio)
    target_compile_definitions(game_engine_core INTERFACE GAMEENGINE_HAS_AUDIO)
    list(APPEND INSTALL_TARGETS game_engine_audio)
    message(STATUS "GameEngine: Audio subsystem enabled")
endif()

if(GAMEENGINE_BUILD_AUDIO_SYSTEM)
    if(NOT GAMEENGINE_BUILD_AUDIO)
        message(FATAL_ERROR "GAMEENGINE_BUILD_AUDIO_SYSTEM requires GAMEENGINE_BUILD_AUDIO=ON")
    endif()

    add_library(game_engine_audio_system STATIC
        src/Systems/AudioSystem/AudioSystem.cpp
    )

    target_include_directories(game_engine_audio_system
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(game_engine_audio_system PUBLIC
        GameEngine::core
        GameEngine::audio
    )

    add_library(GameEngine::audio_system ALIAS game_engine_audio_system)

    target_compile_definitions(game_engine_audio_system PUBLIC GAMEENGINE_HAS_AUDIO_SYSTEM)

    list(APPEND INSTALL_TARGETS game_engine_audio_system)

    message(STATUS "GameEngine: Audio ECS System enabled")
endif()

# ═══════════════════════════════════════════════════════════════════
# LEVEL SYSTEM
# ═══════════════════════════════════════════════════════════════════

option(GAMEENGINE_BUILD_LEVEL_SYSTEM "Build Level System" OFF)

if(GAMEENGINE_BUILD_LEVEL_SYSTEM)
    CPMAddPackage(
        NAME nlohmann_json
        VERSION 3.11.3
        GITHUB_REPOSITORY nlohmann/json
        OPTIONS
        "JSON_BuildTests OFF"
        "JSON_Install OFF"
    )

    add_library(game_engine_level_system STATIC
        src/Systems/LevelSystem/PrefabManager.cpp
        src/Systems/LevelSystem/LevelLoader.cpp
        src/Systems/LevelSystem/LevelSystem.cpp
    )

    target_include_directories(game_engine_level_system
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(game_engine_level_system
        PUBLIC
        GameEngine::core
    )

    add_library(GameEngine::level_system ALIAS game_engine_level_system)

    target_compile_definitions(game_engine_level_system PUBLIC GAMEENGINE_HAS_LEVEL_SYSTEM)

    list(APPEND INSTALL_TARGETS game_engine_level_system)

    message(STATUS "GameEngine: Level System enabled")
endif()

# ═══════════════════════════════════════════════════════════════════
# SCRIPTING SUBSYSTEM

if(GAMEENGINE_BUILD_SCRIPTING)
    message(STATUS "GameEngine: Fetching Lua 5.4.7...")

    CPMAddPackage(
        NAME lua
        URL https://www.lua.org/ftp/lua-5.4.7.tar.gz
        URL_HASH SHA256=9fbf5e28ef86c69858f6d3d34eccc32e911c1a28b4120ff3e84aaa70cfbf1e30
        DOWNLOAD_ONLY YES
    )

    if(lua_ADDED)
        # Build Lua as a library
        file(GLOB LUA_CORE_SOURCES
            "${lua_SOURCE_DIR}/src/*.c"
        )

        # Remove lua.c and luac.c (they contain main())
        list(FILTER LUA_CORE_SOURCES EXCLUDE REGEX ".*/lua\\.c$")
        list(FILTER LUA_CORE_SOURCES EXCLUDE REGEX ".*/luac\\.c$")

        add_library(lua_static STATIC ${LUA_CORE_SOURCES})

        target_include_directories(lua_static
            PUBLIC
            $<BUILD_INTERFACE:${lua_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:include/lua>
        )

        if(UNIX)
            target_compile_definitions(lua_static PUBLIC LUA_USE_POSIX)
            target_link_libraries(lua_static PUBLIC m dl)
        endif()

        list(APPEND INSTALL_TARGETS lua_static)
    endif()

    set(LUA_INCLUDE_DIR ${lua_SOURCE_DIR}/src)
    set(LUA_LIBRARIES lua_static)
    set(LUA_VERSION_STRING "5.4.7")

    CPMAddPackage(
        NAME LuaBridge3
        GITHUB_REPOSITORY kunitoki/LuaBridge3
        GIT_TAG 3.0-rc3
    )

    # Fix LuaBridge's target properties to use proper generator expressions
    if(TARGET LuaBridge)
        get_target_property(LUABRIDGE_INCLUDES LuaBridge INTERFACE_INCLUDE_DIRECTORIES)

        set_target_properties(LuaBridge PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ""
        )

        target_include_directories(LuaBridge INTERFACE
            $<BUILD_INTERFACE:${LuaBridge3_SOURCE_DIR}/Source>
            $<INSTALL_INTERFACE:include/LuaBridge>
        )

        set_target_properties(LuaBridge PROPERTIES
            INTERFACE_SOURCES ""
        )

        list(APPEND INSTALL_TARGETS LuaBridge)
    endif()

    add_library(game_engine_scripting STATIC
        src/Scripting/ScriptManager.cpp
        src/Scripting/ScriptContext.cpp
        src/Scripting/FileWatcher.cpp
    )

    target_include_directories(game_engine_scripting
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${LUA_INCLUDE_DIR}
    )

    target_link_libraries(game_engine_scripting
        PUBLIC
        LuaBridge
        ${LUA_LIBRARIES}
    )

    add_library(GameEngine::scripting ALIAS game_engine_scripting)
    target_compile_definitions(game_engine_core INTERFACE GAMEENGINE_HAS_SCRIPTING)
    list(APPEND INSTALL_TARGETS game_engine_scripting)

    message(STATUS "GameEngine: Scripting subsystem enabled")
    message(STATUS "  - Lua version: ${LUA_VERSION_STRING}")
    message(STATUS "  - Lua libraries: ${LUA_LIBRARIES}")
endif()

if(GAMEENGINE_BUILD_SCRIPTING_SYSTEM)
    if(NOT GAMEENGINE_BUILD_SCRIPTING)
        message(FATAL_ERROR "GAMEENGINE_BUILD_SCRIPTING_SYSTEM requires GAMEENGINE_BUILD_SCRIPTING=ON")
    endif()

    add_library(game_engine_scripting_system STATIC
        src/Systems/ScriptingSystem/ScriptingSystem.cpp
    )

    target_include_directories(game_engine_scripting_system
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(game_engine_scripting_system
        PUBLIC
        GameEngine::core
        GameEngine::scripting
    )

    add_library(GameEngine::scripting_system ALIAS game_engine_scripting_system)
    target_compile_definitions(game_engine_core INTERFACE GAMEENGINE_HAS_SCRIPTING_SYSTEM)
    list(APPEND INSTALL_TARGETS game_engine_scripting_system)

    message(STATUS "GameEngine: Scripting ECS System enabled")
endif()

if(GAMEENGINE_BUILD_PHYSICS_SYSTEM)
    CPMAddPackage(
        NAME box2d
        GITHUB_REPOSITORY erincatto/box2d
        GIT_TAG v3.1.1
    )

    if(box2d_ADDED)
        message(STATUS "GameEngine: Box2D added for Physics System")
    endif()

    add_library(game_engine_physics_system STATIC
        src/Utils/Physics/PhysicsManager.cpp
        src/Systems/PhysicsSystem/PhysicsSystem.cpp
    )

    target_include_directories(game_engine_physics_system
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${box2d_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(game_engine_physics_system PUBLIC
        GameEngine::core
        box2d
    )

    add_library(GameEngine::physics_system ALIAS game_engine_physics_system)

    target_compile_definitions(game_engine_physics_system PUBLIC GAMEENGINE_HAS_PHYSICS_SYSTEM)

    list(APPEND INSTALL_TARGETS game_engine_physics_system)

    message(STATUS "GameEngine: Physics ECS System enabled")
endif()

# Input System (ECS integration - requires SDL3)
option(GAMEENGINE_BUILD_INPUT_SYSTEM "Build Input ECS System" OFF)

if(GAMEENGINE_BUILD_INPUT_SYSTEM)
    if(NOT TARGET SDL3::SDL3)
        CPMAddPackage(
            NAME SDL3
            GITHUB_REPOSITORY libsdl-org/SDL
            GIT_TAG release-3.2.22
        )
    endif()
    add_library(game_engine_input_system STATIC
        src/Systems/InputSystem/InputSystem.cpp
        src/Systems/InputSystem/InputManager.cpp
        src/Systems/InputSystem/InputBindingManager.cpp
        src/Systems/InputSystem/InputState.cpp
        src/Systems/InputSystem/SDLInputDevice.cpp
    )

    target_include_directories(game_engine_input_system
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # Link SDL3 for input polling
    if(DEFINED sdl3_SOURCE_DIR)
        target_include_directories(game_engine_input_system PUBLIC
            $<BUILD_INTERFACE:${sdl3_SOURCE_DIR}/include>
        )
        target_link_libraries(game_engine_input_system PUBLIC
            GameEngine::core
            $<BUILD_INTERFACE:SDL3::SDL3>
            $<INSTALL_INTERFACE:SDL3::SDL3>
        )
    else()
        find_package(SDL3 REQUIRED)
        target_link_libraries(game_engine_input_system PUBLIC
            GameEngine::core
            SDL3::SDL3
        )
    endif()

    # Link logger if available
    if(GAMEENGINE_BUILD_LOGGER)
        target_link_libraries(game_engine_input_system PUBLIC GameEngine::logger)
    endif()

    add_library(GameEngine::input_system ALIAS game_engine_input_system)

    target_compile_definitions(game_engine_input_system PUBLIC GAMEENGINE_HAS_INPUT_SYSTEM)

    # Only add to install targets if we're the root project
    if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        list(APPEND INSTALL_TARGETS game_engine_input_system)
    endif()

    message(STATUS "GameEngine: Input ECS System enabled")
endif()

# Render System (ECS integration - requires SDL3, SDL3_image, SDL3_ttf, SDL3_gfx)
option(GAMEENGINE_BUILD_RENDER_SYSTEM "Build Render ECS System" OFF)

if(GAMEENGINE_BUILD_RENDER_SYSTEM)
    CPMAddPackage(
        NAME SDL3
        GITHUB_REPOSITORY libsdl-org/SDL
        GIT_TAG release-3.2.22
    )

    CPMAddPackage(
        NAME SDL3_ttf
        GITHUB_REPOSITORY libsdl-org/SDL_ttf
        GIT_TAG release-3.2.2
    )

    CPMAddPackage(
        NAME SDL3_image
        GITHUB_REPOSITORY libsdl-org/SDL_image
        GIT_TAG release-3.2.4
    )

    CPMAddPackage(
        NAME SDL3_gfx
        GITHUB_REPOSITORY sabdul-khabir/SDL3_gfx
        GIT_TAG main
    )

    if(TARGET SDL3_gfx_Static)
        set_target_properties(SDL3_gfx_Static PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
        target_link_libraries(SDL3_gfx_Static PUBLIC SDL3::SDL3)
    endif()

    if(TARGET SDL3_gfx_Shared)
        set_target_properties(SDL3_gfx_Shared PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
        target_link_libraries(SDL3_gfx_Shared PUBLIC SDL3::SDL3)
    endif()
    add_library(game_engine_render_system STATIC
        src/Systems/RenderSystem/RenderSystem.cpp
        src/Utils/Rendering/RenderManager.cpp
        src/Utils/Rendering/wrapperSDL3.cpp
    )

    target_include_directories(game_engine_render_system
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    if(DEFINED sdl3_SOURCE_DIR)
        target_include_directories(game_engine_render_system PUBLIC
            $<BUILD_INTERFACE:${sdl3_SOURCE_DIR}/include>
        )
        target_link_libraries(game_engine_render_system PUBLIC
            GameEngine::core
            $<BUILD_INTERFACE:SDL3::SDL3>
            $<INSTALL_INTERFACE:SDL3::SDL3>
        )
    else()
        find_package(SDL3 REQUIRED)
        target_link_libraries(game_engine_render_system PUBLIC
            GameEngine::core
            SDL3::SDL3
        )
    endif()

    # Link SDL3_image
    if(DEFINED sdl3_image_SOURCE_DIR)
        target_include_directories(game_engine_render_system PUBLIC
            $<BUILD_INTERFACE:${sdl3_image_SOURCE_DIR}/include>
        )
        target_link_libraries(game_engine_render_system PUBLIC 
            $<BUILD_INTERFACE:SDL3_image::SDL3_image>
            $<INSTALL_INTERFACE:SDL3_image::SDL3_image>
        )
    else()
        find_package(SDL3_image REQUIRED)
        target_link_libraries(game_engine_render_system PUBLIC SDL3_image::SDL3_image)
    endif()

    # Link SDL3_ttf
    if(DEFINED sdl3_ttf_SOURCE_DIR)
        target_include_directories(game_engine_render_system PUBLIC
            $<BUILD_INTERFACE:${sdl3_ttf_SOURCE_DIR}/include>
        )
        target_link_libraries(game_engine_render_system PUBLIC 
            $<BUILD_INTERFACE:SDL3_ttf::SDL3_ttf>
            $<INSTALL_INTERFACE:SDL3_ttf::SDL3_ttf>
        )
    else()
        find_package(SDL3_ttf REQUIRED)
        target_link_libraries(game_engine_render_system PUBLIC SDL3_ttf::SDL3_ttf)
    endif()

    # Link SDL3_gfx (for primitives like circles)
    if(TARGET SDL3_gfx_Shared)
        target_include_directories(game_engine_render_system PUBLIC
            $<BUILD_INTERFACE:${SDL3_gfx_SOURCE_DIR}>
        )
        target_link_libraries(game_engine_render_system PUBLIC SDL3_gfx_Shared)
    elseif(TARGET SDL3_gfx_Static)
        target_include_directories(game_engine_render_system PUBLIC
            $<BUILD_INTERFACE:${SDL3_gfx_SOURCE_DIR}>
        )
        target_link_libraries(game_engine_render_system PUBLIC SDL3_gfx_Static)
    elseif(TARGET SDL3_gfx)
        target_include_directories(game_engine_render_system PUBLIC
            $<BUILD_INTERFACE:${SDL3_gfx_SOURCE_DIR}>
        )
        target_link_libraries(game_engine_render_system PUBLIC SDL3_gfx)
    endif()

    # Link logger if available
    if(GAMEENGINE_BUILD_LOGGER)
        target_link_libraries(game_engine_render_system PUBLIC GameEngine::logger)
    endif()

    add_library(GameEngine::render_system ALIAS game_engine_render_system)

    target_compile_definitions(game_engine_render_system PUBLIC GAMEENGINE_HAS_RENDER_SYSTEM)

    # Only add to install targets if we're the root project
    if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        list(APPEND INSTALL_TARGETS game_engine_render_system)
    endif()

    message(STATUS "GameEngine: Render ECS System enabled")
endif()

if(GAMEENGINE_BUILD_CONSOLE_SYSTEM)
    if(NOT GAMEENGINE_BUILD_RENDER_SYSTEM)
        message(FATAL_ERROR "GAMEENGINE_BUILD_CONSOLE_SYSTEM requires GAMEENGINE_BUILD_RENDER_SYSTEM=ON")
    endif()

    add_library(game_engine_console_system STATIC
        src/Systems/DeveloperConsoleSystem/DeveloperConsoleSystem.cpp
    )

    target_include_directories(game_engine_console_system
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(game_engine_console_system PUBLIC
        GameEngine::core
        game_engine_render_system
    )

    add_library(GameEngine::console_system ALIAS game_engine_console_system)

    target_compile_definitions(game_engine_console_system PUBLIC GAMEENGINE_HAS_CONSOLE_SYSTEM)
    target_compile_definitions(game_engine_core INTERFACE GAMEENGINE_HAS_CONSOLE_SYSTEM)

    if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        list(APPEND INSTALL_TARGETS game_engine_console_system)
    endif()

    message(STATUS "GameEngine: Developer Console ECS System enabled")
endif()


include(GNUInstallDirs)

install(TARGETS ${INSTALL_TARGETS}
    EXPORT GameEngineTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY src/Core src/Utils src/Systems src/Scripting
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GameEngine
    FILES_MATCHING PATTERN "*.hpp"
)

# Install LuaBridge headers if scripting is enabled
if(GAMEENGINE_BUILD_SCRIPTING AND TARGET LuaBridge)
    install(DIRECTORY ${LuaBridge3_SOURCE_DIR}/Source/LuaBridge
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/LuaBridge
        FILES_MATCHING PATTERN "*.h"
    )
endif()

install(EXPORT GameEngineTargets
    FILE GameEngineTargets.cmake
    NAMESPACE GameEngine::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GameEngine
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GameEngineConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/GameEngineConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GameEngine
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/GameEngineConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/GameEngineConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/GameEngineConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GameEngine
)

message(STATUS "GameEngine: Version ${PROJECT_VERSION}")
message(STATUS "GameEngine: Core (header-only) configured")
