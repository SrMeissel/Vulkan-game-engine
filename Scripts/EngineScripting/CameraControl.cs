using GameEngine;
using System;
using System.Numerics;

class CameraControl : EngineCore
{
    public void update()
    {
        float speedModifier = 2.5f;
        float speedModifierPOS = 3.0f;

        if(isButtonDown(GLFW_KEY_LEFT_SHIFT))
        {
            speedModifier *= 1.4f;
            speedModifierPOS *= 2.5f;
        }

        Vector3 velocity = new Vector3(0.0f, 0.0f, 0.0f);

        if (isButtonDown(GLFW_KEY_RIGHT)) { velocity.X += 1; }
        if (isButtonDown(GLFW_KEY_LEFT)) { velocity.X -= 1; }
        if (isButtonDown(GLFW_KEY_UP)) { velocity.Y += 1; }
        if (isButtonDown(GLFW_KEY_DOWN)) { velocity.Y -= 1; }

        if (Vector3.Dot(velocity, velocity) > 0)
        {
            velocity = Vector3.Normalize(velocity);
        }

        Object.transform.rotation.Y += velocity.X * deltaTime * speedModifier;
        Object.transform.rotation.X += velocity.Y * deltaTime * speedModifier;

        Object.transform.rotation.X = Math.Max(-1.5f, Object.transform.rotation.X);
        Object.transform.rotation.X = Math.Min(1.5f, Object.transform.rotation.X);

        Object.transform.rotation.Y = Object.transform.rotation.Y % (float)(2.0 * Math.PI);

        //translation ===============================================

        float yaw = Object.transform.rotation.Y;
        float pitch = Object.transform.rotation.X;

        Vector3 forwardDir = new Vector3((float)Math.Sin(yaw), (float)-Math.Tan(pitch), (float)Math.Cos(yaw));
        Vector3 rightDir = new Vector3(forwardDir.Z, 0.0f, -forwardDir.X);
        Vector3 upDir = new Vector3(0.0f, -1.0f, 0.0f);

        Vector3 moveDir = new Vector3(0.0f, 0.0f, 0.0f);
        if (isButtonDown(GLFW_KEY_W)) { moveDir += forwardDir; }
        if (isButtonDown(GLFW_KEY_S)) { moveDir -= forwardDir; }
        if (isButtonDown(GLFW_KEY_D)) { moveDir += rightDir; }
        if (isButtonDown(GLFW_KEY_A)) { moveDir -= rightDir; }
        if (isButtonDown(GLFW_KEY_E)) { moveDir += upDir; }
        if (isButtonDown(GLFW_KEY_Q)) { moveDir -= upDir; }

        if(Vector3.Dot(moveDir, moveDir) > 0)
        {
            Object.transform.position += Vector3.Normalize(moveDir) * deltaTime * speedModifierPOS;
        }
    }
}