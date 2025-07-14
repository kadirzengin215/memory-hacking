# C++ Memory hacking

A simple c++ project for memory hacking.

-   [Usage](#usage)
    -   [External](#external)
        -   [Using with memory instance](#using-with-memory-instance)
            -   [Writing or Reading string values](#writing-or-reading-string-values)
            -   [Checking attach process](#checking-attach-process)
            -   [Getting module informations](#getting-module-informations)
            -   [Other helpful methods](#other-helpful-methods)
        -   [Using with static methods](#using-with-static-methods)
            -   [Writing or Reading string values with static methods](#writing-or-reading-string-values-with-static-methods)
            -   [Getting module informations with static methods](#getting-module-informations-with-static-methods)
-   [License](#license)

## Usage

### External

#### Using with memory instance.

```cpp
#include <iostream>
#include "Memory.h"

int main() {
	// Create an instance of the Memory class for the target process "ac_client.exe"
	Memory memory(L"ac_client.exe");

	// Check if the Memory object is successfully attached to the process
	if (!memory.isAttached()) {
		// If not attached, print an error message and exit
		std::cout << "Failed to attach to process";

		return 0;
	}

	// Resolve the player health address using address (with out base address) and offsets
	uintptr_t playerHealthAddress = memory.GetAddress(0x17E0A8, { 0xEC });

	// Change player health to 999 by writing the value to the resolved address
	memory.Write<int>(playerHealthAddress, 999);

	// Read the player health from memory and print it to the console
	std::cout << "Player health: " << memory.Read<int>(playerHealthAddress);

	return 0;
}
```

##### Writing or Reading string values

```cpp
#include <iostream>
#include "Memory.h"

int main() {
	// Create an instance of the Memory class for the target process "ac_client.exe"
	Memory memory(L"ac_client.exe");

	// Resolve the player name address using base address and offset
	uintptr_t playerNameAddress = memory.GetAddress(0x17E0A8, { 0x205 });

	// Write the string "MitraX" to the resolved player name address in the target process
	memory.WriteString(playerNameAddress, "MitraX");

	// Read the player name from memory and print it to the console
	std::cout << "Player name: " << memory.ReadString(playerNameAddress) << std::endl;

	return 0;
}
```

##### Checking attach process

```cpp
#include <iostream>
#include "Memory.h"

int main() {
	// Create an instance of the Memory class for the target process "ac_client.exe"
  	Memory memory(L"ac_client.exe");

	// Check if the Memory object is successfully attached to the process
	if (!memory.isAttached()) {

		while(!memory.isAttached()) {
			// If not attached, print an error message
			std::cout << "Failed to attach to process error: " + memory.GetErrorMessage() + " trying again..."
			<< std::endl;

			// Wait for a short period before retrying
			Sleep(1000); // Sleep for 1 second

			// Attempt to reattach
			memory.attachProcess(L"ac_client.exe");
		}
  	}

	// If successfully attached, print the process name
	std::cout << "Attached to process: " << memory.GetProcessName() << std::endl;

	return 0;
}
```

##### Getting module informations

```cpp
#include <iostream>
#include "Memory.h"

int main() {
	// Create an instance of the Memory class for the target process "ac_client.exe"
	Memory memory(L"ac_client.exe");

	// Retrieve information about the main module of the attached process
	MODULEINFO moduleInfo = memory.GetModuleInfo();

	// Get the base address of the module
	uintptr_t base = (uintptr_t)moduleInfo.lpBaseOfDll;

	// Get the size of the module's image in memory
	DWORD size = moduleInfo.SizeOfImage;

	// Get the entry point address of the module
	uintptr_t entry = (uintptr_t)moduleInfo.EntryPoint;

	// Print the base address in hexadecimal format
	std::cout << "Base Address   : 0x" << std::hex << base << std::endl;

	// Print the size of the image in hexadecimal format
	std::cout << "Size of Image  : 0x" << std::hex << size << std::endl;

	// Print the entry point address in hexadecimal format
	std::cout << "Entry Point    : 0x" << std::hex << entry << std::endl;

	return 0;
}
```

##### Other helpful methods

```cpp
#include <iostream>
#include "Memory.h"

int main() {
	// Create an instance of the Memory class for the target process "ac_client.exe"
	Memory memory(L"ac_client.exe");

	// Retrieve the process handle (used to read/write the memory of the target process)
	HANDLE process = memory.GetProcess();

	// Get the wide string (UTF-16) version of the process name
	const wchar_t* processNameW = memory.GetProcessNameW();

	// Print the process name to the console
	std::cout << "Process name: " << memory.GetProcessName() << std::endl;

	// Print the process ID (PID) to the console
	std::cout << "Process ID: " << memory.GetProcessID() << std::endl;

	// Print the base address of the main module (usually the .exe file) in the target process
	std::cout << "Module base address: " << memory.GetModuleBaseAddress() << std::endl;

	return 0;
}
```

#### Using with static methods

```cpp
#include <iostream>
#include "Memory.h"

int main() {
	// Define the target process name (must match the window or executable name exactly)
	const wchar_t* processName = L"ac_client.exe";

	// Get the Process ID (PID) of the target process using its name
	DWORD processID = Memory::GetProcessID(processName);

	// Open a handle to the process with all access rights (required to read/write memory)
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

	// Get the base address of the main module (the executable) in the process's memory
	uintptr_t moduleBaseAddress = Memory::GetModuleBaseAddress(processID, processName);

	// Calculate the dynamic address of the player's health using pointer dereferencing
	// Starting from base + 0x17E0A8, follow pointer chain { 0xEC } (i.e., *(base + 0x17E0A8) + 0xEC)
	uintptr_t playerHealthAddress = Memory::GetAddress(process, moduleBaseAddress + 0x17E0A8, { 0xEC });

	// Write a new health value (999) to the calculated address
	Memory::Write(process, playerHealthAddress, 999);

	// Read the value back from memory to verify and print it to the console
	std::cout << "Player health: " << Memory::Read<int>(process, playerHealthAddress);

	return 0;
}
```

##### Writing or Reading string values with static methods

```cpp
#include <iostream>
#include "Memory.h"

int main() {
	// Define the target process name (must match the window or executable name exactly)
	const wchar_t* processName = L"ac_client.exe";

	// Get the Process ID (PID) of the target process using its name
	DWORD processID = Memory::GetProcessID(processName);

	// Open a handle to the process with all access rights (required to read/write memory)
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

	// Get the base address of the main module (the executable) in the process's memory
	uintptr_t moduleBaseAddress = Memory::GetModuleBaseAddress(processID, processName);

	// Resolve the player name address using base address and offset
	uintptr_t playerNameAddress = Memory::GetAddress(moduleBaseAddress + 0x17E0A8, { 0x205 });

	// Write the string "MitraX" to the resolved player name address in the target process
	Memory::WriteString(process,playerNameAddress, "MitraX");

	// Read the player name from memory and print it to the console
	std::cout << "Player name: " << memory.ReadString(process,playerNameAddress) << std::endl;

	return 0;
}
```

##### Getting module informations with static methods

```cpp
#include <iostream>
#include "Memory.h"

int main() {
	// Define the target process name (must match the window or executable name exactly)
	const wchar_t* processName = L"ac_client.exe";

	// Get the Process ID (PID) of the target process using its name
	DWORD processID = Memory::GetProcessID(processName);

	// Open a handle to the process with all access rights (required to read/write memory)
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

	// Get the base address of the main module (the executable) in the process's memory
	uintptr_t moduleBaseAddress = Memory::GetModuleBaseAddress(processID, processName);

	// Retrieve information about the main module of the attached process
	MODULEINFO moduleInfo = Memory::GetModuleInfo(process,(HMODULE)moduleBaseAddress);

	// Get the base address of the module
	uintptr_t base = (uintptr_t)moduleInfo.lpBaseOfDll;

	// Get the size of the module's image in memory
	DWORD size = moduleInfo.SizeOfImage;

	// Get the entry point address of the module
	uintptr_t entry = (uintptr_t)moduleInfo.EntryPoint;

	// Print the base address in hexadecimal format
	std::cout << "Base Address   : 0x" << std::hex << base << std::endl;

	// Print the size of the image in hexadecimal format
	std::cout << "Size of Image  : 0x" << std::hex << size << std::endl;

	// Print the entry point address in hexadecimal format
	std::cout << "Entry Point    : 0x" << std::hex << entry << std::endl;

	return 0;
}
```

## License

This project is open-sourced software licensed under the [MIT license](LICENSE).
