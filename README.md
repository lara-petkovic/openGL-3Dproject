# 3D Air Defense Game in OpenGL

## Overview
This project is an immersive 3D air defense game developed in OpenGL. The player takes control of a drone stationed on a mountain, defending the city from incoming enemy targets. The game incorporates various features, including realistic lighting, 3D models for drones and targets, and dynamic environmental elements.

## Prerequisites
Ensure that you have the following dependencies installed before running the project:

- glew-2.2.0
- glfw
- glm

## Controls
- **Arrow Keys:** Move the drone
- **Space Key:** Activate or reset the drone's position
- **X Key:** Destroy the drone (limited uses)
- **W/S Keys:** Raise or lower the drone
- **1 Key:** Hide the map
- **2 Key:** Unhide the map
- **Esc Key:** Escape

## How to Play
1. Launch the game and hit the Space key to activate the drone.
2. Use arrow keys to move the drone, and 'W' and 'S' keys to raise or lower it.
3. Press the Space key to reset the drone position if needed.
4. Use the X key to destroy the drone (limited uses).
5. Defend the city by destroying incoming enemy targets.
6. The game ends when all enemy targets are destroyed, or the player runs out of drone lives.

## Lighting and Graphics
- The project incorporates a Fong lighting model for realistic illumination.
- The terrain is flat, except for a mountain where the drone station is located.
- The terrain texture is mapped as in 2D project.
- The scene is set at night with a subtle directional light.
- Depth testing and back-face culling are enabled for a more realistic rendering.

## 3D Models
- The drone is loaded as a 3D model.
- Targets are randomly generated at different heights with limited range point lights.
- Low-flying targets have a distinctive color, move at 1/3 the speed, and lack lights.

## Environmental Effects
- Semi-transparent clouds hover above the terrain.
- In the city center, there is at least one powerful spotlight that changes direction over time and illuminates a wide area.

## Appearance of the Application
![PVO-NS](https://github.com/lara-petkovic/openGL-3Dproject/assets/116621727/ac56f85a-4c46-4021-95fc-2716207d1330)

## Specification (Serbian)
![spec](https://github.com/lara-petkovic/openGL-3Dproject/assets/116621727/c558e73e-f886-4012-b6fe-76ee23ef2410)
