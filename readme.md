# Introduction

This engine originally followed the tutorial series:
[Vulkan (c++) Game Engine Tutorials](https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR)
This engine has since surpassed it's scope.

Intending to create a differed renderer with a universal ray marcher. The pipeline will be defined as follows:
Objects -> Volumetrics -> Lighting -> Post-Proccessing -> UI
 I'm thinking UI should actually be first, no?

## Features w/ Sources

### Textures

Sources:

- <https://vulkan-tutorial.com/Texture_mapping/Images>
- <https://vkguide.dev/docs/chapter-5/drawing_images/>
- <https://www.youtube.com/watch?v=lbaHrocxQdM>

Textures were originally binded as a global array, but are now binded per object.

![Textures Showcase](Images/ScreenShots/BindedTextures.JPG)

I am looking into a more involved material system with multiple masks for things like:

- Albedo
- AO
- Bump
- Cavity
- Displacement
- gloss
- Normal
- Roughness
- Specular
- Transmission

![Example goal for material system](Experimental/Mossy_Ground_xiboab2r/preview.png)

### MultiSampling anti-aliasing

Sources:

- <https://vulkan-tutorial.com/Multisampling>

### Screenshots

Sources:

- <https://github.com/SaschaWillems/Vulkan/blob/master/examples/screenshot/screenshot.cpp>
- <https://reference.torque3d.org/coding/file/swizzle_8h/>

![BGR Screenshot](Images\ScreenShots\didntcrashscreenshot.jpg)

Not a very useful or optimized system, but I learned something. Also, If the image seems very blue, It's because it BGR instead of RGB and I cant be bothered to fix it.

### Physics engine

Sources:

- <https://www.realtimerendering.com/intersections.html>
- <https://www.youtube.com/watch?v=eED4bSkYCB8&t=62s>

- I discovered Nvidia Physx, but I dont think I can implement it without completely transitioning to visual studio and I dont want to do that. However I think I can use it's documentation to help me out:

  - <https://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Manual/Index.html>
  - <http://blog.virtualmethodstudio.com/2017/11/physics-101-3-solvers/>

### Fun Finds

- <https://www.youtube.com/watch?v=eED4bSkYCB8>
- <https://www.youtube.com/watch?v=ajv46BSqcK4>
- <https://www.youtube.com/watch?v=h7apO7q16V0>
- <https://www.khronos.org/collada/>
- <http://web.mit.edu/djwendel/www/weblogo/shapes/>
- <https://www.saschawillems.de/blog/2018/09/05/vulkan-conditional-rendering/>
- <https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/>

## Tasks

TO DO:

- [ ] shadows
- [ ] skybox
Going for volumetric atmosphere, turns out that is very hard.
- [ ] ECS
- [ ] UI
Going to use imgui for tooling UI to make the traisition for hardcoded objects to loaded files easier.
- [ ] Sound
- [ ] Physics
I'm taking a break from physics to do clouds
- [ ] Asset System
- [ ] CVAR System

### Atmosphere and clouds

Sources:

- <https://www.youtube.com/watch?v=8OrvIQUFptA>
- horizon: zero dawn has a lot of info
  - <https://www.youtube.com/watch?v=ToCozpl1sYY>

> wtf is 'dithering'
> wtf is a kilometer

- <https://sebh.github.io/publications/egsr2020.pdf>
- <http://evasion.imag.fr/~Antoine.Bouthors/research/phd/>
- <https://github.com/sebh/UnrealEngineSkyAtmosphere>
- <https://advances.realtimerendering.com/s2019/index.htm>

### Vulkan descriptors and memory bullshit

Turns out, I could/should set up an *arbitrary-ish* amount of discriptor sets based on how frequently they change. eg. per camera, per object, per frame.

- <https://www.reddit.com/r/vulkan/comments/4gvmus/best_way_for_textures_in_shaders/>
  first comment knows whats up ^

- <https://stackoverflow.com/questions/42214710/vkallocatedescriptorsets-returns-vk-out-of-host-memory>

These types are apparently very useful, but lacks proper hardware support so it would only make things more difficult in the end.

- Descriptor indexing
- Push descriptors
- bindless descriptors
- dynamic UBO's

## Vulkan resources

This is becoming more and more useful:
<https://vulkan.gpuinfo.org/displayreport.php?id=25342#formats_optimal>
built in variables for frag shaders:
<https://www.khronos.org/opengl/wiki/Fragment_Shader>

## Dependancies

- Vulkan SDK
- stb - <https://github.com/nothings/stb>
- glfw - <https://github.com/glfw/glfw>
- glm - <https://github.com/g-truc/glm>
- tinyobjloader - <https://github.com/tinyobjloader/tinyobjloader>
- imgui - <https://github.com/ocornut/imgui.git>
