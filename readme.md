This engine originally followed the tutorial series https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR

MultiSampling anti-aliasing implemented from: 
https://vulkan-tutorial.com/Multisampling

textures implemented from : 
https://vulkan-tutorial.com/Texture_mapping/Images
https://vkguide.dev/docs/chapter-5/drawing_images/
https://www.youtube.com/watch?v=lbaHrocxQdM

textures are currently binded in an array

![](ScreenShots/BindedTextures.JPG)

There exists a way to implement textrures without binding which seems more flexible.
PBR materials inspired from :
https://vulkanppp.wordpress.com/2017/07/06/week-6-normal-mapping-specular-mapping-pipeline-refactoring/

![](Experimental/Mossy_Ground_xiboab2r/preview.png)

uses a series of maps to apply different lighting across a texture

Screenshots impletmented from :
https://github.com/SaschaWillems/Vulkan/blob/master/examples/screenshot/screenshot.cpp
https://reference.torque3d.org/coding/file/swizzle_8h/

physics inspired from:
https://www.realtimerendering.com/intersections.html
https://www.youtube.com/watch?v=eED4bSkYCB8&t=62s
I discovered Nvidia Physx, but I dont think I can implement it without completely transitioning to visual studiom and I dont want to do that.
However I think I can use it's documentation to help me out:
https://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Manual/Index.html
http://blog.virtualmethodstudio.com/2017/11/physics-101-3-solvers/


fun finds:
https://www.youtube.com/watch?v=eED4bSkYCB8
https://www.youtube.com/watch?v=ajv46BSqcK4
https://www.youtube.com/watch?v=h7apO7q16V0
https://www.khronos.org/collada/
http://web.mit.edu/djwendel/www/weblogo/shapes/


TO DO:
shadows
skybox
ECS
UI
Sound
Physics - current objective


Depends on:
    Vulkan SDK
    stb - https://github.com/nothings/stb
    glfw
    glm
    tinyobjloader
    
This is becoming more and more useful: https://vulkan.gpuinfo.org/displayreport.php?id=25342#formats_optimal