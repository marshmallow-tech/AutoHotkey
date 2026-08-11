# CLAUDE.md

Guidance for Claude Code when working in this repository.

## What this is

AutoHotkey v2 — the scripting language/interpreter, written in C++ for Windows. Source
lives entirely under `source/`; `AutoHotkeyx.sln` / `AutoHotkeyx.vcxproj` are the MSVC
project files.

**This project only builds on Windows with MSVC.** It uses the Win32 API directly and
has no cross-platform layer, so it cannot be compiled in a Linux container. Build with:

```
msbuild AutoHotkeyx.sln /p:Configuration=Debug /p:Platform=x64
```

There is no test suite in this repository. Verification of behavioural changes means
building on Windows and exercising the affected feature from a `.ahk` script.

## Window identity: `LastGastWater`

This fork registers its window classes and titles its main window under the name
**`LastGastWater`** rather than `AutoHotkey`. This is intentional. The goal is that the
Win32 `FindWindow` API can locate this build's windows only by the new name — by class
*and* by title — and that this build and a stock AutoHotkey build cannot see each
other's instances.

Current values:

| | Value |
|---|---|
| Main window class | `LastGastWater` |
| GUI window class | `LastGastWaterGUI` |
| Main window title | `<script path> - LastGastWater v<version>` |

### Two coupled sites — change them together

Renaming again means updating both, or the build breaks subtly rather than loudly:

1. **`source/defines.h`** — `WINDOW_CLASS_MAIN` and `WINDOW_CLASS_GUI`. Nearly every
   consumer goes through these macros (`RegisterClassEx` and `CreateWindowEx` in
   `Script::CreateWindows()` and `GuiType::Create()`, `UnregisterClass` in `~Script()`,
   and the `FindWindow` prior-instance check in `CheckPriorInstance()`), so they follow
   automatically.
2. **`source/ahkversion.cpp`** — `T_AHK_NAME_VERSION`, which forms the main window title
   via `Script::Init()`. This is the second argument to `FindWindow` in the
   prior-instance check, so it must stay consistent with the class name. It is
   deliberately *not* derived from `WINDOW_CLASS_MAIN`: `defines.h` is not in the
   precompiled header (see the commented-out include in `stdafx.h`), and
   `ahkversion.cpp` is kept a minimal translation unit on purpose so that only it needs
   recompiling when the git-describe version changes.

`Script::Edit()` in `source/script.cpp` also filters on the class-name prefix, but it
derives that prefix from `WINDOW_CLASS_MAIN` via
`_tcsnicmp(class_name, WINDOW_CLASS_MAIN, _tcslen(WINDOW_CLASS_MAIN))`, so it follows a
rename automatically. It relies on `WINDOW_CLASS_GUI` sharing `WINDOW_CLASS_MAIN`'s
prefix — keep that true. (Upstream hardcoded the literal and its length here, which is
exactly the sync hazard this avoids.)

`GuiType::FindGui()` (`source/script_gui.cpp`) identifies our GUI windows by class
**atom** (`GetClassLong(hwnd, GCW_ATOM)`), not by name, so it is rename-safe and needs
no changes.

### Lookalike literals — do NOT rename these

Several other occurrences of the string `"AutoHotkey"` are unrelated to window identity
and must be left alone:

- `source/ahkversion.h` — `AHK_NAME`. Feeds the DBGp debugger `appid` and `language`
  name (`source/Debugger.cpp`, `source/Debugger.h`). Renaming it breaks debugger clients
  such as the VS Code AutoHotkey extension, which match on `language="AutoHotkey"`. It
  also feeds the `#Requires` error message, which should keep naming the real product.
- `source/script.cpp`, `Script::IsDirective()` — `_tcsnicmp(parameter, _T("AutoHotkey"), 10)`
  parses the `#Requires AutoHotkey <version>` directive. Same literal-plus-length shape
  as site 2 above; entirely unrelated. Renaming it breaks every script using `#Requires`.
- `source/script_autoit.cpp` — `InternetOpen(_T("AutoHotkey"), ...)` is an HTTP
  User-Agent string.
- `source/resources/AutoHotkey.rc`, `source/resources/resource.h`,
  `source/resources/AutoHotkey.exe.manifest` — version-resource product name, company
  name, and the side-by-side assembly identity. Not window identity.

### Known consequence

Scripts that hardcode `ahk_class AutoHotkey` or `ahk_class AutoHotkeyGUI` to find
AutoHotkey's own windows will no longer match under this build. This is inherent to
renaming the class; no compatibility aliases are registered. Everything else — hotkeys,
GUI creation and events, `#SingleInstance`, `Reload`, the tray menu, `ListLines`,
`ListVars`, and the debugger — is unaffected, because it all routes through the macros
or the class atom.
