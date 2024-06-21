using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;


namespace GameEngine
{
    public class EngineCore
    {

        public float deltaTime;
        public struct Transform
        {
            [StructLayout(LayoutKind.Sequential)]
            public struct Position
            {
                public float x;
                public float y;
                public float z;

                public Position(float x, float y, float z)
                {
                    this.x = x;
                    this.y = y;
                    this.z = z;
                }
            }
            public Position position;

            public Transform(float x, float y, float z)
            {
                this.position = new Position(x, y, z);
            }
        }
        public Transform transform;

        public void loadTransform(float posX, float posY, float posZ)
        {
            this.transform = new Transform(posX, posY, posZ);
        } // so I can load the data with a single funciton call, but how do I get it back?

        public Transform.Position returnTransform()
        {
            return transform.position;
        }
    }
}
