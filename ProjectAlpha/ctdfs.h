#pragma once

#include <stdint.h>
#include <glm/glm.hpp>

//Matrices
using glm::mat3;
using glm::mat4;

using glm::quat;

//Vectors
using glm::vec2;
using glm::vec3;
using glm::vec4;

#define ASSERT(expr) if(!expr) { \
	((int*)0)[0] = 0; \
		}

#define Kilobytes(Value) (Value * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;