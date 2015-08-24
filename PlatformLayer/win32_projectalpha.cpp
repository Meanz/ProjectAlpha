#include <ProjectAlpha.h>
#include <Windows.h>
#include <iostream>
#include <ostream>
#include <fstream>

struct Win32GameCode
{
	HMODULE gameCodeDLL;
	GameInit* gameInit;
	GameUpdateAndRender* gameRender;
};

struct Win32OffscreenBuffer 
{
	BITMAPINFO info;
	void* memory;
	int32 memorySize;
	int32 width;
	int32 height;
	int32 bytesPerPixel;
	int32 stride;
};

struct Win32WindowDimension
{
	int32 width;
	int32 height;
};


global_variable bool running = false;
global_variable Win32OffscreenBuffer backbuffer;

//wndproc
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

inline Win32WindowDimension __Win32GetWindowDimension(HWND hwnd) {
	Win32WindowDimension dim;
	RECT rect;
	GetClientRect(hwnd, &rect);
	dim.width = rect.right - rect.left;
	dim.height = rect.bottom - rect.top;
	return dim;
}

void RenderWeirdGradient(Win32OffscreenBuffer* buffer, int32 xOffset, int32 yOffset) {
	uint8* row = (uint8*)buffer->memory;
	for (int y = 0; y < buffer->height; y++) {
		uint32* pixel = (uint32*)row;
		for (int x = 0; x < buffer->width; x++) {
			//00 ? RR GG BB
			//*pixel = 0x0000ffff;

			// 
			// uint32 = 0 x XX RR GG BB
			// uint8  = BB GG RR xx
			// Windows folks wanted to see it as RGB in the register
			// Little endian order thing! it's opposite!
			//

			uint8 r = (x + xOffset);
			uint8 g = (y + yOffset);
			uint8 b = 0;
			uint8 pad = 0;

			//0 x XX RR GG BB
			*pixel++ = ((pad << 24) | (r << 16) | (g << 8) | b);
		}
		row += buffer->stride;
	}

}

//Window alloc
HWND __Win32CreateWindow(HINSTANCE hInstance, LPCSTR windowName, int32 width, int32 height)
{
	const LPCSTR className = "ProjectAlphaWindowClass";

	WNDCLASS wndclass = {}; //zero
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WindowProc; //We need a wndproc
	wndclass.hInstance = hInstance; //Hinstance from winmain? Is needed?
	wndclass.lpszClassName = className;

	RegisterClass(&wndclass);

	//Create the window.
	HWND hwnd = CreateWindowEx(0, className, windowName, WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

	if (!hwnd) {
		UnregisterClass(className, hInstance);
		return NULL;
	}

	ShowWindow(hwnd, SW_SHOWDEFAULT);

	return hwnd;
}

void __Win32BlitBuffer(Win32OffscreenBuffer buffer, HDC hdc, Win32WindowDimension dim)
{
	StretchDIBits(hdc,
		0, 0, dim.width, dim.height, //dst
		0, 0, buffer.width, buffer.height, //src
		buffer.memory,
		&buffer.info,
		DIB_RGB_COLORS,
		SRCCOPY
		);
}

void __Win32ResizeDIBSection(Win32OffscreenBuffer* buffer, int32 width, int32 height)
{

	if (buffer->memory) {
		VirtualFree(buffer->memory, NULL, MEM_RELEASE);
	}

	buffer->width = width;
	buffer->height = height;
	buffer->bytesPerPixel = 4;
	buffer->memorySize = (width * height) * buffer->bytesPerPixel;
	buffer->stride = (width * buffer->bytesPerPixel);

	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = width;
	buffer->info.bmiHeader.biHeight = -height;
	buffer->info.bmiHeader.biPlanes = 1; //always 1
	buffer->info.bmiHeader.biBitCount = 32; //32bit RGBA + word align
	buffer->info.bmiHeader.biCompression = BI_RGB;
	buffer->info.bmiHeader.biSizeImage = 0; //If using compression
	buffer->info.bmiHeader.biXPelsPerMeter = 0; //used for printing? phys size
	buffer->info.bmiHeader.biYPelsPerMeter = 0; //used for printing? phys size
	buffer->info.bmiHeader.biClrUsed = 0; //used for index pals
	buffer->info.bmiHeader.biClrImportant = 0; //used for index pals

	//Get from the game memory?
	buffer->memory = VirtualAlloc(0, buffer->memorySize, MEM_COMMIT, PAGE_READWRITE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch (uMsg)
	{

	case WM_SIZE:
	{
		Win32WindowDimension dim = __Win32GetWindowDimension(hwnd);
		__Win32ResizeDIBSection(&backbuffer, dim.width, dim.height);
	}
	break;

	case WM_CLOSE:
	{
		//When the user presses close
		running = false;
		PostQuitMessage(0);
	}
	break;

	case WM_DESTROY:
	{
		//Window get's destroyed
		running = false;
		PostQuitMessage(0);
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		Win32WindowDimension dim = __Win32GetWindowDimension(hwnd);
		__Win32BlitBuffer(backbuffer, hdc, dim); //Copy the rect struct, don't pass it as a pointer

		EndPaint(hwnd, &ps);
	}
	break;

	default:
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	}
	return result;
}

void LoadGameCode(Win32GameCode* gameCode) {
	//LoadLibrary
	gameCode->gameCodeDLL = LoadLibrary("ProjectAlpha.dll");

	//GetProcAddress
	gameCode->gameRender = (GameUpdateAndRender*)GetProcAddress(gameCode->gameCodeDLL, "_GameUpdateAndRender");
	gameCode->gameInit = (GameInit*)GetProcAddress(gameCode->gameCodeDLL, "_GameInit");
}

bool __Win32ReadFile(File file)
{
	
	std::ifstream inFile;

	inFile.open((char*)file.name.text, std::ios::binary);

	std::ifstream::pos_type begin, end;
	if (inFile.is_open())
	{
		//find file size
		begin = inFile.tellg();
		inFile.seekg(0, std::ios::end);
		end = inFile.tellg();
		inFile.seekg(0, std::ios::beg);

		uint32 size = (uint32) (end - begin);

		//We need to alloc some data, luckily we have the memory!
		//Or maybe we don't, who cares, let's virtual alloc
		file.data = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		
		inFile.read((char*)file.data, size);

		inFile.close();
		return true;
	}
	else
	{
		//It's not open, no need to close!
		//inFile.close();
		return false;
	}
}

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow
	) {
	//Do alloc

	uint64 permanentMemorySize = Megabytes(64);
	uint64 transientMemorySize = Gigabytes(4);
	uint64 totalSize = permanentMemorySize + transientMemorySize;

	GameMemory* gameMemory = (GameMemory*)VirtualAlloc(0, (SIZE_T)totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	gameMemory->permanentMemory = (void*)((uint8*)(gameMemory + sizeof(GameMemory)));
	gameMemory->permanentMemorySize = permanentMemorySize - sizeof(GameMemory);
	gameMemory->transientMemory = (void*)((uint8*)gameMemory->permanentMemory + gameMemory->permanentMemorySize);
	gameMemory->transientMemorySize = transientMemorySize;
	

	//Setup state
	GameState gameState;
	gameState.platform.PlatformReadFile = __Win32ReadFile;
	gameState.pixelBuffer.memory = 0;
	gameState.pixelBuffer.width = 0;
	gameState.pixelBuffer.height = 0;
	gameState.pixelBuffer.size = 0;
	gameState.number = 5;

	//Create window
	HWND window = __Win32CreateWindow(hInstance, "ProjectAlpha", 1280, 960);

	//Create our dib section
	//CreateDIBSection()

	//Laod game code
	Win32GameCode gameCode;
	LoadGameCode(&gameCode);

	if (gameCode.gameRender)
	{
		gameCode.gameInit(gameMemory, &gameState);

		running = true;
		int xOffset = 0;
		int yOffset = 0;
		while (running)
		{
			MSG msg;
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT) {
					running = false;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			//Because
			gameState.pixelBuffer.memory = backbuffer.memory;
			gameState.pixelBuffer.width = backbuffer.width;
			gameState.pixelBuffer.height = backbuffer.height;
			gameState.pixelBuffer.size = backbuffer.memorySize;

			//Do modifications of our pixel buffer
			gameCode.gameRender(gameMemory, &gameState);

			//Blit the pixelbuffer to the screen
			HDC hdc = GetDC(window);
			Win32WindowDimension dim = __Win32GetWindowDimension(window);
			__Win32BlitBuffer(backbuffer, hdc, dim);

			ReleaseDC(window, hdc);			
			++xOffset;
		}
	}

	//Do dealloc
	VirtualFree(gameMemory, NULL, MEM_RELEASE);

	return 0;
}