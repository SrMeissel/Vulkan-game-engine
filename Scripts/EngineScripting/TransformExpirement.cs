using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class TransformExpirement
{

    public float deltaTime;

    public float transformPositionX;

    public void update()
    {
        transformPositionX += 0.5f * deltaTime;
    }

}
