wtf am i doing?

from top level README: 
https://www.youtube.com/watch?v=8OrvIQUFptA
horizon: zero dawn has a lot of info 
https://www.youtube.com/watch?v=ToCozpl1sYY
wtf is 'dithering'
wtf is a kilometer
https://sebh.github.io/publications/egsr2020.pdf
http://evasion.imag.fr/~Antoine.Bouthors/research/phd/ 
https://github.com/sebh/UnrealEngineSkyAtmosphere
https://advances.realtimerendering.com/s2019/index.htm


https://graphics.pixar.com/library/ProductionVolumeRendering/paper.pdf
you know when pixar shows up shits getting real.
SUDO CODE ^

https://michaelwalczyk.com/blog-ray-marching.html
https://timcoster.com/2020/02/11/raymarching-shader-pt1-glsl/
https://learnopengl.com/Advanced-Lighting/Deferred-Shading


Create 4 LUT'S stored as image attachments in the render pass
Luts created once at startup or when atmospheric conditions change
Can be stored in participating media volume object.

Transmittance 256 x 64
Sky-View 200 x 100
Aerial Perspective 32^3 
Multiple Scattering 32^2 

Use in raymarching fragment shader referencing LUTS
Reference the depth buffer to figure out distances between steps


My first step will be simply to march along a ray from the camera to depth value adding to the value.
https://blog.demofox.org/2020/05/10/ray-marching-fog-with-blue-noise/