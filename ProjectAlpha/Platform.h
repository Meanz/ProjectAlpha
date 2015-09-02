#pragma once

#include "ctdfs.h"

#define Align16(x) ((x + 15) & ~15)

//Remove this later
struct File
{
	char* name;
	void* data;
};

struct Platform
{

};
