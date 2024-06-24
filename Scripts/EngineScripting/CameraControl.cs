using GameEngine;
using System;

class CameraControl : EngineCore
{
    public void update()
    {
        float horizontalVelocity = 0;
        float verticalVelocity = 0;

        if(isButtonDown(GLFW_KEY_RIGHT)) { horizontalVelocity += 1; }
        if(isButtonDown(GLFW_KEY_LEFT)) { horizontalVelocity -= 1; }
        if(isButtonDown(GLFW_KEY_UP)) { verticalVelocity += 1; }
        if(isButtonDown(GLFW_KEY_DOWN)) { verticalVelocity -= 1; }

        Object.transform.rotation.y += horizontalVelocity * deltaTime;
        Object.transform.rotation.x += verticalVelocity * deltaTime;

    }
}
