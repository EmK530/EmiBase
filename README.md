# EmiBase
**A 2D game development framework for Raylib in C that offers various useful functions**.
> [!CAUTION]
>  EmiBase is still under active development, many APIs are subject to breaking changes.<br>
>  Please review changes before you update your copy of EmiBase if you are starting a project.<br><br>
>  APIs will be extensively documented when the engine is closer to completion.

**An example of some notable functions being**:
- Local transform objects with hierarchy and positioning similar to Roblox UI.
- Scene system to manage swapping through different sections of game code.
- Custom crash handler for catching and displaying exceptions, rather than silently closing.
- Custom binary format to contain your game assets with optional CRC32 validation and/or encryption.
- Various helper functions for managing audio playback, text rendering, image drawing and fullscreen shading.
- Nuklear debugging interface to swap scenes, toggle rendering stages and alter objects in real-time.

## How to build
EmiBase is currently only supported for Windows, however Linux will be included in the future.
- Download Raylib 6.0 through the itch.io Windows installer: https://raysan5.itch.io/raylib/purchase
- Download Python and ensure it is in PATH: https://www.python.org/downloads/
- Add `C:\raylib\w64devkit\bin` to your PATH.
- Now you can run `mingw32-make` for debug builds and `mingw32-make release` for release builds.
> [!IMPORTANT]
>  Note that it is recommended you run `mingw32-make clean` when you swap build modes to flush the build cache.

## Libraries
The EmiBase engine uses the following third-party libraries:
- [Raylib](https://github.com/raysan5/raylib) ([Zlib License](https://github.com/EmK530/EmiBase/blob/main/THIRD_PARTY#L1))
- [raylib-nuklear](https://github.com/RobLoach/raylib-nuklear) ([Zlib License](https://github.com/EmK530/EmiBase/blob/main/THIRD_PARTY#L21))
  - [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) ([MIT License](https://github.com/EmK530/EmiBase/blob/main/THIRD_PARTY#L41))

## Assets
The template scene includes the following third-party assets:
- [MxPlus_IBM_VGA_8x16](https://int10h.org/oldschool-pc-fonts/fontlist/font?ibm_vga_8x16) / Created by VileR (https://int10h.org/)