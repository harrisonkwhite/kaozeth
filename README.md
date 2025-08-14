# Terraria Clone in C

<img src="https://github.com/user-attachments/assets/5bd76a70-30ed-4fe0-8b2e-2decf3a3511f" alt="Terraria Clone Screenshot" style="max-width: 100%; height: auto;" />

---

## About

This is a personal project I am developing to improve my programming skills.

It began as an original game, but I eventually realised it would be more productive to just clone an existing one so I could focus solely on programming technique and not have to worry about game design.

I chose to clone Terraria specifically because it is composed of so many different interesting systems (e.g. world generation, lighting, liquid simulation, NPC AI, networking).

> **Note:** This project uses [Zeta Framework](https://github.com/harrisonkwhite/zeta_framework), a simple framework I made for developing 2D games using OpenGL.

---

## Building

Building and running this project has been tested on Linux and Windows.

Clone the repository by running `git clone --recursive https://github.com/harrisonkwhite/terraria_clone.git`.

> **Note:** If the repository was cloned non-recursively before, just run `git submodule update --init --recursive` to clone the necessary submodules.

Then go into the repository root and build with CMake:

```
mkdir build
cd build
cmake ..
```

For Linux, there are a number of dependencies you might need to manually install. CMake will report if any are missing.

You can also use the "run.sh" shell script if on a platform which supports it.

---

## Upcoming Features

- Animations  
- Crafting  
- Biomes  
- Liquids  
- Falling Tiles (e.g. Sand)  
- Day/Night Cycle
