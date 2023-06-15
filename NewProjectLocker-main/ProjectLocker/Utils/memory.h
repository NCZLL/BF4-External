#pragma once
#include <Windows.h>
#include <iostream>

class Memory
{
public:
	HWND Ghwnd;
	DWORD PID;
	HANDLE pHandle;
	char ClassName[128];
	char MenuName[128];
	
	template <typename T>
	constexpr const T RPM(const uintptr_t& adder) const noexcept
	{
		T v = { };
		ReadProcessMemory(pHandle, reinterpret_cast<const void*>(adder), &v, sizeof(T), NULL);
		return v;
	}

	template <typename T>
	constexpr void WPM(const uintptr_t& adder, const T& v) const noexcept
	{
		WriteProcessMemory(pHandle, reinterpret_cast<void*>(adder), &v, sizeof(T), NULL);
	}
};