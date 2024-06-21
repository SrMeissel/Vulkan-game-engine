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
        transform.position.x = radius * (float)Math.Cos(elapsedTime);
        transform.position.z = radius * (float)Math.Sin(elapsedTime);

/*        transform.position.x += deltaTime;
        transform.position.y -= deltaTime;
        transform.position.z -= deltaTime;*/
    }
}