# Captal Engine
Captal is a C++20 low-level 2D game engine.  
Low level means that, it is not the kind of engine you plug your code in, it is the kind of engine that you plug in your code. 
It also tries to keep the lower overhead as possible, while being accessible by most C++ developpers that have basic knowledge of GPU, audio and game programming.

## Modules

Captal is composed of 5 modules:

* Captal foundation: A header-only standard-compliant library that serve as a base of all other modules.
* Apyre: A windowing and input management library built on SDL2.
* Swell: An audio playback and spacialization library built on Portaudio and other libraries for audio files decoding.
* Tephra: A low-level GPU API based on Vulkan. 
* Captal: The actual game engine, depends of the 4 other modules.

Apyre, Swell and Tephra can be used independently from each others.

## Features

* Captal foundation
  * Maths (vectors, matrices and associated functions)
  * Stack allocators
  * Unicode encoding conversion, few text transformation functions (to upper/lower that support more than ASCII)
  * Utility types and functions.
* Apyre
  * Windowing
  * Input management (no joystick/game constroller support, yet)
  * Monitors informations
  * Power informations
* Swell
  * Audio mixing and spacialization
  * WAV, OGG and FLAC file decoding
  * Audio devices informations
  * Audio playback (and recoding WIP)
* Tephra
  * Vulkan equivalent interface, but with automatic memory management (both CPU ressources and GPU memory)
  * Easily extensible (can use foreign handles)
  * PNG, JPEG, TGA, BMP and GIF (single frame) file decoder 
* Captal
  * 2D rendering engine (sprites, tilemaps, convex shapes, custom renderables)
  * On-screen rendering and off-screen rendering
  * Lower level modern GPU usage (custom shaders, UBO, SSBO, push constants, compute shaders, ...)
  * Font loader
  * Text rendering (rich text support planned)
  * 2D physics
  * Signals/slots, using [sigslot](https://github.com/palacaze/sigslot)
  * ECS, using [Entt](https://github.com/skypjack/entt), with additional prebuild systems and components
  * Parser for [Tiled](https://www.mapeditor.org/) TMX files
  * Custom translation files support (parser, editor and high-level translator)
  * State machine
  * Zlib wrapper

## Planned Features

* Apyre
  * Game controllers and Joysticks support
* Swell
  * File encoder for OGG, WAV and FLAC
* Tephra
* Captal
  * Rich text
  * Widgets
  * Virtual file system (archives)
  * Higher level interface for compute shaders
  * Particules

## Platforms

Captal is known to build and run on Windows 10, compiled with either MinGW GCC 10.3, or MSVC v142 (VS 16.11). Windows 7+ should be OK to.  
Also note that **Captal requires a Vulkan capable GPU**, most GPU released in 2012 or later support it with up-to-date drivers (the only exception I know are Intel integrated GPU, only on Windows where you need a chip released in 2015 or later).  
Because Captal uses cross-platform or standard-compliant dependencies, it should theoretically compile and run on Linux, and maybe OSX. 
Even Android may work, but it will probably need some adjustments. 
For now, I do plan to support Linux and Android "officialy" but I can't know when I will do it.

## License

Captal uses the [MIT license](https://opensource.org/licenses/MIT).

## Build

Captal is currently build with **CMake 3.21+**, you **must have Git installed** on your system, because the CMake file will download and compile all dependencies.
Here is the list of available options for the CMakeFiles:

|  Option                          | Default |  Description 
| :------------------------------: | :-----: | :---:
| CAPTAL_USE_LTO                   | OFF     | Build Captal and its submodules with LTO enabled, if supported
| CAPTAL_USE_CUSTOM_C_FLAGS        | ON      | Build C submodules with predefined compiler options, if supported
| CAPTAL_BUILD_FOUNDATION_EXAMPLES | OFF     | Build Captal Foundation's examples if ON
| CAPTAL_BUILD_FOUNDATION_TESTS    | OFF     | Build Captal Foundation's unit tests if ON
| CAPTAL_BUILD_APYRE               | OFF     | Build Apyre if ON
| CAPTAL_BUILD_APYRE_STATIC        | OFF     | Build Apyre as a static library if ON
| CAPTAL_BUILD_APYRE_EXAMPLES      | OFF     | Build Apyre's examples if ON, does nothing if CAPTAL_BUILD_APYRE is off
| CAPTAL_BUILD_APYRE_TESTS         | OFF     | Build Apyre's unit tests if ON, does nothing if CAPTAL_BUILD_APYRE is off
| CAPTAL_BUILD_TEPHRA              | OFF     | Build Tephra if ON
| CAPTAL_BUILD_TEPHRA_STATIC       | OFF     | Build Tephra as a static library if ON
| CAPTAL_BUILD_TEPHRA_EXAMPLES     | OFF     | Build Tephra's examples if ON, implies CAPTAL_BUILD_APYRE. Does nothing if CAPTAL_BUILD_TEPHRA is off
| CAPTAL_BUILD_TEPHRA_TESTS        | OFF     | Build Tephra's unit tests if ON, implies CAPTAL_BUILD_APYRE. Does nothing if CAPTAL_BUILD_TEPHRA is off
| CAPTAL_BUILD_SWELL               | OFF     | Build Swell if ON
| CAPTAL_BUILD_SWELL_STATIC        | OFF     | Build Swell as a static library if ON
| CAPTAL_BUILD_SWELL_EXAMPLES      | OFF     | Build Swell's examples if ON. Does nothing if CAPTAL_BUILD_SWELL is off
| CAPTAL_BUILD_SWELL_TESTS         | OFF     | Build Swell's unit tests if ON. Does nothing if CAPTAL_BUILD_SWELL is off
| CAPTAL_BUILD_CAPTAL              | OFF     | Build Captal if ON, implies CAPTAL_BUILD_TEPHRA, CAPTAL_BUILD_APYRE and CAPTAL_BUILD_SWELL if ON
| CAPTAL_BUILD_CAPTAL_STATIC       | OFF     | Build Captal as a static library if ON
| CAPTAL_BUILD_CAPTAL_EXAMPLES     | OFF     | Build Captal's examples if ON. Does nothing if CAPTAL_BUILD_CAPTAL is off
| CAPTAL_BUILD_CAPTAL_TESTS        | OFF     | Build Captal's unit tests if ON. Does nothing if CAPTAL_BUILD_CAPTAL is off

For most usages, you will just want to enable CAPTAL_BUILD_CAPTAL (which will compile also all other modules), and CAPTAL_USE_LTO for release build.  
CAPTAL_BUILD_XXX_STATIC creates of a static library for the specified modules. Because both Captal and your application will link to Apyre, Swell and Tephra, I recommend to use one of the following combination of parameters to prevent code duplication:

* All dynamic (none) 
* Only Captal as a static library (CAPTAL_BUILD_CAPTAL_STATIC)
* All static (CAPTAL_BUILD_APYRE_STATIC + CAPTAL_BUILD_TEPHRA_STATIC + CAPTAL_BUILD_SWELL_STATIC + CAPTAL_BUILD_CAPTAL_STATIC)
