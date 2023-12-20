In moving away from hardcoded data, A lot of changes to the app, swapchain, and renderpass need to be made
<https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/>
<https://google.github.io/filament/Filament.html>
Following Decima's example, I want to integrate the full game into the primary editor.
However, if the editor and game run as 1 proccess then when 1 crashes, both crash
If the engine launches the game as a seperate proccess, that wont happen. I just dont know how to do that
Avoid putting editor code in game code.
<https://www.guerrilla-games.com/media/News/Files/GDC2017_Sumaili_VanDerSteen_CreatingAToolsPipelineForHorizonZeroDawn.pdf>