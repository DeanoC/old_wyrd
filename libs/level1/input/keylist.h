
#ifndef KEYTABLE_START
#define KEYTABLE_START enum class Key : uint16_t {
#endif

#ifndef KEYTABLE_END
#define KEYTABLE_END };
#endif

#ifndef VIRTUALKEY_KEY_MAP
#define VIRTUALKEY_KEY_MAP(c) KT_##c,
#endif

#ifndef VIRTUALKEY_KEY_MAP2
#define VIRTUALKEY_KEY_MAP2(c, w, m) KT_##c,
#endif

#ifndef VIRTUALKEY_KEY_MAP_WINONLY
#define VIRTUALKEY_KEY_MAP_WINONLY(c, w) KT_##c,
#endif

#ifndef NORMALKEY_KEY_MAP
#define NORMALKEY_KEY_MAP(c, d) KT_##c,
#endif

KEYTABLE_START
NORMALKEY_KEY_MAP(1, '1')
NORMALKEY_KEY_MAP(2, '2')
NORMALKEY_KEY_MAP(3, '3')
NORMALKEY_KEY_MAP(4, '4')
NORMALKEY_KEY_MAP(5, '5')
NORMALKEY_KEY_MAP(6, '6')
NORMALKEY_KEY_MAP(7, '7')
NORMALKEY_KEY_MAP(8, '8')
NORMALKEY_KEY_MAP(9, '9')
NORMALKEY_KEY_MAP(0, '0')
NORMALKEY_KEY_MAP(Q, 'Q')
NORMALKEY_KEY_MAP(W, 'W')
NORMALKEY_KEY_MAP(E, 'E')
NORMALKEY_KEY_MAP(R, 'R')
NORMALKEY_KEY_MAP(T, 'T')
NORMALKEY_KEY_MAP(Y, 'Y')
NORMALKEY_KEY_MAP(U, 'U')
NORMALKEY_KEY_MAP(I, 'I')
NORMALKEY_KEY_MAP(O, 'O')
NORMALKEY_KEY_MAP(P, 'P')
NORMALKEY_KEY_MAP(A, 'A')
NORMALKEY_KEY_MAP(S, 'S')
NORMALKEY_KEY_MAP(D, 'D')
NORMALKEY_KEY_MAP(F, 'F')
NORMALKEY_KEY_MAP(G, 'G')
NORMALKEY_KEY_MAP(H, 'H')
NORMALKEY_KEY_MAP(J, 'J')
NORMALKEY_KEY_MAP(K, 'K')
NORMALKEY_KEY_MAP(L, 'L')
NORMALKEY_KEY_MAP(Z, 'Z')
NORMALKEY_KEY_MAP(X, 'X')
NORMALKEY_KEY_MAP(C, 'C')
NORMALKEY_KEY_MAP(V, 'V')
NORMALKEY_KEY_MAP(B, 'B')
NORMALKEY_KEY_MAP(N, 'N')
NORMALKEY_KEY_MAP(M, 'M')

VIRTUALKEY_KEY_MAP(ESCAPE)
VIRTUALKEY_KEY_MAP(TAB)
VIRTUALKEY_KEY_MAP(RETURN)

VIRTUALKEY_KEY_MAP2(MINUS, OEM_MINUS, ANSI_MINUS)
VIRTUALKEY_KEY_MAP2(EQUALS, OEM_PLUS, ANSI_EQUAL)
VIRTUALKEY_KEY_MAP2(BACK, BACK, DELETE )
VIRTUALKEY_KEY_MAP2(SEMICOLON, OEM_1, ANSI_SEMICOLON)
VIRTUALKEY_KEY_MAP2(GRAVE, OEM_102, ANSI_GRAVE)
VIRTUALKEY_KEY_MAP2(APOSTROPHE, OEM_3, ANSI_QUOTE)
VIRTUALKEY_KEY_MAP2(LBRACKET, OEM_4, ANSI_LEFTBRACKET)
VIRTUALKEY_KEY_MAP2(BACKSLASH, OEM_5, ANSI_BACKSLASH)
VIRTUALKEY_KEY_MAP2(RBRACKET, OEM_5, ANSI_RIGHTBRACKET)
VIRTUALKEY_KEY_MAP2(COMMA, OEM_COMMA, ANSI_COMMA)
VIRTUALKEY_KEY_MAP2(PEROID, OEM_PERIOD, ANSI_PERIOD)
VIRTUALKEY_KEY_MAP2(CAPITAL, CAPITAL, CAPSLOCK)

VIRTUALKEY_KEY_MAP2(PAGEUP, PRIOR, PAGEUP)
VIRTUALKEY_KEY_MAP2(PAGEDOWN, NEXT, PAGEDOWN)
VIRTUALKEY_KEY_MAP(HOME)
VIRTUALKEY_KEY_MAP(END)
VIRTUALKEY_KEY_MAP2(INSERT, INSERT, HELP)
VIRTUALKEY_KEY_MAP2(DELETE, DELETE, FORWARDDELETE)

VIRTUALKEY_KEY_MAP2(LCONTROL, LCONTROL, CONTROL)
VIRTUALKEY_KEY_MAP2(RCONTROL, RCONTROL, RIGHTCONTROL)
VIRTUALKEY_KEY_MAP2(LSHIFT, LSHIFT, SHIFT)
VIRTUALKEY_KEY_MAP2(RSHIFT, RSHIFT, RIGHTSHIFT)
VIRTUALKEY_KEY_MAP2(LALT, LMENU, COMMAND)
VIRTUALKEY_KEY_MAP2(LWIN, LWIN, OPTION)
VIRTUALKEY_KEY_MAP2(RWIN, RWIN, RIGHTOPTION)

VIRTUALKEY_KEY_MAP(SPACE)
VIRTUALKEY_KEY_MAP(F1)
VIRTUALKEY_KEY_MAP(F2)
VIRTUALKEY_KEY_MAP(F3)
VIRTUALKEY_KEY_MAP(F4)
VIRTUALKEY_KEY_MAP(F5)
VIRTUALKEY_KEY_MAP(F6)
VIRTUALKEY_KEY_MAP(F7)
VIRTUALKEY_KEY_MAP(F8)
VIRTUALKEY_KEY_MAP(F9)
VIRTUALKEY_KEY_MAP(F10)
VIRTUALKEY_KEY_MAP(F11)
VIRTUALKEY_KEY_MAP(F12)

VIRTUALKEY_KEY_MAP2(SUBTRACT, SUBTRACT, ANSI_KEYPADMINUS)
VIRTUALKEY_KEY_MAP2(ADD, ADD, ANSI_KEYPADPLUS)
VIRTUALKEY_KEY_MAP2(DIVIDE, DIVIDE, ANSI_KEYPADDIVIDE)
VIRTUALKEY_KEY_MAP2(MULTIPLY, MULTIPLY, ANSI_KEYPADMULTIPLY)
VIRTUALKEY_KEY_MAP2(DECIMAL, DECIMAL, ANSI_KEYPADDECIMAL)
VIRTUALKEY_KEY_MAP2(NUMLOCK, NUMLOCK, ANSI_KEYPADCLEAR)
VIRTUALKEY_KEY_MAP2(PRINTSCREEN, SNAPSHOT, F13)
VIRTUALKEY_KEY_MAP2(SCROLL, SCROLL, F14)
VIRTUALKEY_KEY_MAP2(PAUSE, PAUSE, F15)


VIRTUALKEY_KEY_MAP2(NUMPAD7, NUMPAD7, ANSI_KEYPAD7)
VIRTUALKEY_KEY_MAP2(NUMPAD8, NUMPAD8, ANSI_KEYPAD8)
VIRTUALKEY_KEY_MAP2(NUMPAD9, NUMPAD9, ANSI_KEYPAD9)
VIRTUALKEY_KEY_MAP2(NUMPAD4, NUMPAD4, ANSI_KEYPAD4)
VIRTUALKEY_KEY_MAP2(NUMPAD5, NUMPAD5, ANSI_KEYPAD5)
VIRTUALKEY_KEY_MAP2(NUMPAD6, NUMPAD6, ANSI_KEYPAD6)
VIRTUALKEY_KEY_MAP2(NUMPAD1, NUMPAD1, ANSI_KEYPAD1)
VIRTUALKEY_KEY_MAP2(NUMPAD2, NUMPAD2, ANSI_KEYPAD2)
VIRTUALKEY_KEY_MAP2(NUMPAD3, NUMPAD3, ANSI_KEYPAD3)
VIRTUALKEY_KEY_MAP2(NUMPAD0, NUMPAD0, ANSI_KEYPAD0)

VIRTUALKEY_KEY_MAP2(LEFT, LEFT, LEFTARROW)    	/* LeftArrow on arrow keypad */
VIRTUALKEY_KEY_MAP2(RIGHT,RIGHT, RIGHTARROW)    /* RightArrow on arrow keypad */
VIRTUALKEY_KEY_MAP2(UP, UP, UPARROW)    		/* UpArrow on arrow keypad */
VIRTUALKEY_KEY_MAP2(DOWN, DOWN, DOWNARROW)    	/* DownArrow on arrow keypad */

KEYTABLE_END

#undef VIRTUALKEY_KEY_MAP_WINONLY
#undef VIRTUALKEY_KEY_MAP
#undef VIRTUALKEY_KEY_MAP2
#undef NORMALKEY_KEY_MAP
#undef KEYTABLE_START
#undef KEYTABLE_END