#pragma once

#include "common/EnumClassOperators.hpp"

namespace RR::Platform::Input
{
    enum class Key : uint32_t
    {
        // Key codes 0..255 are reserved for ASCII codes.
        Space = ' ',
        Apostrophe = '\'',
        Comma = ',',
        Minus = '-',
        Period = '.',
        Slash = '/',
        Key0 = '0',
        Key1 = '1',
        Key2 = '2',
        Key3 = '3',
        Key4 = '4',
        Key5 = '5',
        Key6 = '6',
        Key7 = '7',
        Key8 = '8',
        Key9 = '9',
        Semicolon = ';',
        Equal = '=',
        A = 'A',
        B = 'B',
        C = 'C',
        D = 'D',
        E = 'E',
        F = 'F',
        G = 'G',
        H = 'H',
        I = 'I',
        J = 'J',
        K = 'K',
        L = 'L',
        M = 'M',
        N = 'N',
        O = 'O',
        P = 'P',
        Q = 'Q',
        R = 'R',
        S = 'S',
        T = 'T',
        U = 'U',
        V = 'V',
        W = 'W',
        X = 'X',
        Y = 'Y',
        Z = 'Z',
        LeftBracket = '[',
        Backslash = '\\',
        RightBracket = ']',
        GraveAccent = '`',

        // Special keys start at key code 256.
        Escape = 256,
        Tab,
        Enter,
        Backspace,
        Insert,
        Delete,
        Right,
        Left,
        Down,
        Up,
        PageUp,
        PageDown,
        Home,
        End,
        CapsLock,
        ScrollLock,
        NumLock,
        PrintScreen,
        Pause,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        Keypad0,
        Keypad1,
        Keypad2,
        Keypad3,
        Keypad4,
        Keypad5,
        Keypad6,
        Keypad7,
        Keypad8,
        Keypad9,
        KeypadDecimal,
        KeypadDivide,
        KeypadMultiply,
        KeypadSubtract,
        KeypadAdd,
        KeypadEnter,
        KeypadEqual,
        LeftShift,
        LeftControl,
        LeftAlt,
        LeftSuper, // Windows key on windows
        RightShift,
        RightControl,
        RightAlt,
        RightSuper, // Windows key on windows
        Menu,
        Unknown, // Any unknown key code

        Count,
    };

    enum class KeyAction : uint32_t
    {
        Press,
        Release,
        Repeat
    };

    enum class ModifierFlag : uint32_t
    {
        None = 0,
        Shift = 1 << 0,
        Control = 1 << 1,
        Alt = 1 << 2,
        Super = 1 << 3,
        CapsLock = 1 << 4,
        NumLock = 1 << 5,
    };
    ENUM_CLASS_BITWISE_OPS(ModifierFlag)

    enum class MouseButton : uint32_t
    {
        Left,
        Right,
        Middle,
        Count
    };
}