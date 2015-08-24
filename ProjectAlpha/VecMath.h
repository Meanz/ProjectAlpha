#pragma once

#include "ctdfs.h"
#include <iostream>

struct vec2
{
	real32 x;
	real32 y;

	vec2 operator/(const real32& by)
	{
		return vec2{ x / by, y / by };
	}

	vec2 operator+(const vec2& other)
	{
		return vec2{ other.x + x, other.y + y };
	}

	vec2 operator-(const vec2& other)
	{
		return vec2{ other.x - x, other.y - y };
	}
};

real32 Cross(vec2 v1, vec2 v2);
real32 Length(vec2 v1);
real32 Dist(vec2 v1, vec2 v2);

struct vec3
{

	real32 x;
	real32 y;
	real32 z;

	vec3 operator/(const real32& by)
	{
		return vec3{ x / by, y / by, z / by };
	}

	vec3 operator+(const vec3& other)
	{
		return vec3{ other.x + x, other.y + y, other.z + z };
	}

	vec3 operator-(const vec3& other)
	{
		return vec3{ other.x - x, other.y - y, other.z - z };
	}
};

vec3 Cross(vec3 v1, vec3 v2);
real32 Length(vec3 v1);
real32 Dist(vec3 v1, vec3 v2);

struct vec4
{
	real32 x;
	real32 y;
	real32 z;
	real32 w;

	vec4 operator/(const real32& by)
	{
		return vec4{ x / by, y / by, z / by, w / by };
	}

	vec4 operator+(const vec4& other)
	{
		return vec4{ other.x + x, other.y + y, other.z + z, other.w + other.w };
	}

	vec4 operator-(const vec4& other)
	{
		return vec4{ other.x - x, other.y - y, other.z - z, other.w - other.w };
	}
};

real32 Length(vec4 v1);
vec4 Cross(vec4 v1, vec4 v2);

struct mat3
{
	vec3 rows[3];
};

struct mat4
{
	//column major
	real32 values[16];

	mat4() {
	}

	mat4 operator*(const mat4& other)
	{
		mat4 res;
		for (int32 y = 0; y < 4; y++)
		{
			for (int32 x = 0; x < 4; x++)
			{
				res.values[x + (y * 4)] =
					values[x + (0 * 4)] * other.values[0 + (y * 4)] +
					values[x + (1 * 4)] * other.values[1 + (y * 4)] +
					values[x + (2 * 4)] * other.values[2 + (y * 4)] +
					values[x + (3 * 4)] * other.values[3 + (y * 4)]
				;
			}
		}
		return res;
	}

	vec4 operator*(const vec4& vec)
	{     
#define m(y, x) values[x + (y * 4)]
		//mat[y][x]
		return vec4
		{
			m(0, 0)	* vec.x + m(0, 1) * vec.y + m(0, 2) * vec.z + m(0, 3) * vec.w,
			m(1, 0)	* vec.x + m(1, 1) * vec.y + m(1, 2) * vec.z + m(1, 3) * vec.w,
			m(2, 0)	* vec.x + m(2, 1) * vec.y + m(2, 2) * vec.z + m(2, 3) * vec.w,
			m(3, 0)	* vec.x + m(3, 1) * vec.y + m(3, 2) * vec.z + m(3, 3) * vec.w
		};
	}
};

inline void PerspectiveDivide(vec3& vec, real32 w)
{
	vec.x /= w;
	vec.y /= w;
	vec.z /= w;
}

inline void PerspectiveDivide(vec4& vec)
{
	//If w is 0 then we have a problem :P
	ASSERT(vec.w != 0);
	vec.x /= vec.w;
	vec.y /= vec.w;
	vec.z /= vec.w;
}

inline void InitIdentity(mat4& mat) {
	((vec4*)mat.values)[0] = vec4{ 1, 0, 0, 0 };
	((vec4*)mat.values)[1] = vec4{ 0, 1, 0, 0 };
	((vec4*)mat.values)[2] = vec4{ 0, 0, 1, 0 };
	((vec4*)mat.values)[3] = vec4{ 0, 0, 0, 1 };
}

inline void InitScreenSpace(mat4& mat, real32 halfWidth, real32 halfHeight)
{
	((vec4*)mat.values)[0] = vec4{ halfWidth,	0,				0,		halfWidth };
	((vec4*)mat.values)[1] = vec4{ 0,			-halfHeight,	0,		halfHeight };
	((vec4*)mat.values)[2] = vec4{ 0,			0,				1,		0 };
	((vec4*)mat.values)[3] = vec4{ 0,			0,				0,		1 };
}

inline vec3 Transform(mat4& mat, vec3 vec)
{
	return vec3
	{
		mat.values[0] * vec.x + mat.values[1] * vec.y + mat.values[2] * vec.z + mat.values[3],
		mat.values[4] * vec.x + mat.values[5] * vec.y + mat.values[6] * vec.z + mat.values[7],
		mat.values[8] * vec.x + mat.values[9] * vec.y + mat.values[10] * vec.z + mat.values[11],
	};
}

inline void InitTranslation(mat4& mat, vec3 vec)
{
	((vec4*)mat.values)[0] = vec4{ 1, 0, 0, vec.x };
	((vec4*)mat.values)[1] = vec4{ 0, 1, 0, vec.y };
	((vec4*)mat.values)[2] = vec4{ 0, 0, 1, vec.z };
	((vec4*)mat.values)[3] = vec4{ 0, 0, 0, 1 };
}

inline void InitRotation(mat4& mat, vec3 axis, real32 angle)
{
	real32 _sin = sin(angle);
	real32 _cos = cos(angle);
	((vec4*)mat.values)[0] = vec4{ _cos + axis.x * axis.x * (1 - _cos),				axis.x * axis.y *(1 - _cos) - axis.z * _sin,	axis.x*axis.z*(1 - _cos) + axis.y*_sin, 0 };
	((vec4*)mat.values)[1] = vec4{ axis.y * axis.x * (1 - _cos) + axis.z * _sin,	_cos + axis.y * axis.y*(1 - _cos),				axis.y*axis.z*(1 - _cos) - axis.x*_sin, 0 };
	((vec4*)mat.values)[2] = vec4{ axis.z * axis.x * (1 - _cos) - axis.y * _sin,	axis.z * axis.y* (1 - _cos) + axis.x * _sin,	_cos + axis.z*axis.z*(1 - _cos),		0 };
	((vec4*)mat.values)[3] = vec4{ 0,												0,												0,										1 };
}

inline void InitRotation(mat4& mat, vec4 forward, vec4 up, vec4 right)
{
	((vec4*)mat.values)[0] = vec4{ right.x,		right.y,	right.z,	0 };
	((vec4*)mat.values)[1] = vec4{ up.x,		up.y,		up.z,		0 };
	((vec4*)mat.values)[2] = vec4{ forward.x,	forward.y,  forward.z,	0 };
	((vec4*)mat.values)[3] = vec4{ 0,			0,			0,			1 };
}

inline void InitRotation(mat4& mat, vec4 forward, vec4 up)
{
	vec4 f = forward / Length(forward);
	vec4 r = up / Length(up);
	r = Cross(r, f);
	vec4 u = Cross(f, r);
	return InitRotation(mat, f, u, r);
}

inline void InitPerspective(mat4& mat, real32 fov, real32 aspect, real32 zNear, real32 zFar)
{
	real32 tanHalfFov = (real32)tan(fov / 2);
	real32 zRange = zNear - zFar;
	((vec4*)mat.values)[0] = vec4{ 1.0f / (tanHalfFov * aspect),	0,					0,							0 };
	((vec4*)mat.values)[1] = vec4{ 0,								1.0f / tanHalfFov,	0,							0 };
	((vec4*)mat.values)[2] = vec4{ 0,								0,					(-zNear - zFar) / zRange,	2 * zFar * zNear / zRange };
	((vec4*)mat.values)[3] = vec4{ 0,								0,					1,							0 };
}

inline real32 ToRadians(real32 deg)
{
	return (deg * Pi32) / 180;
}

inline real32 ToDegrees(real32 rad)
{
	return (rad * 180) / Pi32;
}