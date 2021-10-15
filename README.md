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

## Features

* Captal foundation
  * Maths (vectors, matrices and associated functions)
  * Stack allocators
  * Unicode 
  * Utility types and functions.
* Apyre
  * Windowing
  * Input management
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
  * ECS, using [Entt](https://github.com/skypjack/entt), with additional prebuild systems
  * Support for [Tiled](https://www.mapeditor.org/) TMX files
  * Custom translation files support (parser and writter)
  * State machine
  * Zlib wrapper

## Platforms

Captal is known to build and run on Windows 10, compiled with either MinGW GCC 10.3, or MSVC v142 (VS 16.11).
Because Captal uses cross-platform or standard-compliant dependencies, it should theoretically compile and run on Linux, and Mac. 
Even Android may work, but it will probably need some adjustments. 
For now, I do plan to support Linux and Android but I can't know when I will do it.

## License

Not Enough Standards use the [MIT license](https://opensource.org/licenses/MIT).

## Build

Captal is currently build with **CMake 3.15.7+**, you **must have Git installed** on your system, because the CMake file will download and compile all dependencies.
Here is the list of available options for the CMakeFiles.

|  Option                          |  Description 
|:--------------------------------:|:------------------------------------------------------------------:
| CAPTAL_USE_LTO                   | Build Captal and its submodules with LTO enabled, if supported
| CAPTAL_USE_CUSTOM_C_FLAGS        | Build Captal and its submodules with predefined compiler options, if supported
| CAPTAL_BUILD_FOUNDATION_EXAMPLES | Build Captal Foundation's examples if ON
| CAPTAL_BUILD_FOUNDATION_TESTS    | Build Captal Foundation's unit tests if ON
| CAPTAL_BUILD_APYRE               | Build Apyre if ON
| CAPTAL_BUILD_APYRE_STATIC        | Build Apyre as a static library if ON
| CAPTAL_BUILD_APYRE_EXAMPLES      | Build Apyre's examples if ON, does nothing if CAPTAL_BUILD_APYRE is off
| CAPTAL_BUILD_APYRE_TESTS         | Build Apyre's unit tests if ON, does nothing if CAPTAL_BUILD_APYRE is off
| CAPTAL_BUILD_TEPHRA              | Build Tephra if ON
| CAPTAL_BUILD_TEPHRA_STATIC       | Build Tephra as a static library if ON
| CAPTAL_BUILD_TEPHRA_EXAMPLES     | Build Tephra's examples if ON, implies CAPTAL_BUILD_APYRE. Does nothing if CAPTAL_BUILD_TEPHRA is off
| CAPTAL_BUILD_TEPHRA_TESTS        | Build Tephra's unit tests if ON, implies CAPTAL_BUILD_APYRE. Does nothing if CAPTAL_BUILD_TEPHRA is off
| CAPTAL_BUILD_SWELL               | Build Swell if ON
| CAPTAL_BUILD_SWELL_STATIC        | Build Swell as a static library if ON
| CAPTAL_BUILD_SWELL_EXAMPLES      | Build Swell's examples if ON. Does nothing if CAPTAL_BUILD_SWELL is off
| CAPTAL_BUILD_SWELL_TESTS         | Build Swell's unit tests if ON. Does nothing if CAPTAL_BUILD_SWELL is off
| CAPTAL_BUILD_CAPTAL              | Build Captal if ON, implies CAPTAL_BUILD_TEPHRA, CAPTAL_BUILD_APYRE and CAPTAL_BUILD_SWELL if ON
| CAPTAL_BUILD_CAPTAL_STATIC       | Build Captal as a static library if ON
| CAPTAL_BUILD_CAPTAL_EXAMPLES     | Build Captal's examples if ON. Does nothing if CAPTAL_BUILD_CAPTAL is off
| CAPTAL_BUILD_CAPTAL_TESTS        | Build Captal's unit tests if ON. Does nothing if CAPTAL_BUILD_CAPTAL is off
