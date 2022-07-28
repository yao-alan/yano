#ifndef XKEYCODES_H
#define XKEYCODES_H

#include <stdint.h>

enum XKEYCODE {
    ESC = 9,
    ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, ZERO, MINUS, EQUAL, BACKSPACE,
    TAB, Q, W, E, R, T, Y, U, I, O, P, BRACKET_L, BRACKET_R, RETURN,
    CAPS, A, S, D, F, G, H, J, K, L, SEMICOLON, APOSTROPHE,
    TILDE, SHIFT_L, BACKSLASH,
    Z, X, C, V, B, N, M, COMMA, PERIOD, SLASH, SHIFT_R,
    ALT_L = 64, SPACE, CTRL_L,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10,
    NUM_LOCK, SCROLL_LOCK,
    CTRL_R = 105,
    PRINT = 107,
    ALT_R,
    LF, HOME, UP, PRIOR, LEFT, RIGHT, END, DOWN, NEXT, INS, DEL,
    PLUS_MINUS = 126,
    SUPER_L = 133, SUPER_R, MENU, CANCEL, REDO, UNDO = 139,
    PAREN_L = 187, PAREN_R
};

class XToAscii
{
    public:
        XToAscii() {
            m_table[ONE][0]   = '1';
            m_table[ONE][1]   = '!';
            m_table[TWO][0]   = '2';
            m_table[TWO][1]   = '@';
            m_table[THREE][0] = '3';
            m_table[THREE][1] = '#';
            m_table[FOUR][0]  = '4';
            m_table[FOUR][1]  = '$';
            m_table[FIVE][0]  = '5';
            m_table[FIVE][1]  = '%';
            m_table[SIX][0]   = '6';
            m_table[SIX][1]   = '^';
            m_table[SEVEN][0] = '7';
            m_table[SEVEN][1] = '&';
            m_table[EIGHT][0] = '8';
            m_table[EIGHT][1] = '*';
            m_table[NINE][0]  = '9';
            m_table[NINE][1]  = '(';
            m_table[ZERO][0]  = '0';
            m_table[ZERO][1]  = ')';
            m_table[MINUS][0] = '-';
            m_table[MINUS][1] = '_';
            m_table[EQUAL][0] = '=';
            m_table[EQUAL][1] = '+';
            m_table[Q][0]     = 'q';
            m_table[Q][1]     = 'Q';
            m_table[W][0]     = 'w';
            m_table[W][1]     = 'W';
            m_table[E][0]     = 'e';
            m_table[E][1]     = 'E';
            m_table[R][0]     = 'r';
            m_table[R][1]     = 'R';
            m_table[T][0]     = 't';
            m_table[T][1]     = 'T';
            m_table[Y][0]     = 'y';
            m_table[Y][1]     = 'Y';
            m_table[U][0]     = 'u';
            m_table[U][1]     = 'U';
            m_table[I][0]     = 'i';
            m_table[I][1]     = 'I';
            m_table[O][0]     = 'o';
            m_table[O][1]     = 'O';
            m_table[P][0]     = 'p';
            m_table[P][1]     = 'P';
            m_table[BRACKET_L][0] = '[';
            m_table[BRACKET_L][1] = '{';
            m_table[BRACKET_R][0] = ']';
            m_table[BRACKET_R][1] = '}';
            m_table[RETURN][0] = '\n';
            m_table[A][0]      = 'a';
            m_table[A][1]      = 'A';
            m_table[S][0]      = 's';
            m_table[S][1]      = 'S';
            m_table[D][0]      = 'd';
            m_table[D][1]      = 'D';
            m_table[F][0]      = 'f';
            m_table[F][1]      = 'F';
            m_table[G][0]      = 'g';
            m_table[G][1]      = 'G';
            m_table[H][0]      = 'h';
            m_table[H][1]      = 'H';
            m_table[J][0]      = 'j';
            m_table[J][1]      = 'J';
            m_table[K][0]      = 'k';
            m_table[K][1]      = 'K';
            m_table[L][0]      = 'l';
            m_table[L][1]      = 'L';
            m_table[SEMICOLON][0] = ';';
            m_table[SEMICOLON][1] = ':';
            m_table[APOSTROPHE][0] = '\'';
            m_table[APOSTROPHE][1] = '"'; 
            m_table[TILDE][0]  = '`';
            m_table[TILDE][1]  = '~';
            m_table[BACKSLASH][0] = '\\';
            m_table[BACKSLASH][1] = '|';
            m_table[Z][0]      = 'z';
            m_table[Z][1]      = 'Z';
            m_table[X][0]      = 'x';
            m_table[X][1]      = 'X';
            m_table[C][0]      = 'c';
            m_table[C][1]      = 'C';
            m_table[V][0]      = 'v';
            m_table[V][1]      = 'V';
            m_table[B][0]      = 'b';
            m_table[B][1]      = 'B';
            m_table[N][0]      = 'n';
            m_table[N][1]      = 'N';
            m_table[M][0]      = 'm';
            m_table[M][1]      = 'M';
            m_table[COMMA][0]  = ',';
            m_table[COMMA][1]  = '<';
            m_table[PERIOD][0] = '.';
            m_table[PERIOD][1] = '>';
            m_table[SLASH][0]  = '/';
            m_table[SLASH][1]  = '?';
            m_table[SPACE][0]  = 0;
        }

        char convert(int kc, int modifier) {
            return m_table[kc][modifier];
        }
    private:
        char m_table[256][6];
};

#endif