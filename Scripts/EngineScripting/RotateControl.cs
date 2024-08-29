using GameEngine;
using System;

class RotateControl : EngineCore
{
    float Xspeed = 0.0f;
    float Yspeed = 0.0f;
    float Zspeed = 0.0f;

    float acceleration = 15.0f;

    public void update()
    {
        //im a dumbass
        if (isButtonDown(GLFW_KEY_I)) Xspeed = acceleration;
        else if (isButtonDown(GLFW_KEY_K)) Xspeed = -acceleration;  else Xspeed = 0;
        if (isButtonDown(GLFW_KEY_O)) Yspeed = acceleration;
        else if (isButtonDown(GLFW_KEY_L)) Yspeed = -acceleration; else Yspeed = 0;
        if (isButtonDown(GLFW_KEY_U)) Zspeed = acceleration;
        else if (isButtonDown(GLFW_KEY_J)) Zspeed = -acceleration; else Zspeed = 0;

        Object.transform.rotation.X += Xspeed * deltaTime;
        Object.transform.rotation.Y += Yspeed * deltaTime;
        Object.transform.rotation.Z += Zspeed * deltaTime;

        //if (Object.transform.rotation.X > Math.PI * 2) Object.transform.rotation.X = 0;
        //if (Object.transform.rotation.Y > Math.PI * 2) Object.transform.rotation.Y = 0;
        //if (Object.transform.rotation.Z > Math.PI * 2) Object.transform.rotation.Z = 0;
        //Console.WriteLine(Object.transform.rotation.ToString());
    }
}
