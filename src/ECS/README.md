#### this will be my ECS system  

https://medium.com/@savas/nomad-game-engine-part-2-ecs-9132829188e5  
this is great because it has nice pictures  
https://austinmorlan.com/posts/entity_component_system/  
components just hold data. Systems hold logic.  
std::bitset  
This is cool ^, real cool  

im going to replace the current system with one closely based off of austins exaple. Because its nice and I can understand it :)  
austin suggests using 2 maps, one entity/key pair, and a key/entity pair. why? no clue  

austin uses a type template to register types as components  

using a std::set to store keys for each system to iterate through  

my asset handler class will be my version of the coordinator  
but will include loading and saving things to disk (Ex. gltf stuff, editor things)  