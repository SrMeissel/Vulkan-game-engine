This describes the proccess's involved in the physics system.

Separating Axis Theorem
https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics4collisiondetection/2017%20Tutorial%204%20-%20Collision%20Detection.pdf
sweep-and-prune
multi box pruning
Gauss-Seidel Collision solver
https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/5collisionresponse/Physics%20-%20Collision%20Response.pdf
raycasting
https://docs.decentraland.org/creator/development-guide/sdk7/raycasting/
https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/1raycasting/Physics%20-%20Raycasting.pdf

Collision detection:
every physics object is given an AABB //with padding equal to the resting extent
object pairs are culled using by sweeping along coordinate axis'

To do:
create AABB
cast ray from camera pos looking forward.
ray/plane collision detection
ray/AAB collision detection 