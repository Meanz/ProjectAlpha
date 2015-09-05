#include "VecMath.h"

namespace ProjectAlpha
{
	namespace Math
	{


		//Vec2 functions

		v2 v2::operator/(const real32& by)
		{
			return v2{ x / by, y / by };
		}

		v2 v2::operator+(const v2& other)
		{
			return v2{ other.x + x, other.y + y };
		}

		v2 v2::operator-(const v2& other)
		{
			return v2{ other.x - x, other.y - y };
		}

		real32 Cross(v2 v1, v2 v2)
		{
			return v1.x * v2.y - v2.x * v1.y;
		}

		real32 Length(v2 v1)
		{
			return sqrt((v1.x * v1.x) + (v1.y * v1.y));
		}

		real32 Dist(v2 v1, v2 v2)
		{
			return Length(v2 - v1);
		}

		//Vec3 functions

		v3 v3::operator/(const real32& by)
		{
			return v3{ x / by, y / by, z / by };
		}

		v3 v3::operator+(const v3& other)
		{
			return v3{ other.x + x, other.y + y, other.z + z };
		}

		v3 v3::operator-(const v3& other)
		{
			return v3{ x - other.x, y - other.y, z - other.z };
		}

		v3 Cross(v3 v1, v3 v2)
		{
			return v3{
				v1.y * v2.z - v1.z * v2.y,
				v1.z * v2.x - v1.x * v2.z,
				v1.x * v2.y - v1.y * v2.x
			};
		}

		real32 Length(v3 v1)
		{
			return sqrt((v1.x * v1.x) + (v1.y * v1.y) + (v1.z * v1.z));
		}

		real32 Dist(v3 v1, v3 v2)
		{
			return Length(v2 - v1);
		}

		v3 Normalize(v3 in)
		{
			real32 len = 1.0f / Length(in);
			return v3(in.x *= len,
				in.y *= len,
				in.z *= len);
		}

		real32 Dot(v3 l, v3 r)
		{
			return (l.x * r.x + l.y * r.y + l.z * r.z);
		}

		//Vec4 functions

		v4 v4::operator/(const real32& by)
		{
			return v4{ x / by, y / by, z / by, w / by };
		}

		v4 v4::operator+(const v4& other)
		{
			return v4{ other.x + x, other.y + y, other.z + z, other.w + other.w };
		}

		v4 v4::operator-(const v4& other)
		{
			return v4{ x - other.x, y - other.y, z - other.z, other.w - other.w };
		}

		v4 v4::operator*(const r32& other)
		{
			return v4{ x * other, y * other, z * other, w * other };
		}

		real32 Length(v4 v1)
		{
			return sqrt((v1.x * v1.x) + (v1.y * v1.y) + (v1.z * v1.z));
		}

		v4 Cross(v4 v1, v4 v2)
		{
			return v4{
				v1.y * v2.z - v1.z * v2.y,
				v1.z * v2.x - v1.x * v2.z,
				v1.x * v2.y - v1.y * v2.x,
				0
			};
		}

		v4 Normalize(v4 in)
		{
			real32 len = Length(in);
			return v4(in.x /= len,
				in.y /= len,
				in.z /= len,
				in.w /= len);
		}

		real32 Dot(v4 l, v4 r)
		{
			return (l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w);
		}

		//mat4 functions
#define m(x, y) mat.values[x + (y * 4)]
#define l(x, y) values[x + (y *4 )]
#define r(x, y) other.values[x + (y * 4)]
		mat4 mat4::operator*(const mat4& other)
		{
			mat4 mat;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					m(i, j) = l(i, 0) * r(0, j) +
						l(i, 1) * r(1, j) +
						l(i, 2) * r(2, j) +
						l(i, 3) * r(3, j);
				}
			}
			return mat;
		}

		v4 mat4::operator*(const v4& other)
		{
			return v4{ l(0, 0) * other.x + l(0, 1) * other.y + l(0, 2) * other.z + l(0, 3) * other.w,
				l(1, 0) * other.x + l(1, 1) * other.y + l(1, 2) * other.z + l(1, 3) * other.w,
				l(2, 0) * other.x + l(2, 1) * other.y + l(2, 2) * other.z + l(2, 3) * other.w,
				l(3, 0) * other.x + l(3, 1) * other.y + l(3, 2) * other.z + l(3, 3) * other.w };
		}


		void InitIdentity(mat4& mat)
		{
			m(0, 0) = 1;	m(0, 1) = 0;	m(0, 2) = 0;	m(0, 3) = 0;
			m(1, 0) = 0;	m(1, 1) = 1;	m(1, 2) = 0;	m(1, 3) = 0;
			m(2, 0) = 0;	m(2, 1) = 0;	m(2, 2) = 1;	m(2, 3) = 0;
			m(3, 0) = 0;	m(3, 1) = 0;	m(3, 2) = 0;	m(3, 3) = 1;
		}

		void InitScreenSpace(mat4& mat, real32 halfWidth, real32 halfHeight)
		{
			m(0, 0) = -halfWidth;	m(0, 1) = 0;	m(0, 2) = 0;	m(0, 3) = halfWidth - 0.5f;
			m(1, 0) = 0;	m(1, 1) = halfHeight;	m(1, 2) = 0;	m(1, 3) = halfHeight - 0.5f;
			m(2, 0) = 0;	m(2, 1) = 0;	m(2, 2) = 1;	m(2, 3) = 0;
			m(3, 0) = 0;	m(3, 1) = 0;	m(3, 2) = 0;	m(3, 3) = 1;
		}

		void InitTranslation(mat4& mat, v3 vec)
		{
			m(0, 0) = 1;	m(0, 1) = 0;	m(0, 2) = 0;	m(0, 3) = vec.x;
			m(1, 0) = 0;	m(1, 1) = 1;	m(1, 2) = 0;	m(1, 3) = vec.y;
			m(2, 0) = 0;	m(2, 1) = 0;	m(2, 2) = 1;	m(2, 3) = vec.z;
			m(3, 0) = 0;	m(3, 1) = 0;	m(3, 2) = 0;	m(3, 3) = 1;
		}

		void InitRotation(mat4& mat, v3 axis, real32 angle)
		{
			real32 _sin = (real32)sin(angle);
			real32 _cos = (real32)cos(angle);
			m(0, 0) = _cos + axis.x*axis.x*(1 - _cos); m(0, 1) = axis.x*axis.y*(1 - _cos) - axis.z*_sin; m(0, 2) = axis.x*axis.z*(1 - _cos) + axis.y*_sin; m(0, 3) = 0;
			m(1, 0) = axis.y*axis.x*(1 - _cos) + axis.z*_sin; m(1, 1) = _cos + axis.y*axis.y*(1 - _cos);	m(1, 2) = axis.y*axis.z*(1 - _cos) - axis.x*_sin; m(1, 3) = 0;
			m(2, 0) = axis.z*axis.x*(1 - _cos) - axis.y*_sin; m(2, 1) = axis.z*axis.y*(1 - _cos) + axis.x*_sin; m(2, 2) = _cos + axis.z*axis.z*(1 - _cos); m(2, 3) = 0;
			m(3, 0) = 0;	m(3, 1) = 0;	m(3, 2) = 0;	m(3, 3) = 1;
		}

		void InitScale(mat4& mat, v3 scale)
		{
			m(0, 0) = scale.x;	m(0, 1) = 0;		m(0, 2) = 0;		m(0, 3) = 0;
			m(1, 0) = 0;		m(1, 1) = scale.y;	m(1, 2) = 0;		m(1, 3) = 0;
			m(2, 0) = 0;		m(2, 1) = 0;		m(2, 2) = scale.z;	m(2, 3) = 0;
			m(3, 0) = 0;		m(3, 1) = 0;		m(3, 2) = 0;		m(3, 3) = 1;
		}

		void InitPerspective(mat4& mat, real32 fov, real32 aspect, real32 zNear, real32 zFar)
		{
			real32 tanHalfFOV = (real32)tan(fov / 2);
			real32 zRange = zNear - zFar;

			m(0, 0) = 1.0f / (tanHalfFOV * aspect);	m(0, 1) = 0;					m(0, 2) = 0;						m(0, 3) = 0;
			m(1, 0) = 0;							m(1, 1) = 1.0f / tanHalfFOV;	m(1, 2) = 0;						m(1, 3) = 0;
			m(2, 0) = 0;							m(2, 1) = 0;					m(2, 2) = (-zNear - zFar) / zRange;	m(2, 3) = 2 * zFar * zNear / zRange;
			m(3, 0) = 0;							m(3, 1) = 0;					m(3, 2) = 1;						m(3, 3) = 0;

			if (true) return;
		}

		void InitOrthographic(mat4& mat, real32 left, real32 right, real32 bottom, real32 top, real32 near, real32 far)
		{
			real32 width = right - left;
			real32 height = top - bottom;
			real32 depth = far - near;

			m(0, 0) = 2 / width; m(0, 1) = 0;	m(0, 2) = 0;	m(0, 3) = -(right + left) / width;
			m(1, 0) = 0;	m(1, 1) = 2 / height; m(1, 2) = 0;	m(1, 3) = -(top + bottom) / height;
			m(2, 0) = 0;	m(2, 1) = 0;	m(2, 2) = -2 / depth; m(2, 3) = -(far + near) / depth;
			m(3, 0) = 0;	m(3, 1) = 0;	m(3, 2) = 0;	m(3, 3) = 1;
		}

		void InitLookAt(mat4& mat, v3 eye, v3 center, v3 up)
		{
			InitIdentity(mat);
			v3 f = Normalize(center - eye);
			v3 u = Normalize(up);
			v3 s = Normalize(Cross(f, u));
			u = Cross(s, f);

			m(0, 0) = s.x;
			m(0, 1) = s.y;
			m(0, 2) = s.z;

			m(1, 0) = u.x;
			m(1, 1) = u.y;
			m(1, 2) = u.z;

			m(2, 0) = -f.x;
			m(2, 1) = -f.y;
			m(2, 2) = -f.z;


			Translate(mat, v3(-eye.x, -eye.y, -eye.z));
		}

		void Translate(mat4& mat, v3 vec)
		{
			m(0, 3) += m(0, 0) * vec.x + m(0, 1) * vec.y + m(0, 2) * vec.z;
			m(1, 3) += m(1, 0) * vec.x + m(1, 1) * vec.y + m(1, 2) * vec.z;
			m(2, 3) += m(2, 0) * vec.x + m(2, 1) * vec.y + m(2, 2) * vec.z;
			m(3, 3) += m(3, 0) * vec.x + m(3, 1) * vec.y + m(3, 2) * vec.z;
		}

		void TranslateNoScale(mat4& mat, v3 v)
		{
			m(3, 0) += v.x;
			m(3, 1) += v.y;
			m(3, 2) += v.z;
		}

	}
}