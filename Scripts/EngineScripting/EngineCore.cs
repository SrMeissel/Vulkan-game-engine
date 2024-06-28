using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Net;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Numerics;


namespace GameEngine
{
    public class EngineCore
    {

        public String thingy; // WHY WONT THIS WORK. MUST MORE BLOOD BE SPILLED?

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
            public Vector3 position;
            public Vector3 rotation;

            public Transform(Vector3 position, Vector3 rotation)
            {
                this.position = position;
                this.rotation = rotation;

            }
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct GameObject
        {
            public Transform transform;
        }
        public GameObject Object;

        // c++ Functions ======================================================================

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool DoSomething(String pointer, int key);
        public bool isButtonDown(int key)
        {
            return DoSomething(this.thingy, key);
        }

        // Data Transfer ==================================================================================

        public void loadTransform(float posX, float posY, float posZ, float rotX, float rotY, float rotZ)
        {
            this.Object.transform = new Transform(new Vector3(posX, posY, posZ), new Vector3(rotX, rotY, rotZ)); 
        } // so I can load the data with a single funciton call, but how do I get it back?          Solved :)       nvm :(

        public Vector3 returnPosition()
        {
            return this.Object.transform.position;
        }
        public Vector3 returnRotation()
        {
            return this.Object.transform.rotation;
        }

        public const int GLFW_KEY_SPACE = 32;
        public const int GLFW_KEY_APOSTROPHE = 39;  /* ' */
        public const int GLFW_KEY_COMMA = 44;  /* , */
        public const int GLFW_KEY_MINUS = 45;  /* - */
        public const int GLFW_KEY_PERIOD = 46;  /* . */
        public const int GLFW_KEY_SLASH = 47;  /* / */
        public const int GLFW_KEY_0 = 48;
        public const int GLFW_KEY_1 = 49;
        public const int GLFW_KEY_2 = 50;
        public const int GLFW_KEY_3 = 51;
        public const int GLFW_KEY_4 = 52;
        public const int GLFW_KEY_5 = 53;
        public const int GLFW_KEY_6 = 54;
        public const int GLFW_KEY_7 = 55;
        public const int GLFW_KEY_8 = 56;
        public const int GLFW_KEY_9 = 57;
        public const int GLFW_KEY_SEMICOLON = 59;  /* ; */
        public const int GLFW_KEY_EQUAL = 61;  /* = */
        public const int GLFW_KEY_A = 65;
        public const int GLFW_KEY_B = 66;
        public const int GLFW_KEY_C = 67;
        public const int GLFW_KEY_D = 68;
        public const int GLFW_KEY_E = 69;
        public const int GLFW_KEY_F = 70;
        public const int GLFW_KEY_G = 71;
        public const int GLFW_KEY_H = 72;
        public const int GLFW_KEY_I = 73;
        public const int GLFW_KEY_J = 74;
        public const int GLFW_KEY_K = 75;
        public const int GLFW_KEY_L = 76;
        public const int GLFW_KEY_M = 77;
        public const int GLFW_KEY_N = 78;
        public const int GLFW_KEY_O = 79;
        public const int GLFW_KEY_P = 80;
        public const int GLFW_KEY_Q = 81;
        public const int GLFW_KEY_R = 82;
        public const int GLFW_KEY_S = 83;
        public const int GLFW_KEY_T = 84;
        public const int GLFW_KEY_U = 85;
        public const int GLFW_KEY_V = 86;
        public const int GLFW_KEY_W = 87;
        public const int GLFW_KEY_X = 88;
        public const int GLFW_KEY_Y = 89;
        public const int GLFW_KEY_Z = 90;
        public const int GLFW_KEY_LEFT_BRACKET = 91;  /* [ */
        public const int GLFW_KEY_BACKSLASH = 92;  /* \ */
        public const int GLFW_KEY_RIGHT_BRACKET = 93;  /* ] */
        public const int GLFW_KEY_GRAVE_ACCENT = 96;  /* ` */

        public const int  GLFW_KEY_ENTER              =257;
        public const int  GLFW_KEY_ESCAPE             =256;
        public const int  GLFW_KEY_TAB                =258;
        public const int  GLFW_KEY_BACKSPACE          =259;
        public const int  GLFW_KEY_INSERT             =260;
        public const int  GLFW_KEY_DELETE             =261;
        public const int  GLFW_KEY_RIGHT              =262;
        public const int  GLFW_KEY_LEFT               =263;
        public const int  GLFW_KEY_DOWN               =264;
        public const int  GLFW_KEY_UP                 =265;
        public const int  GLFW_KEY_PAGE_UP            =266;
        public const int  GLFW_KEY_PAGE_DOWN          =267;
        public const int  GLFW_KEY_HOME               =268;
        public const int  GLFW_KEY_END                =269;
        public const int  GLFW_KEY_CAPS_LOCK          =280;
        public const int  GLFW_KEY_SCROLL_LOCK        =281;
        public const int  GLFW_KEY_NUM_LOCK           =282;
        public const int  GLFW_KEY_PRINT_SCREEN       =283;
        public const int  GLFW_KEY_PAUSE              =284;
        public const int  GLFW_KEY_F1                 =290;
        public const int  GLFW_KEY_F2                 =291;
        public const int  GLFW_KEY_F3                 =292;
        public const int  GLFW_KEY_F4                 =293;
        public const int  GLFW_KEY_F5                 =294;
        public const int  GLFW_KEY_F6                 =295;
        public const int  GLFW_KEY_F7                 =296;
        public const int  GLFW_KEY_F8                 =297;
        public const int  GLFW_KEY_F9                 =298;
        public const int  GLFW_KEY_F10                =299;
        public const int  GLFW_KEY_F11                =300;
        public const int  GLFW_KEY_F12                =301;
        public const int  GLFW_KEY_F13                =302;
        public const int  GLFW_KEY_F14                =303;
        public const int  GLFW_KEY_F15                =304;
        public const int  GLFW_KEY_F16                =305;
        public const int  GLFW_KEY_F17                =306;
        public const int  GLFW_KEY_F18                =307;
        public const int  GLFW_KEY_F19                =308;
        public const int  GLFW_KEY_F20                =309;
        public const int  GLFW_KEY_F21                =310;
        public const int  GLFW_KEY_F22                =311;
        public const int  GLFW_KEY_F23                =312;
        public const int  GLFW_KEY_F24                =313;
        public const int  GLFW_KEY_F25                =314;
        public const int  GLFW_KEY_KP_0               =320;
        public const int  GLFW_KEY_KP_1               =321;
        public const int  GLFW_KEY_KP_2               =322;
        public const int  GLFW_KEY_KP_3               =323;
        public const int  GLFW_KEY_KP_4               =324;
        public const int  GLFW_KEY_KP_5               =325;
        public const int  GLFW_KEY_KP_6               =326;
        public const int  GLFW_KEY_KP_7               =327;
        public const int  GLFW_KEY_KP_8               =328;
        public const int  GLFW_KEY_KP_9               =329;
        public const int  GLFW_KEY_KP_DECIMAL         =330;
        public const int  GLFW_KEY_KP_DIVIDE          =331;
        public const int  GLFW_KEY_KP_MULTIPLY        =332;
        public const int  GLFW_KEY_KP_SUBTRACT        =333;
        public const int  GLFW_KEY_KP_ADD             =334;
        public const int  GLFW_KEY_KP_ENTER           =335;
        public const int  GLFW_KEY_KP_EQUAL           =336;
        public const int  GLFW_KEY_LEFT_SHIFT         =340;
        public const int  GLFW_KEY_LEFT_CONTROL       =341;
        public const int  GLFW_KEY_LEFT_ALT           =342;
        public const int  GLFW_KEY_LEFT_SUPER         =343;
        public const int  GLFW_KEY_RIGHT_SHIFT        =344;
        public const int  GLFW_KEY_RIGHT_CONTROL      =345;
        public const int  GLFW_KEY_RIGHT_ALT          =346;
        public const int  GLFW_KEY_RIGHT_SUPER        =347;
        public const int  GLFW_KEY_MENU               =348;
    }
}
