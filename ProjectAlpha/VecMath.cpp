#include "VecMath.h"

namespace ProjectAlpha
{
	namespace Math
	{


		//Vec2 functions

		real32 Cross(vec2 v1, vec2 v2)
		{
			return v1.x * v2.y - v2.x * v1.y;
		}

		real32 Length(vec2 v1)
		{
			return sqrt((v1.x * v1.x) + (v1.y * v1.y));
		}

		real32 Dist(vec2 v1, vec2 v2)
		{
			return Length(v2 - v1);
		}

		//Vec3 functions

		vec3 Cross(vec3 v1, vec3 v2)
		{
			return vec3{
				v1.y * v2.z - v1.z * v2.y,
				v1.z * v2.x - v1.x * v2.z,
				v1.x * v2.y - v1.y * v2.x
			};
		}

		real32 Length(vec3 v1)
		{
			return sqrt((v1.x * v1.x) + (v1.y * v1.y) * (v1.z * v1.z));
		}

		real32 Dist(vec3 v1, vec3 v2)
		{
			return Length(v2 - v1);
		}

		//Vec4 functions
		real32 Length(vec4 v1)
		{
			return sqrt((v1.x * v1.x) + (v1.y * v1.y) * (v1.z * v1.z));
		}

		vec4 Cross(vec4 v1, vec4 v2)
		{
			return vec4{
				v1.y * v2.z - v1.z * v2.y,
				v1.z * v2.x - v1.x * v2.z,
				v1.x * v2.y - v1.y * v2.x,
				0
			};
		}

	}
}