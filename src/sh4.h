
#undef KEY_CHAR_0
#undef KEY_CHAR_1
#undef KEY_CHAR_2
#undef KEY_CHAR_3
#undef KEY_CHAR_4
#undef KEY_CHAR_5
#undef KEY_CHAR_6
#undef KEY_CHAR_7
#undef KEY_CHAR_8
#undef KEY_CHAR_9
#undef KEY_CHAR_DP
#undef KEY_CHAR_EXP
#undef KEY_CHAR_PMINUS
#undef KEY_CHAR_PLUS
#undef KEY_CHAR_MINUS
#undef KEY_CHAR_MULT
#undef KEY_CHAR_DIV
#undef KEY_CHAR_FRAC
#undef KEY_CHAR_LPAR
#undef KEY_CHAR_RPAR
#undef KEY_CHAR_COMMA
#undef KEY_CHAR_STORE
#undef KEY_CHAR_LOG
#undef KEY_CHAR_LN
#undef KEY_CHAR_SIN
#undef KEY_CHAR_COS
#undef KEY_CHAR_TAN
#undef KEY_CHAR_SQUARE
#undef KEY_CHAR_POW
#undef KEY_CTRL_EXE
#undef KEY_CTRL_DEL
#undef KEY_CTRL_AC
#undef KEY_CTRL_FD
#undef KEY_CTRL_EXIT
#undef KEY_CTRL_SHIFT
#undef KEY_CTRL_ALPHA
#undef KEY_CTRL_OPTN
#undef KEY_CTRL_VARS
#undef KEY_CTRL_UP
#undef KEY_CTRL_DOWN
#undef KEY_CTRL_LEFT
#undef KEY_CTRL_RIGHT
#undef KEY_CTRL_F1
#undef KEY_CTRL_F2
#undef KEY_CTRL_F3
#undef KEY_CTRL_F4
#undef KEY_CTRL_F5
#undef KEY_CTRL_F6
#undef KEY_CTRL_MENU

#define KEY_CHAR_0 71
#define KEY_CHAR_1 72
#define KEY_CHAR_2 62
#define KEY_CHAR_3 52
#define KEY_CHAR_4 73
#define KEY_CHAR_5 63
#define KEY_CHAR_6 53
#define KEY_CHAR_7 74
#define KEY_CHAR_8 64
#define KEY_CHAR_9 54
#define KEY_CHAR_DP 61
#define KEY_CHAR_EXP 51
#define KEY_CHAR_PMINUS 41
#define KEY_CHAR_PLUS 42
#define KEY_CHAR_MINUS 32
#define KEY_CHAR_MULT 43
#define KEY_CHAR_DIV 33
#define KEY_CHAR_FRAC 75
#define KEY_CHAR_LPAR 55
#define KEY_CHAR_RPAR 45
#define KEY_CHAR_COMMA 35
#define KEY_CHAR_STORE 25
#define KEY_CHAR_LOG 66
#define KEY_CHAR_LN 56
#define KEY_CHAR_SIN 46
#define KEY_CHAR_COS 36
#define KEY_CHAR_TAN 26
#define KEY_CHAR_SQUARE 67
#define KEY_CHAR_POW 57
#define KEY_CTRL_EXE 31
#define KEY_CTRL_DEL 44
#define KEY_CTRL_AC 32
#define KEY_CTRL_FD 65
#define KEY_CTRL_EXIT 47
#define KEY_CTRL_SHIFT 78
#define KEY_CTRL_ALPHA 77
#define KEY_CTRL_OPTN 68
#define KEY_CTRL_VARS 58
#define KEY_CTRL_UP 28
#define KEY_CTRL_DOWN 37
#define KEY_CTRL_LEFT 38
#define KEY_CTRL_RIGHT 27
#define KEY_CTRL_F1 79
#define KEY_CTRL_F2 69
#define KEY_CTRL_F3 59
#define KEY_CTRL_F4 49
#define KEY_CTRL_F5 39
#define KEY_CTRL_F6 29
#define KEY_CTRL_MENU 48

#define IsKeyDown(x) KeyDown(x)
#define IsKeyUp(x) !KeyDown(x)
#define GetKey(x) GetKeyMod(x)
