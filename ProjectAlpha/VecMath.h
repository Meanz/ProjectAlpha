#pragma once

#include "ctdfs.h"
#include <iostream>

namespace ProjectAlpha
{
	namespace Math
	{

		struct v2
		{
			real32 x;
			real32 y;

			v2() {}
			v2(real32 _x, real32 _y) : x(_x), y(_y) {}

			v2 operator/(const real32& by);
			v2 operator+(const v2& other);
			v2 operator-(const v2& other);
		};

		real32 Cross(v2 v1, v2 v2);
		real32 Length(v2 v1);
		real32 Dist(v2 v1, v2 v2);

		struct v3
		{

			real32 x;
			real32 y;
			real32 z;

			v3() {}
			v3(real32 _x, real32 _y, real32 _z) : x(_x), y(_y), z(_z) {}

			v3 operator/(const real32& by);
			v3 operator+(const v3& other);
			v3 operator-(const v3& other);
		};

		v3 Cross(v3 v1, v3 v2);
		real32 Length(v3 v1);
		real32 Dist(v3 v1, v3 v2);
		v3 Normalize(v3 v1);

		struct v4
		{
			real32 x;
			real32 y;
			real32 z;
			real32 w;
			
			v4() {}
			v4(real32 _x, real32 _y, real32 _z, real32 _w) : x(_x), y(_y), z(_z), w(_w) {}

			v4 operator/(const real32& by);
			v4 operator+(const v4& other);
			v4 operator-(const v4& other);
		};

		real32 Length(v4 v1);
		v4 Cross(v4 v1, v4 v2);
		v4 Normalize(v4 v1);

		struct mat3
		{
			v3 rows[3];
		};

		struct mat4
		{
			//column major
			real32 values[16];

			mat4 operator*(const mat4& other);
			v4 operator*(const v4& vec);
		};

		inline void PerspectiveDivide(v3& vec, real32 w)
		{
			vec.x /= w;
			vec.y /= w;
			vec.z /= w;
		}

		inline void PerspectiveDivide(v4* vec)
		{
			//If w is 0 then we have a problem :P
			ASSERT(vec->w != 0);
			vec->x /= vec->w;
			vec->y /= vec->w;
			vec->z /= vec->w;
		}

		void InitIdentity(mat4& mat);
		void InitScreenSpace(mat4& mat, real32 halfWidth, real32 halfHeight);
		void InitTranslation(mat4& mat, v3 vec);
		void InitRotation(mat4& mat, v3 axis, real32 angle);
		void InitScale(mat4& mat, v3 scale);
		void InitPerspective(mat4& mat, real32 fov, real32 aspect, real32 zNear, real32 zFar);
		void InitOrthographic(mat4& mat, real32 left, real32 right, real32 bottom, real32 top, real32 near, real32 far);
		void InitLookAt(mat4& mat, v3 eye, v3 center, v3 up);

		inline real32 ToRadians(real32 deg)
		{
			return (deg * Pi32) / 180;
		}

		inline real32 ToDegrees(real32 rad)
		{
			return (rad * 180) / Pi32;
		}

		typedef v2 vec2;
		typedef v3 vec3;
		typedef v4 vec4;
	}
}