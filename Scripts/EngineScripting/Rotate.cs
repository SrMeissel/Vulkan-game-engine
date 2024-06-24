using GameEngine;
using System;

class Rotate : EngineCore
{
    public void update()
    {
        Object.transform.rotation.x += deltaTime;
        Object.transform.rotation.y += deltaTime;
        Object.transform.rotation.z += deltaTime;
    }
}
