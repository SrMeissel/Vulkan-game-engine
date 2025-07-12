#pragma once

#include <unordered_map>

// Generic key codes to be used as cross-platform keys
enum class Key {
    KEY_UNKNOWN = 0,
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G,
    KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N,
    KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U,
    KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
    KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_ESCAPE, KEY_ENTER, KEY_TAB, KEY_BACKSPACE, KEY_INSERT, KEY_DELETE,
    KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP,
    KEY_PAGE_UP, KEY_PAGE_DOWN, KEY_HOME, KEY_END,
    KEY_CAPS_LOCK, KEY_SCROLL_LOCK, KEY_NUM_LOCK,
    KEY_PRINT_SCREEN, KEY_PAUSE,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
    KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_SPACE, KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT, KEY_LEFT_CTRL, KEY_RIGHT_CTRL,
    KEY_LEFT_ALT, KEY_RIGHT_ALT, KEY_MENU,
    KeyCount
};

// #include <GLFW/glfw3.h>
// #include <unordered_map>

// static const std::unordered_map<Key, int> GenericToGLFWKey = {
//     {Key::KEY_A, GLFW_KEY_A},
//     {Key::KEY_B, GLFW_KEY_B},
//     {Key::KEY_C, GLFW_KEY_C},
//     {Key::KEY_D, GLFW_KEY_D},
//     {Key::KEY_E, GLFW_KEY_E},
//     {Key::KEY_F, GLFW_KEY_F},
//     {Key::KEY_G, GLFW_KEY_G},
//     {Key::KEY_H, GLFW_KEY_H},
//     {Key::KEY_I, GLFW_KEY_I},
//     {Key::KEY_J, GLFW_KEY_J},
//     {Key::KEY_K, GLFW_KEY_K},
//     {Key::KEY_L, GLFW_KEY_L},
//     {Key::KEY_M, GLFW_KEY_M},
//     {Key::KEY_N, GLFW_KEY_N},
//     {Key::KEY_O, GLFW_KEY_O},
//     {Key::KEY_P, GLFW_KEY_P},
//     {Key::KEY_Q, GLFW_KEY_Q},
//     {Key::KEY_R, GLFW_KEY_R},
//     {Key::KEY_S, GLFW_KEY_S},
//     {Key::KEY_T, GLFW_KEY_T},
//     {Key::KEY_U, GLFW_KEY_U},
//     {Key::KEY_V, GLFW_KEY_V},
//     {Key::KEY_W, GLFW_KEY_W},
//     {Key::KEY_X, GLFW_KEY_X},
//     {Key::KEY_Y, GLFW_KEY_Y},
//     {Key::KEY_Z, GLFW_KEY_Z},
//     {Key::KEY_0, GLFW_KEY_0},
//     {Key::KEY_1, GLFW_KEY_1},
//     {Key::KEY_2, GLFW_KEY_2},
//     {Key::KEY_3, GLFW_KEY_3},
//     {Key::KEY_4, GLFW_KEY_4},
//     {Key::KEY_5, GLFW_KEY_5},
//     {Key::KEY_6, GLFW_KEY_6},
//     {Key::KEY_7, GLFW_KEY_7},
//     {Key::KEY_8, GLFW_KEY_8},
//     {Key::KEY_9, GLFW_KEY_9},
//     {Key::KEY_ESCAPE, GLFW_KEY_ESCAPE},
//     {Key::KEY_ENTER, GLFW_KEY_ENTER},
//     {Key::KEY_TAB, GLFW_KEY_TAB},
//     {Key::KEY_BACKSPACE, GLFW_KEY_BACKSPACE},
//     {Key::KEY_INSERT, GLFW_KEY_INSERT},
//     {Key::KEY_DELETE, GLFW_KEY_DELETE},
//     {Key::KEY_RIGHT, GLFW_KEY_RIGHT},
//     {Key::KEY_LEFT, GLFW_KEY_LEFT},
//     {Key::KEY_DOWN, GLFW_KEY_DOWN},
//     {Key::KEY_UP, GLFW_KEY_UP},
//     {Key::KEY_PAGE_UP, GLFW_KEY_PAGE_UP},
//     {Key::KEY_PAGE_DOWN, GLFW_KEY_PAGE_DOWN},
//     {Key::KEY_HOME, GLFW_KEY_HOME},
//     {Key::KEY_END, GLFW_KEY_END},
//     {Key::KEY_CAPS_LOCK, GLFW_KEY_CAPS_LOCK},
//     {Key::KEY_SCROLL_LOCK, GLFW_KEY_SCROLL_LOCK},
//     {Key::KEY_NUM_LOCK, GLFW_KEY_NUM_LOCK},
//     {Key::KEY_PRINT_SCREEN, GLFW_KEY_PRINT_SCREEN},
//     {Key::KEY_PAUSE, GLFW_KEY_PAUSE},
//     {Key::KEY_F1, GLFW_KEY_F1},
//     {Key::KEY_F2, GLFW_KEY_F2},
//     {Key::KEY_F3, GLFW_KEY_F3},
//     {Key::KEY_F4, GLFW_KEY_F4},
//     {Key::KEY_F5, GLFW_KEY_F5},
//     {Key::KEY_F6, GLFW_KEY_F6},
//     {Key::KEY_F7, GLFW_KEY_F7},
//     {Key::KEY_F8, GLFW_KEY_F8},
//     {Key::KEY_F9, GLFW_KEY_F9},
//     {Key::KEY_F10, GLFW_KEY_F10},
//     {Key::KEY_F11, GLFW_KEY_F11},
//     {Key::KEY_F12, GLFW_KEY_F12},
//     {Key::KEY_SPACE, GLFW_KEY_SPACE},
//     {Key::KEY_LEFT_SHIFT, GLFW_KEY_LEFT_SHIFT},
//     {Key::KEY_RIGHT_SHIFT, GLFW_KEY_RIGHT_SHIFT},
//     {Key::KEY_LEFT_CTRL, GLFW_KEY_LEFT_CONTROL},
//     {Key::KEY_RIGHT_CTRL, GLFW_KEY_RIGHT_CONTROL},
//     {Key::KEY_LEFT_ALT, GLFW_KEY_LEFT_ALT},
//     {Key::KEY_RIGHT_ALT, GLFW_KEY_RIGHT_ALT},
//     {Key::KEY_MENU, GLFW_KEY_MENU}
// };

#ifndef VK_ESCAPE
#define VK_ESCAPE        0x1B
#endif
#ifndef VK_RETURN
#define VK_RETURN        0x0D
#endif
#ifndef VK_TAB
#define VK_TAB           0x09
#endif
#ifndef VK_BACK
#define VK_BACK          0x08
#endif
#ifndef VK_INSERT
#define VK_INSERT        0x2D
#endif
#ifndef VK_DELETE
#define VK_DELETE        0x2E
#endif
#ifndef VK_RIGHT
#define VK_RIGHT         0x27
#endif
#ifndef VK_LEFT
#define VK_LEFT          0x25
#endif
#ifndef VK_DOWN
#define VK_DOWN          0x28
#endif
#ifndef VK_UP
#define VK_UP            0x26
#endif
#ifndef VK_PRIOR
#define VK_PRIOR         0x21
#endif
#ifndef VK_NEXT
#define VK_NEXT          0x22
#endif
#ifndef VK_HOME
#define VK_HOME          0x24
#endif
#ifndef VK_END
#define VK_END           0x23
#endif
#ifndef VK_CAPITAL
#define VK_CAPITAL       0x14
#endif
#ifndef VK_SCROLL
#define VK_SCROLL        0x91
#endif
#ifndef VK_NUMLOCK
#define VK_NUMLOCK       0x90
#endif
#ifndef VK_SNAPSHOT
#define VK_SNAPSHOT      0x2C
#endif
#ifndef VK_PAUSE
#define VK_PAUSE         0x13
#endif
#ifndef VK_F1
#define VK_F1            0x70
#endif
#ifndef VK_F2
#define VK_F2            0x71
#endif
#ifndef VK_F3
#define VK_F3            0x72
#endif
#ifndef VK_F4
#define VK_F4            0x73
#endif
#ifndef VK_F5
#define VK_F5            0x74
#endif
#ifndef VK_F6
#define VK_F6            0x75
#endif
#ifndef VK_F7
#define VK_F7            0x76
#endif
#ifndef VK_F8
#define VK_F8            0x77
#endif
#ifndef VK_F9
#define VK_F9            0x78
#endif
#ifndef VK_F10
#define VK_F10           0x79
#endif
#ifndef VK_F11
#define VK_F11           0x7A
#endif
#ifndef VK_F12
#define VK_F12           0x7B
#endif
#ifndef VK_SPACE
#define VK_SPACE         0x20
#endif
#ifndef VK_LSHIFT
#define VK_LSHIFT        0xA0
#endif
#ifndef VK_RSHIFT
#define VK_RSHIFT        0xA1
#endif
#ifndef VK_SHIFT
#define VK_SHIFT        0x10
#endif
#ifndef VK_LCONTROL
#define VK_LCONTROL      0xA2
#endif
#ifndef VK_RCONTROL
#define VK_RCONTROL      0xA3
#endif
#ifndef VK_LMENU
#define VK_LMENU         0xA4
#endif
#ifndef VK_RMENU
#define VK_RMENU         0xA5
#endif
#ifndef VK_MENU
#define VK_MENU          0x12
#endif

static const std::unordered_map<int, Key> Win32ToGenericKey = {
    {'A', Key::KEY_A},
    {'B', Key::KEY_B},
    {'C', Key::KEY_C},
    {'D', Key::KEY_D},
    {'E', Key::KEY_E},
    {'F', Key::KEY_F},
    {'G', Key::KEY_G},
    {'H', Key::KEY_H},
    {'I', Key::KEY_I},
    {'J', Key::KEY_J},
    {'K', Key::KEY_K},
    {'L', Key::KEY_L},
    {'M', Key::KEY_M},
    {'N', Key::KEY_N},
    {'O', Key::KEY_O},
    {'P', Key::KEY_P},
    {'Q', Key::KEY_Q},
    {'R', Key::KEY_R},
    {'S', Key::KEY_S},
    {'T', Key::KEY_T},
    {'U', Key::KEY_U},
    {'V', Key::KEY_V},
    {'W', Key::KEY_W},
    {'X', Key::KEY_X},
    {'Y', Key::KEY_Y},
    {'Z', Key::KEY_Z},
    {'0', Key::KEY_0},
    {'1', Key::KEY_1},
    {'2', Key::KEY_2},
    {'3', Key::KEY_3},
    {'4', Key::KEY_4},
    {'5', Key::KEY_5},
    {'6', Key::KEY_6},
    {'7', Key::KEY_7},
    {'8', Key::KEY_8},
    {'9', Key::KEY_9},
    {VK_ESCAPE, Key::KEY_ESCAPE},
    {VK_RETURN, Key::KEY_ENTER},
    {VK_TAB, Key::KEY_TAB},
    {VK_BACK, Key::KEY_BACKSPACE},
    {VK_INSERT, Key::KEY_INSERT},
    {VK_DELETE, Key::KEY_DELETE},
    {VK_RIGHT, Key::KEY_RIGHT},
    {VK_LEFT, Key::KEY_LEFT},
    {VK_DOWN, Key::KEY_DOWN},
    {VK_UP, Key::KEY_UP},
    {VK_PRIOR, Key::KEY_PAGE_UP},
    {VK_NEXT, Key::KEY_PAGE_DOWN},
    {VK_HOME, Key::KEY_HOME},
    {VK_END, Key::KEY_END},
    {VK_CAPITAL, Key::KEY_CAPS_LOCK},
    {VK_SCROLL, Key::KEY_SCROLL_LOCK},
    {VK_NUMLOCK, Key::KEY_NUM_LOCK},
    {VK_SNAPSHOT, Key::KEY_PRINT_SCREEN},
    {VK_PAUSE, Key::KEY_PAUSE},
    {VK_F1, Key::KEY_F1},
    {VK_F2, Key::KEY_F2},
    {VK_F3, Key::KEY_F3},
    {VK_F4, Key::KEY_F4},
    {VK_F5, Key::KEY_F5},
    {VK_F6, Key::KEY_F6},
    {VK_F7, Key::KEY_F7},
    {VK_F8, Key::KEY_F8},
    {VK_F9, Key::KEY_F9},
    {VK_F10, Key::KEY_F10},
    {VK_F11, Key::KEY_F11},
    {VK_F12, Key::KEY_F12},
    {VK_SPACE, Key::KEY_SPACE},
    {VK_SHIFT, Key::KEY_LEFT_SHIFT},
    {VK_LSHIFT, Key::KEY_LEFT_SHIFT},
    {VK_RSHIFT, Key::KEY_RIGHT_SHIFT},
    {VK_LCONTROL, Key::KEY_LEFT_CTRL},
    {VK_RCONTROL, Key::KEY_RIGHT_CTRL},
    {VK_LMENU, Key::KEY_LEFT_ALT},
    {VK_RMENU, Key::KEY_RIGHT_ALT},
    {VK_MENU, Key::KEY_MENU}
};

void buttonState(Key key, bool state);