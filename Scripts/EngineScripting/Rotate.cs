using GameEngine;
using System;

class Rotate : EngineCore
{
    public void update()
    {
        transform.rotation.x += deltaTime;
        transform.rotation.y += deltaTime;
        transform.rotation.z += deltaTime;
    }
}
