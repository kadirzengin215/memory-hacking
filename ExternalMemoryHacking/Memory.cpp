#include "Memory.h"

Memory::Memory(const std::wstring processName) {
	// Attempt to attach to the process using the provided name.
	// This will initialize processID, process handle, and module base address if successful.
	this->attachProcess(processName);
}

Memory::~Memory() {
	// Check if the process handle is valid (not nullptr)
	if (process) {
		// Close the handle to the process to release system resources
		CloseHandle(process);
	}
}

DWORD Memory::GetProcessID(const wchar_t* processName) {
	// Process ID to return, default 0 (not found)
	DWORD processID = 0;

	// Take a snapshot of all processes in the system
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 processEntry;
		processEntry.dwSize = sizeof(processEntry); // Set the size before using the structure

		// Get the first process in the snapshot
		if (Process32First(hSnap, &processEntry)) {
			do {
				// Compare the process name (case-insensitive)
				if (!_wcsicmp(processEntry.szExeFile, processName)) {
					processID = processEntry.th32ProcessID; // Found, store the process ID
					break; // Exit loop since we found the process
				}
			} while (Process32Next(hSnap, &processEntry)); // Move to next process
		}
	}

	// Release the snapshot handle
	CloseHandle(hSnap);

	// Return the found process ID (or 0 if not found)
	return processID;
}

uintptr_t Memory::GetModuleBaseAddress(DWORD processID, const wchar_t* moduleName) {
	// Variable to store the base address of the module, default is 0 (not found)
	uintptr_t moduleBaseAddress = 0;

	// Take a snapshot of all modules in the specified process
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry); // Set the size before using the structure

		// Get the first module in the snapshot
		if (Module32First(hSnap, &modEntry)) {
			do {
				// Compare the module name (case-insensitive)
				if (!_wcsicmp(modEntry.szModule, moduleName)) {
					// Found the module, store its base address
					moduleBaseAddress = (uintptr_t)modEntry.modBaseAddr;
					break; // Exit loop since we found the module
				}
			} while (Module32Next(hSnap, &modEntry)); // Move to next module
		}
	}

	// Release the snapshot handle
	CloseHandle(hSnap);

	// Return the found module base address (or 0 if not found)
	return moduleBaseAddress;
}

MODULEINFO Memory::GetModuleInfo(HANDLE process, HMODULE hModule) {
    // Structure to hold module information (base address, size, entry point, etc.)
    MODULEINFO moduleInfo;

    // Query the module information from the target process.
    // The function fills the moduleInfo structure with relevant data.
    GetModuleInformation(process, hModule, &moduleInfo, sizeof(moduleInfo));

    // Return the filled MODULEINFO structure.
    return moduleInfo;
}

uintptr_t Memory::GetAddress(HANDLE process, uintptr_t address, std::vector<unsigned int> offsets) {
	// Iterate through each offset in the vector
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		// Read the memory at the current address into 'address' variable.
		if (!ReadProcessMemory(process, (BYTE*)address, &address, sizeof(address), 0)) {
			return 0; // If it fails, return 0 to indicate error.
		}

		// Add the current offset to the address to move to the next pointer in the chain
		address += offsets[i];
	}

	// Return the final resolved address after following all offsets
	return address;
}

std::string Memory::ReadString(HANDLE process, uintptr_t address, SIZE_T maxLength) {
	// Allocate a buffer to hold the string data read from the process memory
	char* buffer = new char[maxLength];

	// Initialize the buffer to zero to avoid garbage data
	ZeroMemory(buffer, maxLength);

	// Read maxLength bytes from the resolved address in the target process
	// If the read fails, buffer will remain zeroed, resulting in an empty string
	ReadProcessMemory(process, (LPCVOID)address, buffer, maxLength, NULL);

	// Construct a std::string from the buffer (will stop at the first null terminator)
	std::string result(buffer);

	// Free the allocated buffer to prevent memory leaks
	delete[] buffer;

	// Return the string read from the process memory
	return result;
}

bool Memory::WriteString(HANDLE process, uintptr_t address, const std::string value) {
	// Write the string value (including null terminator) to the resolved address in the target process.
	return WriteProcessMemory(process, (LPVOID)address, value.c_str(), value.length() + 1, NULL);
}

void Memory::attachProcess(const std::wstring processName) {
	// Store the process name as a std::wstring
	this->processName = processName;

	// Store a pointer to the wide string representation of the process name
	this->processNameW = processName.c_str();

	// Get the process ID of the target process by its name
	this->processID = Memory::GetProcessID(this->processNameW);

	if (this->processID == 0) {
		// If process ID is 0, it means the process was not found
		this->errorMessage = "Process id not found for " + this->GetProcessName();
		return; // Exit the function early
	}

	// Open a handle to the process with all access rights
	this->process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->processID);

	if (!this->process) {
		// If the process handle is invalid, set an error message
		this->errorMessage = "Failed to open process: " + this->GetProcessName();

		// Reset process ID to 0 to indicate failure
		this->processID = 0;

		return; // Exit the function early
	}

	// Get the base address of the main module of the process
	this->moduleBaseAddress = Memory::GetModuleBaseAddress(this->processID, this->processNameW);

	if (this->moduleBaseAddress == 0) {
		// If the module base address is 0, it means the module was not found
		this->errorMessage = "Module base address not found for process: " + this->GetProcessName();

		// Close the process handle to clean up resources
		CloseHandle(this->process);

		// Reset process handle to nullptr
		this->process = nullptr;

		// Reset process ID to 0
		this->processID = 0;
	}

	// Get information about the main module of the process
	this->moduleInfo = Memory::GetModuleInfo(this->process, (HMODULE)this->moduleBaseAddress);

	// Set attach status to true if everything is successful
	this->attachStatus = true;
}

bool Memory::isAttached() {
	// Return the value of attachStatus, which indicates whether the process is attached
	return this->attachStatus;
}

std::string Memory::GetErrorMessage() {
	// Return the error message stored in the Memory instance.
	return this->errorMessage;
}

std::string Memory::GetProcessName() {
	// Convert the process name from std::wstring to std::string and return it
	return std::string(this->processName.begin(), this->processName.end());
}

const wchar_t* Memory::GetProcessNameW() {
	// Return the pointer to the wide string process name
	return this->processNameW;
}

DWORD Memory::GetProcessID() {
	// Return the stored process ID for this instance
	return this->processID;
}

HANDLE Memory::GetProcess() {
	// Return the stored process handle for this instance
	return this->process;
}

uintptr_t Memory::GetModuleBaseAddress() {
	// Return the stored module base address for this instance
	return this->moduleBaseAddress;
}

MODULEINFO Memory::GetModuleInfo() {
    // Return the cached moduleInfo for the attached process.
    return this->moduleInfo;
}

uintptr_t Memory::GetAddress(uintptr_t address, std::vector<unsigned int> offsets) {
	// Resolve the final address by following the pointer chain starting from (moduleBaseAddress + address)
	return Memory::GetAddress(this->process, this->moduleBaseAddress + address, offsets);
}

std::string Memory::ReadString(uintptr_t address, SIZE_T maxLength) {
	// Call the static ReadString function, passing the process handle, address, and maxLength
	return Memory::ReadString(this->process, address, maxLength);
}

bool Memory::WriteString(uintptr_t address, const std::string value) {
	// Delegate the write operation to the static WriteString function,
	// using the process handle stored in this instance.
	return Memory::WriteString(this->process, address, value);
}
