using GameEngine;
using System;

public class TransformExpirement : EngineCore
{

    public double elapsedTime;
    public float radius = 5;
    public void update()
    {
        elapsedTime += deltaTime;
        if (elapsedTime > (2 * Math.PI))
        {
            elapsedTime = 0;
        }
        Object.transform.position.x = radius * (float)Math.Cos(elapsedTime);
        Object.transform.position.z = radius * (float)Math.Sin(elapsedTime);
        //transform.position.y = radius * (float)Math.Tan(elapsedTime); <- this works
    }
}