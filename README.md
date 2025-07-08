# Terraria Clone in C

This is a personal project I am developing to improve my programming skills.

It began as an original game, but I eventually realised it would be more productive to just clone an existing one so I could focus solely on programming technique and not have to worry about game design.

I chose to clone Terraria specifically because it is composed of so many different interesting systems (e.g. world generation, lighting, liquid simulation, NPC AI, networking).

> **Note:** This project uses [Zeta Framework](https://github.com/harrisonkwhite/zeta_framework), a simple framework I made for developing 2D games using OpenGL.

---

## Building

Building and running this project has been tested on Windows and Linux.

Make sure to clone the repository **recursively**, then build using CMake:

```
mkdir build
cd build
cmake ..
```

For Linux, there are a number of dependencies you might need to manually install. CMake will report if any are missing.

> **Important:** The game itself depends on the assets folder being in the current working directory.

---

## Upcoming Features

- Animations  
- Asset Packing  
- Crafting  
- Biomes  
- Liquids  
- Falling Tiles (e.g. Sand)  
- Tilemap Lighting  
- Day/Night Cycle
