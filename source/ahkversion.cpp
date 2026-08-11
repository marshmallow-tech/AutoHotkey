
#include "stdafx.h"
#include "ahkversion.h"

LPSTR AHK_VERSION = RAW_AHK_VERSION;
LPTSTR T_AHK_VERSION = _T(RAW_AHK_VERSION);
// Deliberately not T_AHK_NAME: this string forms the main window's title (see Script::Init()),
// which is the second argument of the FindWindow() prior-instance check, so it carries the same
// renamed identity as WINDOW_CLASS_MAIN.  AHK_NAME itself is left as "AutoHotkey" because it also
// feeds the DBGp appid/language name (Debugger.cpp) and the #Requires error message (script.cpp).
LPTSTR T_AHK_NAME_VERSION = _T("LastGastWater") _T(" v") _T(RAW_AHK_VERSION);
