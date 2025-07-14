#pragma once

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <vector>

class Memory {
private:
	// Stores the name of the target process (e.g., L"notepad.exe")
	std::wstring processName;

	// Pointer to the wide string representation of the process name
	const wchar_t* processNameW;

	// Process ID of the target process, initialized to 0
	DWORD processID = 0;

	// Handle to the target process, initialized to nullptr
	HANDLE process = nullptr;

	// Base address of the main module of the process, initialized to 0
	uintptr_t moduleBaseAddress = 0;

	// Stores information about the main module of the process
	MODULEINFO moduleInfo; 

	// Error message to store any issues encountered during operations
	std::string errorMessage;

	// Indicates whether the Memory object has successfully attached to a process
	bool attachStatus = false;

public:
	/**
	 * @brief Constructs a Memory object and attempts to attach to the specified process.
	 *
	 * This constructor initializes the Memory instance by calling attachProcess with the provided process name.
	 * The attachProcess method will attempt to find the process by name, open a handle to it, and retrieve its module base address.
	 * If any step fails, errorMessage and attachStatus will be set accordingly.
	 *
	 * @param processName The name of the process executable to attach to (e.g., L"notepad.exe").
	 */
	Memory(const std::wstring processName);

	/**
	 * @brief Destructor for the Memory class.
	 *
	 * Cleans up resources by closing the process handle if it is valid.
	 * This ensures that system resources are properly released when the Memory object is destroyed.
	 */
	~Memory();

	/**
	 * @brief Retrieves the process ID of a running process by its executable name.
	 *
	 * This method takes a snapshot of all processes in the system and iterates through them,
	 * comparing each process's executable name (case-insensitive) to the provided name.
	 * If a match is found, the corresponding process ID is returned.
	 * If no match is found, returns 0.
	 *
	 * @param processName The name of the process executable (e.g., L"notepad.exe").
	 * @return The process ID if found, otherwise 0.
	 */
	static DWORD GetProcessID(const wchar_t* processName);

	/**
	* @brief Retrieves the base address of a module within a specified process.
	*
	* This method takes a snapshot of all modules loaded in the process identified by processID,
	* then iterates through them to find a module whose name matches the provided moduleName
	* (case-insensitive). If found, it returns the base address of the module. If not found,
	* returns 0.
	*
	* @param processID The ID of the process to search for the module.
	* @param moduleName The name of the module to find (e.g., L"kernel32.dll").
	* @return The base address of the module if found, otherwise 0.
	*/
	static uintptr_t GetModuleBaseAddress(DWORD processID, const wchar_t* moduleName);

	/**
	 * @brief Retrieves information about a module in a remote process.
	 *
	 * This function fills a MODULEINFO structure with details about the specified module
	 * in the target process, such as its base address, size, and entry point.
	 * It uses the GetModuleInformation API to query the information.
	 *
	 * @param process Handle to the target process.
	 * @param hModule Handle to the module within the process.
	 * @return MODULEINFO structure containing information about the module.
	 */
	static MODULEINFO GetModuleInfo(HANDLE process, HMODULE hModule);

	/**
	* @brief Resolves a multi-level pointer address in a remote process.
	*
	* This method starts from a base address and follows a chain of pointers using the provided offsets.
	* For each offset, it reads the memory at the current address to get the next address, then adds the offset.
	* This is commonly used to find the final address of a value in games or applications that use pointer chains.
	*
	* @param process Handle to the target process with read access.
	* @param address The starting address (module base address + address).
	* @param offsets A vector of offsets to follow in the pointer chain.
	* @return The final resolved address, or 0 if any memory read fails.
	*/
	static uintptr_t GetAddress(HANDLE process, uintptr_t address, std::vector<unsigned int> offsets);

	/**
	 * @brief Reads a string from the memory of a remote process.
	 *
	 * This function allocates a buffer of size maxLength, initializes it to zero,
	 * and reads up to maxLength bytes from the specified address in the target process.
	 * It then constructs a std::string from the buffer, stopping at the first null terminator.
	 * The buffer is freed before returning the result.
	 *
	 * @param process Handle to the target process with read access.
	 * @param address The address in the remote process to read the string from.
	 * @param maxLength The maximum number of bytes to read (default: 100).
	 * @return The string read from the process memory (may be empty if read fails).
	 */
	static std::string ReadString(HANDLE process, uintptr_t address, SIZE_T maxLength = 100);

	/**
	 * @brief Writes a string to the memory of a remote process.
	 *
	 * This function writes the contents of a std::string (including the null terminator)
	 * to the specified address in the target process's memory. It uses WriteProcessMemory
	 * to perform the write operation. The function returns true if the write succeeds,
	 * or false if it fails.
	 *
	 * @param process Handle to the target process with write access.
	 * @param address The address in the remote process to write the string to.
	 * @param value The string value to write (will include the null terminator).
	 * @return True if the write succeeds, false otherwise.
	 */
	static bool WriteString(HANDLE process, uintptr_t address, const std::string value);

	/**
	 * @brief Attaches to a process by its name and initializes relevant members.
	 *
	 * This method attempts to attach to a process given its executable name.
	 * It performs the following steps:
	 * 1. Stores the process name.
	 * 2. Retrieves the process ID using the process name.
	 * 3. Opens a handle to the process.
	 * 4. Gets the base address of the main module.
	 * 5. Sets error messages and status flags if any step fails.
	 *
	 * @param processName The name of the process executable (e.g., L"notepad.exe").
	 */
	void attachProcess(const std::wstring processName);

	/**
	 * @brief Checks if the Memory instance is currently attached to a process.
	 *
	 * This method returns the status of the attach operation. If the Memory object
	 * has successfully attached to a process (i.e., the process handle and module base address
	 * are valid), this function returns true. Otherwise, it returns false.
	 *
	 * @return True if attached to a process, false otherwise.
	 */
	bool isAttached();

	/**
	 * @brief Retrieves the error message stored in the Memory instance.
	 *
	 * This method returns the error message string that was set during any failed
	 * operation (such as failing to attach to a process, open a handle, or find a module).
	 * The error message provides details about what went wrong, which can be useful for
	 * debugging or user feedback.
	 *
	 * @return The error message as a std::string. If no error occurred, this will be empty.
	 */
	std::string GetErrorMessage();

	/**
	 * @brief Retrieves the process name as a std::wstring.
	 *
	 * This method returns the process name stored in the Memory object.
	 * The process name is typically the executable name (e.g., L"notepad.exe").
	 *
	 * @return The process name as a std::wstring.
	 */
	std::string GetProcessName();

	/**
	 * @brief Retrieves the process name as a wide string pointer.
	 *
	 * This method returns a pointer to the wide string representation of the process name.
	 * Useful for APIs that require a const wchar_t* (e.g., Windows API functions).
	 *
	 * @return Pointer to the wide string process name.
	 */
	const wchar_t* GetProcessNameW();

	/**
	 * @brief Returns the process ID associated with this Memory instance.
	 *
	 * This method simply returns the processID member variable, which is set during construction.
	 * It does not perform any additional checks or lookups.
	 *
	 * @return The process ID of the process this Memory object is associated with.
	 */
	DWORD GetProcessID();

	/**
	 * @brief Returns the process handle associated with this Memory instance.
	 *
	 * This method simply returns the process member variable, which is set during construction.
	 * The returned handle can be used for further operations such as reading or writing memory.
	 *
	 * @return The process handle of the process this Memory object is associated with.
	 */
	HANDLE GetProcess();

	/**
	 * @brief Returns the base address of the main module for the process associated with this Memory instance.
	 *
	 * This method simply returns the moduleBaseAddress member variable, which is set during construction.
	 * The base address is useful for calculating offsets and performing memory operations relative to the module.
	 *
	 * @return The base address of the main module of the process this Memory object is associated with.
	 */
	uintptr_t GetModuleBaseAddress();

	/**
	 * @brief Retrieves information about the main module of the attached process.
	 *
	 * This method returns the MODULEINFO structure stored in the Memory instance.
	 * The MODULEINFO contains details such as the base address, size, and entry point
	 * of the main module of the process. This information is useful for memory operations
	 * that require knowledge of the module's layout.
	 *
	 * @return MODULEINFO structure containing information about the main module.
	 */
	MODULEINFO GetModuleInfo();

	/**
	 * @brief Resolves a multi-level pointer address in the target process using a base address and offsets.
	 *
	 * This method starts from a base address (relative to the module base address) and follows a chain of pointers
	 * using the provided offsets. For each offset, it reads the memory at the current address to get the next address,
	 * then adds the offset. This is commonly used to find the final address of a value in games or applications that use pointer chains.
	 *
	 * @param address The base address (relative to the module base) where the pointer chain starts.
	 * @param offsets A vector of offsets to follow in the pointer chain.
	 * @return The final resolved address, or 0 if any memory read fails.
	 */
	uintptr_t GetAddress(uintptr_t address, std::vector<unsigned int> offsets);

	/**
	 * @brief Reads a string from the memory of the target process at the specified address.
	 *
	 * This method calls the static ReadString function, passing the process handle
	 * associated with this Memory instance, the target address, and the maximum length
	 * of the string to read. It returns the string read from the process memory.
	 *
	 * @param address The address in the target process to read the string from.
	 * @param maxLength The maximum number of bytes to read (default: 100).
	 * @return The string read from the process memory (may be empty if read fails).
	 */
	std::string ReadString(uintptr_t address, SIZE_T maxLength = 100);

	/**
	 * @brief Writes a string to the memory of the target process at the specified address.
	 *
	 * This method calls the static WriteString function, passing the process handle
	 * associated with this Memory instance, the target address, and the string value
	 * to write. It returns true if the write operation succeeds, or false otherwise.
	 *
	 * @param address The address in the target process to write the string to.
	 * @param value The string value to write (will include the null terminator).
	 * @return True if the write succeeds, false otherwise.
	 */
	bool WriteString(uintptr_t address, const std::string value);

	/**
	 * @brief Reads a value of type T from the specified address in the target process's memory.
	 *
	 * This template function attempts to read memory from a remote process at the given address.
	 * If the read operation fails, it returns 0 (which may not be suitable for all types).
	 *
	 * @tparam T The type of value to read (e.g., int, float, struct).
	 * @param process Handle to the target process with read access.
	 * @param address The memory address to read from in the target process.
	 * @return The value read from memory, or 0 if the read fails.
	 */
	template <typename T>
	static T Read(HANDLE process, uintptr_t address) {
		T value; // Variable to store the read value

		// Attempt to read memory from the target process at the specified address
		ReadProcessMemory(process, (LPCVOID)address, &value, sizeof(T), NULL);

		// Return the value read from memory
		return value;
	}

	/**
	* @brief Reads a value of type T from the specified address in the target process's memory.
	*
	* This member function attempts to read memory from the process associated with this Memory instance
	* at the given address. It uses the Windows API ReadProcessMemory to perform the read operation.
	* If the read operation fails, the returned value will be uninitialized.
	*
	* @tparam T The type of value to read (e.g., int, float, struct).
	* @param address The memory address to read from in the target process.
	* @return The value read from memory. If the read fails, the value is uninitialized.
	*/
	template <typename T>
	T Read(uintptr_t address) {
		T value; // Variable to store the value read from memory

		// Attempt to read memory from the target process at the specified address.
		ReadProcessMemory(process, (LPCVOID)address, &value, sizeof(T), NULL);

		// Return the value read from memory.
		return value;
	}

	/**
	 * @brief Writes a value of type T to the specified address in the target process's memory.
	 *
	 * This template function attempts to write the provided value to a remote process at the given address.
	 * It returns true if the write operation succeeds, or false if it fails.
	 *
	 * @tparam T The type of value to write (e.g., int, float, struct).
	 * @param process Handle to the target process with write access.
	 * @param address The memory address to write to in the target process.
	 * @param value The value to write to the specified address.
	 * @return True if the write succeeds, false otherwise.
	 */
	template <typename T>
	static bool Write(HANDLE process, uintptr_t address, T value) {
		// Write the value to the target process's memory at the specified address
		return WriteProcessMemory(process, (LPVOID)address, &value, sizeof(T), NULL);
	}

	/**
	* @brief Writes a value of type T to the specified address in the target process's memory.
	*
	* This member function attempts to write the provided value to the process associated with this Memory instance
	* at the given address. It uses the Windows API WriteProcessMemory to perform the write operation.
	* The function returns true if the write operation succeeds, or false if it fails.
	*
	* @tparam T The type of value to write (e.g., int, float, struct).
	* @param address The memory address to write to in the target process.
	* @param value The value to write to the specified address.
	* @return True if the write succeeds, false otherwise.
	*/
	template <typename T>
	bool Write(uintptr_t address, T value) {
		// Write the value to the target process's memory at the specified address.
		return WriteProcessMemory(process, (LPVOID)address, &value, sizeof(T), NULL);
	}
};
