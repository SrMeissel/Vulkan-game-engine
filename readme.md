# Introduction

This project is dedicated to the development of a custom game engine, focusing on the exploration and implementation of advanced rendering techniques and physics simulations. The initiative stems from a desire to deeply understand the mechanics behind game engines and to apply this knowledge in creating visually rich and interactive scenes. While navigating the complexities of game engine architecture, this project also serves as a practical learning journey, aiming to bridge theoretical concepts with real-world application.

![Textures Showcase](Images/ScreenShots/multipleLights.png)
![Stars](Images/ScreenShots/BetterStars.JPG)
![Shadows](Images/ScreenShots/goodShadows.JPG)
![Normal/Importing Image](Images/ScreenShots/SuccessfulNormalsANDImporting.png)

## Scripting API

The engine can now run scripts attached to game objects. The scripting engine uses Mono and scripts can currently process user input and access the properties of the parent entity.

### Parameters

- Object.Transform: This struct mirrors the transform component of the parent entity and the values of the translation and rotation are refleted in the scene. (Except scale atm)

### Functions

- isButtonDown(key): This function returns true if the key in question is currently pressed

## dependencies

- Vulkan SDK- <https://www.vulkan.org/>
- stb - <https://github.com/nothings/stb>
- glm - <https://github.com/g-truc/glm>
- tinyobjloader - <https://github.com/tinyobjloader/tinyobjloader>
- Mono - <https://www.mono-project.com/>
- Assimp 5.4.2 <https://github.com/assimp/assimp>

I started this engine following [Vulkan (c++) Game Engine Tutorials](https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR) by Brendan Galea. This was intrumental on getting the ball rolling and I am very grateful that he took the time to make the series.
