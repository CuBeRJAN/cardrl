#ifndef _OS_SPECIFIC_H
#define _OS_SPECIFIC_H
// the key_press() function is copied code too
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
void cls();
void game_quit();
int key_press();
#elif defined(__linux__)
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
void cls();
void game_quit();
int key_press();
#endif // Windows/Linux
#endif
