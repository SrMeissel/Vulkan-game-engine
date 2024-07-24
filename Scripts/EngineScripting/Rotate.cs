using GameEngine;
using System;

class Rotate : EngineCore
{
    public void update()
    {
        Object.transform.rotation.X += deltaTime;
        Object.transform.rotation.Y += deltaTime;
        Object.transform.rotation.Z += deltaTime;

        //Console.WriteLine(Object.transform.rotation.ToString());
    }
}
