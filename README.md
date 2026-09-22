# Theseus' Labyrinth

![Theseus' Labyrinth preview](Assimp%2C%20Hello%20World%21/resources/textures/endmenu.jpg)

Theseus' Labyrinth is a first-person 3D labyrinth game developed in C++ with OpenGL. Explore the maze, fight the Minotaur and its guardians, collect power-ups, and find the exit.

## Features

- First-person maze exploration
- OpenGL rendering with dynamic lighting and normal mapping
- Animated Minotaur and enemy models
- Sword combat and enemy health systems
- Chests with health and damage power-ups
- Torch particles and lighting
- Ariadne's thread hint system
- Jump scares and background audio
- FPS counter and stamina-based sprinting

## Requirements

- Windows 10 or later
- Visual Studio with the MSVC C++ toolchain
- CMake 3.20 or later
- OpenGL 3.3 compatible graphics card

The repository includes the headers and prebuilt libraries used by the project. Third-party components remain subject to their own licenses.

## Build with CMake

From PowerShell at the repository root:

```powershell
$cmake = "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$source = "$(Get-Location)\Assimp, Hello World!"
$build = "$(Get-Location)\build"

& $cmake -S $source -B $build -G "Visual Studio 18 2026"
& $cmake --build $build --config Debug --parallel
```

The executable is generated at:

```text
build/Debug/Theseus.exe
```

CMake copies the shaders, models, textures, audio files, and runtime DLLs next to the executable. Run the game with `build/Debug` as the working directory so relative resource paths resolve correctly.

## Run

```powershell
Push-Location ".\build\Debug"
& ".\Theseus.exe"
Pop-Location
```

## Controls

| Key | Action |
| --- | --- |
| `Enter` | Start or restart the game |
| `WASD` | Move |
| `Mouse` | Look around |
| `Left Shift` | Sprint |
| `Left Mouse Button` | Attack |
| `E` | Open a nearby chest |
| `H` | Show Ariadne's thread after defeating the Minotaur |
| `F` | Toggle the FPS counter |
| `P` | Print the player coordinates |
| `Esc` | Exit |

## Project Structure

- `Assimp, Hello World!/` - C++ sources, shaders, models, textures, and audio
- `Assimp, Hello World!/CMakeLists.txt` - CMake build configuration
- `Assimp, Hello World!/include/` - bundled third-party headers
- `packages/` - GLFW package files
- `build/` - generated build files and executable; do not commit generated output

## License

The project code is released under the MIT License. See [LICENSE](LICENSE).

Third-party libraries, models, textures, fonts, and audio assets may have separate licenses or usage terms. Check the corresponding package or asset documentation before redistributing them.
