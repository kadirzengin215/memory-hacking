#include <iostream>
#include "Memory.h"

int main() {
	// Create a Memory object to interact with the target process "ac_client.exe"
	Memory memory(L"ac_client.exe");

	// Check if the Memory object is successfully attached to the process
	if (!memory.isAttached()) {

		while (!memory.isAttached()) {
			// If not attached, print an error message and try to attach again
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

	// Resolve the player health address using address (with out base address) and offsets
	uintptr_t playerHealthAddress = memory.GetAddress(0x17E0A8, { 0xEC });

	// Change player health to 999 by writing the value to the resolved address
	memory.Write<int>(playerHealthAddress, 999);

	// Read the player health from memory and print it to the console
	std::cout << "Player health: " << memory.Read<int>(playerHealthAddress);

	return 0;
}
