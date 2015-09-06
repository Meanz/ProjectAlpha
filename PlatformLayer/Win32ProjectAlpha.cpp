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

typedef void WorkQueueCallback(PlatformWorkQueue* queue, void* data);
struct PlatformWorkQueueEntry
{
	WorkQueueCallback* Callback;
	void* Data;
};

struct PlatformWorkQueue
{
	uint32 volatile ComplectionGoal;
	uint32 volatile CompletionCount;

	uint32 volatile NextEntryToWrite;
	uint32 volatile NextEntryToRead;

	uint32 maxentryCount;
	HANDLE semaphoreHandle;
	PlatformWorkQueueEntry entries[256];
};

struct Win32ThreadInfo
{
	uint32 logicalThreadIndex;
	PlatformWorkQueue* queue;
};

void Win32AddEntry(PlatformWorkQueue* queue, WorkQueueCallback* func, void* data)
{
	uint32 newNextEntryToWrite = (queue->NextEntryToWrite + 1) % queue->maxentryCount;
	ASSERT((newNextEntryToWrite != queue->NextEntryToRead));
	PlatformWorkQueueEntry* entry = queue->entries + queue->NextEntryToWrite;
	entry->Data = data;
	entry->Callback = func;
	++queue->ComplectionGoal;
	_WriteBarrier();
	//_mm_sfence(); //RAD Folks said it wasn't needed
	queue->NextEntryToWrite = newNextEntryToWrite;
	ReleaseSemaphore(queue->semaphoreHandle, 1, 0);
}

PLATFORM_WORK_QUEUE_CALLBACK(DoWorkerWork)
{
	//Perform actual work
	char buffer[256];
	wsprintf(buffer, "Thread %u: %s\n", GetCurrentThreadId(), (char*)data);
	OutputDebugStringA(buffer);
}

bool32 Win32DoWorkQueueNextEntry(PlatformWorkQueue* queue)
{
	bool32 shouldSleep = false;
	//Dequeue entry
	uint32 originalNextEntryToDo = queue->NextEntryToRead;
	uint32 newNextEntryToDo = (originalNextEntryToDo + 1) % queue->maxentryCount;
	if (originalNextEntryToDo != queue->NextEntryToWrite)
	{
		uint32 index = InterlockedCompareExchange(
			(LONG volatile *)&queue->NextEntryToRead,
			newNextEntryToDo,
			originalNextEntryToDo
			);

		if (index == originalNextEntryToDo)
		{
			PlatformWorkQueueEntry entry = queue->entries[index];
			entry.Callback(queue, entry.Data);
			InterlockedIncrement((LONG volatile *)&queue->CompletionCount);
		}
	}
	else
	{
		shouldSleep = true;
	}
	return(shouldSleep);
}

void Win32CompleteAllWork(PlatformWorkQueue* queue)
{
	while (queue->ComplectionGoal != queue->CompletionCount)
	{
		Win32DoWorkQueueNextEntry(queue);
	};

	queue->ComplectionGoal = 0;
	queue->CompletionCount = 0;
}

DWORD WINAPI ThreadProc(LPVOID param)
{
	Win32ThreadInfo* threadInfo = (Win32ThreadInfo*)param;
	for (;;)
	{
		if (Win32DoWorkQueueNextEntry(threadInfo->queue))
		{
			WaitForSingleObjectEx(threadInfo->queue->semaphoreHandle, INFINITE, FALSE);
		}
	}
	return 0;
}

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
	HWND hwnd = CreateWindowEx(0, className, windowName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

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
	buffer->stride = Align16(width * buffer->bytesPerPixel);
	buffer->memorySize = buffer->stride * buffer->height;

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
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
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

	inFile.open((char*)file.name, std::ios::binary);

	std::ifstream::pos_type begin, end;
	if (inFile.is_open())
	{
		//find file size
		begin = inFile.tellg();
		inFile.seekg(0, std::ios::end);
		end = inFile.tellg();
		inFile.seekg(0, std::ios::beg);

		uint32 size = (uint32)(end - begin);

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
	//gameState.platform.PlatformReadFile = __Win32ReadFile;
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

	//Threading!
	const uint32 threadCount = 7;
	PlatformWorkQueue queue = {};
	queue.maxentryCount = 256;
	Win32ThreadInfo threadInfo[threadCount];

	const uint32 initialCount = 0;
	HANDLE semaphoreHandle = CreateSemaphoreEx(0, initialCount, threadCount, 0, 0, SEMAPHORE_ALL_ACCESS);

	queue.semaphoreHandle = semaphoreHandle;

	//Tell our game memory about these things
	gameMemory->HighPriorityQueue = &queue;
	gameMemory->PlatformAddEntry = Win32AddEntry;
	gameMemory->PlatformCompleteAllWork = Win32CompleteAllWork;

	//MAEK TREDS
	for (uint32 threadIndex = 0; threadIndex < threadCount; threadIndex++)
	{
		Win32ThreadInfo* info = threadInfo + threadIndex;
		info->logicalThreadIndex = threadIndex;
		info->queue = &queue;

		DWORD threadId;
		HANDLE threadHandle = CreateThread(0, 0, ThreadProc, info, 0, &threadId);
		CloseHandle(threadHandle); //Does not close the thread, only releases the handle
	}

	Win32AddEntry(&queue, DoWorkerWork, "String A0");
	Win32AddEntry(&queue, DoWorkerWork, "String A1");
	Win32AddEntry(&queue, DoWorkerWork, "String A2");
	Win32AddEntry(&queue, DoWorkerWork, "String A3");
	Win32AddEntry(&queue, DoWorkerWork, "String A4");
	Win32AddEntry(&queue, DoWorkerWork, "String A5");

	Sleep(1000);

	Win32AddEntry(&queue, DoWorkerWork, "String A6");
	Win32AddEntry(&queue, DoWorkerWork, "String A7");
	Win32AddEntry(&queue, DoWorkerWork, "String A8");
	Win32AddEntry(&queue, DoWorkerWork, "String A9");
	
	Win32CompleteAllWork(&queue);

	if (gameCode.gameRender)
	{
		gameCode.gameInit(gameMemory, &gameState);

		running = true;

		const uint64 ticksPerSecond = 25;
		const uint64 skipTicks = 1000 / ticksPerSecond;
		const uint64 maxFrameskip = 5;

		uint64 nextTick = GetTickCount64();
		int64 loops = 0;
		real32 interpolation = 0.0f;

		while (running)
		{

			while (GetTickCount64() > nextTick && loops < maxFrameskip)
			{

				nextTick = skipTicks;
				loops++;
			}

			//Window message parsing
			MSG msg;
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT) {
					running = false;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			//update
			int64 start = GetTickCount64();

			//Because
			gameState.pixelBuffer.memory = backbuffer.memory;
			gameState.pixelBuffer.width = backbuffer.width;
			gameState.pixelBuffer.height = backbuffer.height;
			gameState.pixelBuffer.size = backbuffer.memorySize;

			//Do modifications of our pixel buffer
			gameCode.gameRender(gameMemory, &gameState);

			//How long did the render op take?
			int64 delta = GetTickCount64() - start;

			char buf[10];
			_itoa_s(delta, buf, 10);
			OutputDebugStringA("## Frame ##\n");
			OutputDebugStringA("Blit Time: ");
			OutputDebugStringA(buf);
			OutputDebugStringA(" ms");
			OutputDebugStringA("\n");

			OutputDebugStringA("Cycle Debug: ");
			buf[200];
			_itoa_s(gameMemory->_cycles, buf, 10);
			OutputDebugStringA(buf);
			OutputDebugStringA(" microseconds\n");

			//Blit the pixelbuffer to the screen
			HDC hdc = GetDC(window);
			Win32WindowDimension dim = __Win32GetWindowDimension(window);
			__Win32BlitBuffer(backbuffer, hdc, dim);

			ReleaseDC(window, hdc);

			Sleep(1);
		}
	}

	//Do dealloc
	VirtualFree(gameMemory, NULL, MEM_RELEASE);

	return 0;
}