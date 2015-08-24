#pragma once

#include "ctdfs.h"

struct SString
{
	int8* text;
	int16 length;
};

struct File
{
	SString name;
	void* data;
	uint32 size;
};


typedef bool(*_PlatformReadFile)(File file);
typedef void*(*_PlatformMemAlloc)(uint32 size);
typedef void*(*_PlatformMemDealloc)(uint32 address, uint32 size);

//Functions
struct Platform
{
	_PlatformReadFile PlatformReadFile;
	_PlatformMemAlloc PlatformMemAlloc;
	_PlatformMemDealloc PlatformMemDealloc;
};