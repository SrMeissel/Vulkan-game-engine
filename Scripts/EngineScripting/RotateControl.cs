using GameEngine;
using System;

class RotateControl : EngineCore
{
    float speed = 0.0f;
    float acceleration = 15.0f;

    public void update()
    {
        //im a dumbass
        if (isButtonDown(GLFW_KEY_I)) speed += deltaTime * acceleration;
        if (isButtonDown(GLFW_KEY_O)) speed += deltaTime * acceleration;
        if (isButtonDown(GLFW_KEY_U)) speed += deltaTime * acceleration;

        Object.transform.rotation.X += speed * deltaTime;
        Object.transform.rotation.Y += speed * deltaTime;
        Object.transform.rotation.Z += speed * deltaTime;

        //if (Object.transform.rotation.X > Math.PI * 2) Object.transform.rotation.X = 0;
        //if (Object.transform.rotation.Y > Math.PI * 2) Object.transform.rotation.Y = 0;
        //if (Object.transform.rotation.Z > Math.PI * 2) Object.transform.rotation.Z = 0;
        //Console.WriteLine(Object.transform.rotation.ToString());
    }
}
