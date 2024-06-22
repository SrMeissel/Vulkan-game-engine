using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;


namespace GameEngine
{
    public class EngineCore
    {

        public float deltaTime;

        [StructLayout(LayoutKind.Sequential)]
        public struct Vec3
        {
            public float x;
            public float y;
            public float z;

            public Vec3(float x, float y, float z)
            {
                this.x = x;
                this.y = y;
                this.z = z;
            }
        }
        [StructLayout(LayoutKind.Sequential)]
        public struct Transform
        {
            public Vec3 position;
            public Vec3 rotation;

            public Transform(Vec3 position, Vec3 rotation)
            {
                this.position = position;
                this.rotation = rotation;

            }
        }
        public Transform transform;

        public void loadTransform(float posX, float posY, float posZ, float rotX, float rotY, float rotZ)
        {
            this.transform = new Transform(new Vec3(posX, posY, posZ), new Vec3(rotX, rotY, rotZ)); 
        } // so I can load the data with a single funciton call, but how do I get it back?          Solved :)       nvm :(

        public Vec3 returnPosition()
        {
            return this.transform.position;
        }
        public Vec3 returnRotation()
        {
            return this.transform.rotation;
        }
    }
}
