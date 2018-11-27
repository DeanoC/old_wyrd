#pragma once
#ifndef WYRD_OSX_KEYCODES_H
#define WYRD_OSX_KEYCODES_H

#include "core/core.h"

// this is a bit evil and are virtual keycodes (i.e. hw scan) not nessecarily
// whats physically on the keyboard
/*
 *  Summary:
 *    Virtual keycodes
 *
 *  Discussion:
 *    These constants are the virtual keycodes defined originally in
 *    Inside Mac Volume V, pg. V-191. They identify physical keys on a
 *    keyboard. Those constants with "ANSI" in the name are labeled
 *    according to the key position on an ANSI-standard US keyboard.
 *    For example, kVK_ANSI_A indicates the virtual keycode for the key
 *    with the letter 'A' in the US keyboard layout. Other keyboard
 *    layouts may have the 'A' key label on a different physical key;
 *    in this case, pressing 'A' will generate a different virtual
 *    keycode.
 */

    enum class MacVirtualKeyCode : uint8_t {
	KVK_ANSI_A                    = 0X00,
	KVK_ANSI_S                    = 0X01,
	KVK_ANSI_D                    = 0X02,
	KVK_ANSI_F                    = 0X03,
	KVK_ANSI_H                    = 0X04,
	KVK_ANSI_G                    = 0X05,
	KVK_ANSI_Z                    = 0X06,
	KVK_ANSI_X                    = 0X07,
	KVK_ANSI_C                    = 0X08,
	KVK_ANSI_V                    = 0X09,
	KVK_ANSI_B                    = 0X0B,
	KVK_ANSI_Q                    = 0X0C,
	KVK_ANSI_W                    = 0X0D,
	KVK_ANSI_E                    = 0X0E,
	KVK_ANSI_R                    = 0X0F,
	KVK_ANSI_Y                    = 0X10,
	KVK_ANSI_T                    = 0X11,
	KVK_ANSI_1                    = 0X12,
	KVK_ANSI_2                    = 0X13,
	KVK_ANSI_3                    = 0X14,
	KVK_ANSI_4                    = 0X15,
	KVK_ANSI_6                    = 0X16,
	KVK_ANSI_5                    = 0X17,
	KVK_ANSI_EQUAL                = 0X18,
	KVK_ANSI_9                    = 0X19,
	KVK_ANSI_7                    = 0X1A,
	KVK_ANSI_MINUS                = 0X1B,
	KVK_ANSI_8                    = 0X1C,
	KVK_ANSI_0                    = 0X1D,
	KVK_ANSI_RIGHTBRACKET         = 0X1E,
	KVK_ANSI_O                    = 0X1F,
	KVK_ANSI_U                    = 0X20,
	KVK_ANSI_LEFTBRACKET          = 0X21,
	KVK_ANSI_I                    = 0X22,
	KVK_ANSI_P                    = 0X23,
	KVK_ANSI_L                    = 0X25,
	KVK_ANSI_J                    = 0X26,
	KVK_ANSI_QUOTE                = 0X27,
	KVK_ANSI_K                    = 0X28,
	KVK_ANSI_SEMICOLON            = 0X29,
	KVK_ANSI_BACKSLASH            = 0X2A,
	KVK_ANSI_COMMA                = 0X2B,
	KVK_ANSI_SLASH                = 0X2C,
	KVK_ANSI_N                    = 0X2D,
	KVK_ANSI_M                    = 0X2E,
	KVK_ANSI_PERIOD               = 0X2F,
	KVK_ANSI_GRAVE                = 0X32,
	KVK_ANSI_KEYPADDECIMAL        = 0X41,
	KVK_ANSI_KEYPADMULTIPLY       = 0X43,
	KVK_ANSI_KEYPADPLUS           = 0X45,
	KVK_ANSI_KEYPADCLEAR          = 0X47,
	KVK_ANSI_KEYPADDIVIDE         = 0X4B,
	KVK_ANSI_KEYPADENTER          = 0X4C,
	KVK_ANSI_KEYPADMINUS          = 0X4E,
	KVK_ANSI_KEYPADEQUALS         = 0X51,
	KVK_ANSI_KEYPAD0              = 0X52,
	KVK_ANSI_KEYPAD1              = 0X53,
	KVK_ANSI_KEYPAD2              = 0X54,
	KVK_ANSI_KEYPAD3              = 0X55,
	KVK_ANSI_KEYPAD4              = 0X56,
	KVK_ANSI_KEYPAD5              = 0X57,
	KVK_ANSI_KEYPAD6              = 0X58,
	KVK_ANSI_KEYPAD7              = 0X59,
	KVK_ANSI_KEYPAD8              = 0X5B,
	KVK_ANSI_KEYPAD9              = 0X5C,
	KVK_RETURN                    = 0X24,
	KVK_TAB                       = 0X30,
	KVK_SPACE                     = 0X31,
	KVK_DELETE                    = 0X33,
	KVK_ESCAPE                    = 0X35,
	KVK_COMMAND                   = 0X37,
	KVK_SHIFT                     = 0X38,
	KVK_CAPSLOCK                  = 0X39,
	KVK_OPTION                    = 0X3A,
	KVK_CONTROL                   = 0X3B,
	KVK_RIGHTSHIFT                = 0X3C,
	KVK_RIGHTOPTION               = 0X3D,
	KVK_RIGHTCONTROL              = 0X3E,
	KVK_FUNCTION                  = 0X3F,
	KVK_F17                       = 0X40,
	KVK_VOLUMEUP                  = 0X48,
	KVK_VOLUMEDOWN                = 0X49,
	KVK_MUTE                      = 0X4A,
	KVK_F18                       = 0X4F,
	KVK_F19                       = 0X50,
	KVK_F20                       = 0X5A,
	KVK_F5                        = 0X60,
	KVK_F6                        = 0X61,
	KVK_F7                        = 0X62,
	KVK_F3                        = 0X63,
	KVK_F8                        = 0X64,
	KVK_F9                        = 0X65,
	KVK_F11                       = 0X67,
	KVK_F13                       = 0X69,
	KVK_F16                       = 0X6A,
	KVK_F14                       = 0X6B,
	KVK_F10                       = 0X6D,
	KVK_F12                       = 0X6F,
	KVK_F15                       = 0X71,
	KVK_HELP                      = 0X72,
	KVK_HOME                      = 0X73,
	KVK_PAGEUP                    = 0X74,
	KVK_FORWARDDELETE             = 0X75,
	KVK_F4                        = 0X76,
	KVK_END                       = 0X77,
	KVK_F2                        = 0X78,
	KVK_PAGEDOWN                  = 0X79,
	KVK_F1                        = 0X7A,
	KVK_LEFTARROW                 = 0X7B,
	KVK_RIGHTARROW                = 0X7C,
	KVK_DOWNARROW                 = 0X7D,
	KVK_UPARROW                   = 0X7E
};

#endif //WYRD_OSX_KEYCODES_H
