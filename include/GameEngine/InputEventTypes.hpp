/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** InputEventTypes
*/

#pragma once

#include <cstdint>
#include <string>

namespace GameEngine
{
    enum class KeyCode : std::uint32_t {
        Unknown = 0,

        // Letters
        A = 'a', B = 'b', C = 'c', D = 'd', E = 'e', F = 'f', G = 'g',
        H = 'h', I = 'i', J = 'j', K = 'k', L = 'l', M = 'm', N = 'n',
        O = 'o', P = 'p', Q = 'q', R = 'r', S = 's', T = 't', U = 'u',
        V = 'v', W = 'w', X = 'x', Y = 'y', Z = 'z',

        // Numbers
        Num0 = '0', Num1 = '1', Num2 = '2', Num3 = '3', Num4 = '4',
        Num5 = '5', Num6 = '6', Num7 = '7', Num8 = '8', Num9 = '9',

        // Function keys
        F1 = 0x4000003A, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

        // Arrow keys
        Right = 0x4000004F,
        Left = 0x40000050,
        Down = 0x40000051,
        Up = 0x40000052,

        // Special keys
        Return = '\r',
        Escape = '\033',
        Backspace = '\b',
        Tab = '\t',
        Space = ' ',
        Delete = 0x7F,

        // Modifiers
        CapsLock = 0x40000039,
        LShift = 0x400000E1,
        RShift = 0x400000E5,
        LCtrl = 0x400000E0,
        RCtrl = 0x400000E4,
        LAlt = 0x400000E2,
        RAlt = 0x400000E6,

        // Other
        PageUp = 0x4000004B,
        PageDown = 0x4000004E,
        Home = 0x4000004A,
        End = 0x4000004D,
        Insert = 0x40000049,
    };

    enum class MouseButton : std::uint8_t {
        Unknown = 0,
        Left = 1,
        Middle = 2,
        Right = 3,
    };

    struct KeyDownEvent {
        KeyCode key;
        std::uint32_t rawKey;
        bool repeat;

        KeyDownEvent(KeyCode keyCode, std::uint32_t raw, bool isRepeat = false)
            : key(keyCode), rawKey(raw), repeat(isRepeat)
        {}
    };

    struct KeyUpEvent {
        KeyCode key;
        std::uint32_t rawKey;

        KeyUpEvent(KeyCode keyCode, std::uint32_t raw)
            : key(keyCode), rawKey(raw)
        {}
    };

    struct MouseButtonDownEvent {
        MouseButton button;
        float x;
        float y;
        std::uint8_t clicks;

        MouseButtonDownEvent(MouseButton btn, float posX, float posY, std::uint8_t numClicks = 1)
            : button(btn), x(posX), y(posY), clicks(numClicks)
        {}
    };

    struct MouseButtonUpEvent {
        MouseButton button;
        float x;
        float y;
        std::uint8_t clicks;

        MouseButtonUpEvent(MouseButton btn, float posX, float posY, std::uint8_t numClicks = 1)
            : button(btn), x(posX), y(posY), clicks(numClicks)
        {}
    };

    struct MouseMotionEvent {
        float x;
        float y;
        float deltaX;
        float deltaY;
        bool leftButton;
        bool rightButton;
        bool middleButton;

        MouseMotionEvent(float posX, float posY, float relX, float relY,
                        bool left = false, bool right = false, bool middle = false)
            : x(posX), y(posY), deltaX(relX), deltaY(relY),
              leftButton(left), rightButton(right), middleButton(middle)
        {}
    };

    struct MouseWheelEvent {
        float scrollX;
        float scrollY;

        MouseWheelEvent(float x, float y) : scrollX(x), scrollY(y) {}
    };

    struct QuitEvent {
        QuitEvent() = default;
    };

    struct WindowResizedEvent {
        int width;
        int height;

        WindowResizedEvent(int w, int h) : width(w), height(h) {}
    };

    struct WindowFocusGainedEvent {
        WindowFocusGainedEvent() = default;
    };

    struct WindowFocusLostEvent {
        WindowFocusLostEvent() = default;
    };

} // namespace GameEngine
