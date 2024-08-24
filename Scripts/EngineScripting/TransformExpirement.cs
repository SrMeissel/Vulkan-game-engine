using GameEngine;
using System;

public class TransformExpirement : EngineCore
{

    public double elapsedTime;
    public float radius = 2;
    public void update()
    {   
        elapsedTime += deltaTime;
        if (elapsedTime > (2 * Math.PI))
        {
            elapsedTime = 0;
        }
        Object.transform.position.X = radius * (float)Math.Cos(elapsedTime);
        Object.transform.position.Z = radius * (float)Math.Sin(elapsedTime) - 3;
        //transform.position.y = radius * (float)Math.Tan(elapsedTime); <- this works
    }
}