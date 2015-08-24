#pragma once

#include "VecMath.h"

/* MD2 header */
struct md2_header_t
{
	int ident;                  /* magic number: "IDP2" */
	int version;                /* version: must be 8 */

	int skinwidth;              /* texture width */
	int skinheight;             /* texture height */

	int framesize;              /* size in bytes of a frame */

	int num_skins;              /* number of skins */
	int num_vertices;           /* number of vertices per frame */
	int num_st;                 /* number of texture coordinates */
	int num_tris;               /* number of triangles */
	int num_glcmds;             /* number of opengl commands */
	int num_frames;             /* number of frames */

	int offset_skins;           /* offset skin data */
	int offset_st;              /* offset texture coordinate data */
	int offset_tris;            /* offset triangle data */
	int offset_frames;          /* offset frame data */
	int offset_glcmds;          /* offset OpenGL command data */
	int offset_end;             /* offset end of file */
};

/* Texture name */
struct md2_skin_t
{
	char name[64];              /* texture file name */
};

/* Texture coords */
struct md2_texCoord_t
{
	short s;
	short t;
};

struct md2_triangle_t
{
	unsigned short vertex[3];   /* vertex indices of the triangle */
	unsigned short st[3];       /* tex. coord. indices */
};

struct md2_model
{
	md2_header_t header;
	
	md2_skin_t* skins;
	
	vec2* texcoords;
	vec3* vertices;
	uint8* normals;
	md2_triangle_t* triangles;

};

md2_model loadModel(void* modelData);