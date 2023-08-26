#version 450

#define PI 3.14159265358979323846264338327950288419716939937510

#define RED 0xff0000ff
#define GREEN 0xff00ff00
#define BLUE 0xffff0000
#define ALPHA 0xff000000
#define WHITE 0xffffffff

struct newDataStructe
{
    vec4 startAndEndPoint;
    vec4 previusCurveEndDerivateAndNextcurveStartDerivate;
    vec4 colorAndAngles;
};

layout ( push_constant) uniform constants
{
    uint maxIndex, CurvesCount, width, height;
} pushConstants;

layout (std140, binding = 0) readonly uniform UBO
{
    newDataStructe curves[100];
};

layout (std140, binding = 1) readonly buffer Image
{
    vec4 SDF[ ];
    //layout: distance, inside, root, curveIndex
};

layout (std140, binding = 2) writeonly buffer OUTIMAGE
{
    uint image[ ];
    // vec4 putainDeMerde[ ];
};

uint CustomMix(const in uint a, const in uint b, const in float c)
{
    const uint delta = b - a;
    if(c > 1.0)
        return b;
    return a + uint(delta * c);
}

uint insideColor(const in uint a)
{
    return ~a;
}

float zCross(const vec2 a, const vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

uint rgba_interp(uint src, uint dst, uint t) {
    const uint s = 255 - t;
    return (
        (((((src >> 0)  & 0xff) * s +
           ((dst >> 0)  & 0xff) * t) >> 8)) |
        (((((src >> 8)  & 0xff) * s +
           ((dst >> 8)  & 0xff) * t)     )  & ~0xff) |
        (((((src >> 16) & 0xff) * s +
           ((dst >> 16) & 0xff) * t) << 8)  & ~0xffff) |
        (((((src >> 24) & 0xff) * s +
           ((dst >> 24) & 0xff) * t) << 16) & ~0xffffff)
    );
}

float findInterpolationValue(float distanceToCurve, const bool isInside, const float distanceOfRectangle)
{
    if(isInside)
        distanceToCurve = -distanceToCurve;

    distanceToCurve = clamp(distanceToCurve, -distanceOfRectangle, distanceOfRectangle);

    const float a = 1.0/(2.0 * distanceOfRectangle);
    const float b = .5;

    return a * distanceToCurve + b;
}


bool isSmooth(const uint curveIndex, const float root)
{
    return true;
}

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

void main()
{
    uint index = gl_GlobalInvocationID.x;
    if(index < pushConstants.maxIndex)
    {
        const uint curveIndex = uint(SDF[index].w);
        const float inside = SDF[index].y;
        const float root = SDF[index].z;
        const float distanceToCurve = SDF[index].x;
        const bool isInside = inside < 0;
        // const float distanceOfRectangle = sqrt(1.0/(pushConstants.width * pushConstants.width) + 1.0/(pushConstants.height * pushConstants.height)) / 2.0;
        const float distanceOfRectangle = 0.0244086 / 32.5;

        // if (distanceToCurve > distanceOfRectangle)
        // {
        //     if (isInside)
        //         image[index] = WHITE;
        //     else
        //         image[index] = 0;
        // }
        // else
        //     image[index] = rgba_interp(WHITE, 0, uint(255 * findInterpolationValue(distanceToCurve, isInside, distanceOfRectangle)));
        //
        //
        // if(distanceToCurve > distanceOfRectangle * 2.0)
        // {
        //     if(isInside)
        //         image[index] = WHITE;
        //     else
        //         image[index] = 0;
        //     return;
        // }


        uint color = uint(curves[curveIndex].colorAndAngles.x);

        const float x = index % pushConstants.width;
        const float y = (index - x) / float(pushConstants.width);
        const vec2 point = vec2((x + 0.5) / pushConstants.width, (y+0.5) / pushConstants.height);

        vec2 derivate;
        vec2 pointToCurve;
        bool isConcave;

        if(root < 0.5)
        {
            derivate = curves[curveIndex].previusCurveEndDerivateAndNextcurveStartDerivate.xy;
            pointToCurve = point - curves[curveIndex].startAndEndPoint.xy;
            isConcave = bool(curves[curveIndex].colorAndAngles.y);
        }
        else
        {
            derivate = curves[curveIndex].previusCurveEndDerivateAndNextcurveStartDerivate.zw;
            pointToCurve = point - curves[curveIndex].startAndEndPoint.zw;
            isConcave = bool(curves[curveIndex].colorAndAngles.z);
        }

        //TODO rework here
        if(isConcave)
        {
            color = insideColor(color);
            //derivate first then point to curve !!
            const float dis = zCross(derivate, pointToCurve);
            if (isInside && dis < distanceToCurve)
                color = rgba_interp(WHITE, color, uint(findInterpolationValue(abs(dis), dis < 0, distanceOfRectangle) * 255));
            else
                color = rgba_interp(color, 0, uint(findInterpolationValue(distanceToCurve, isInside, distanceOfRectangle) * 255));
        }
        else
        {
            const float dis = zCross(derivate, pointToCurve);
            if(!isInside && dis < distanceToCurve)
                color = rgba_interp(0, color, uint(findInterpolationValue(abs(dis), dis > 0, distanceOfRectangle) * 255));
            else
                 color = rgba_interp(WHITE, color, uint(findInterpolationValue(distanceToCurve, isInside, distanceOfRectangle) * 255));
        }

        if(color != 0)
            image[index] = color | ALPHA;
        else
            image[index] = 0;
    }
}
