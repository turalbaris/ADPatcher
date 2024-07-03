#include <iostream>
#include <fstream>
#include <windows.h>
#include <vector>
#include <map>

// File stream for reading and writing
std::fstream file;

// File path to the global file
std::string globalFilePath;

// Offsets of NOP instructions
std::vector<size_t> nopOffsets;

// Map of opcodes to search for
std::map<uint8_t, std::string> opCodes = {
    {0x7C, "jl--------"},
    {0x7F, "jg--------"},
    {0x74, "je/jz-----"},
    {0x75, "jne/jnz---"},
    {0x73, "jnb/jae---"},
    {0x7E, "jle-------"},
    {0x7D, "jge-------"},
    {0x72, "jb--------"},
    {0x77, "ja--------"},
    {0x76, "jbe-------"},
};

// Function to apply NOP instructions at the specified offset
void applyNop(std::fstream& file, const std::vector<size_t>& nopOffsets) {

    for (size_t offset : nopOffsets) {

        file.seekp(offset); // Go to the specified offset
        char nop = 0x90; // NOP opcode

        // Write the NOP opcode at the specified offset and the next byte
        for (int i = 0; i < 2; i++) {
            file.write(&nop, 1);
        }

        if (file.fail()) {
            std::cerr << "[!] Failed to write NOP at offset 0x" << std::hex << offset << std::endl;
        }
        else {
            std::cout << "[*] NOP applied successfully at offset 0x" << std::hex << offset << std::endl;
        }
    }
}

// Function to add an offset to the global vector for NOP'ing later
void scheduleNopAtOffset(size_t offset) {
    nopOffsets.push_back(offset);
}

// Reads a relative offset following an opcode
int32_t readRelativeOffset(const std::vector<uint8_t>& fileBuffer, size_t index) {
    if (index + 4 > fileBuffer.size()) {
        std::cerr << "Next bytes cannot be read, index is out of bounds." << std::endl;
        return 0;
    }
    int32_t offset;
    memcpy(&offset, &fileBuffer[index], sizeof(offset));
    return offset;
}

// Checks if the jump target is within the program's address space
void checkJumpTarget(const std::vector<uint8_t>& fileBuffer, size_t index, uint8_t opcode, size_t startAddress, size_t endAddress) {

    int32_t offset = readRelativeOffset(fileBuffer, index + 1);

    //std::cout << "Offset: 0x" << std::hex << offset;

    size_t jumpTarget = index + 1 + static_cast<size_t>(offset);

    if (jumpTarget >= startAddress && jumpTarget <= endAddress) {
        // Do nothing
        //std::cout << ", the jump target is within the program's address space: 0x" << std::hex << jumpTarget;
    }
    else {
        //std::cout << ", anti-disassemble detected, the jump target is not within the program's address space: 0x" << std::hex << jumpTarget;

        // Schedule NOP patching at the offset
        scheduleNopAtOffset(index);
    }
    //std::cout << std::endl;
}

// Searches jump opcodes in the file
void searchOpCodes(const std::vector<uint8_t>& fileBuffer, const std::map<uint8_t, std::string>& opCodes, size_t startAddress, size_t endAddress) {
    size_t searchEnd = (std::min)(fileBuffer.size(), endAddress);

    for (size_t i = startAddress; i < searchEnd; ++i) {
        auto it = opCodes.find(fileBuffer[i]);
        if (it != opCodes.end()) {

            if (i + 1 < searchEnd) {
                uint8_t nextByte = fileBuffer[i + 1];
                uint8_t nextByte2 = (i + 2 < searchEnd) ? fileBuffer[i + 2] : 0;

                // If the next byte is 0x02 (the opcode we are looking for)
                if (nextByte == 0x02) {

                    if (nextByte2 == 0xE9 || nextByte2 == 0xE8) {
                        int32_t offset = readRelativeOffset(fileBuffer, i + 3);

                        checkJumpTarget(fileBuffer, i + 2, nextByte2, startAddress, endAddress);

                    }
                }
            }
        }
    }
}

// Checks if the executable is a 32-bit executable
bool is32BitExecutable(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file." << std::endl;
        return false;
    }

    IMAGE_DOS_HEADER dosHeader;
    file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "Not a valid executable." << std::endl;
        return false;
    }

    file.seekg(dosHeader.e_lfanew, std::ios::beg);
    DWORD peSignature;
    file.read(reinterpret_cast<char*>(&peSignature), sizeof(peSignature));
    if (peSignature != IMAGE_NT_SIGNATURE) {
        std::cerr << "Not a valid PE file." << std::endl;
        return false;
    }

    IMAGE_FILE_HEADER fileHeader;
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    return fileHeader.Machine == IMAGE_FILE_MACHINE_I386;
}

int main(int argc, char* argv[]) {

    const std::string ADBPatcher =
        "    ___    ____  ____        __       __                \n"
        "   /   |  / __ \\/ __ \\____ _/ /______/ /_  ___  _____ \n"
        "  / /| | / / / / /_/ / __ `/ __/ ___/ __ \\/ _ \\/ ___/ \n"
        " / ___ |/ /_/ / ____/ /_/ / /_/ /__/ / / /  __/ /       \n"
        "/_/  |_/_____/_/    \\__,_/\\__/\\___/_/ /_/\\___/_/    \n\n"
        " -------------------------------------------------------\n"
        " | Twitter: https://twitter.com/zayotem                |\n"
        " | Github : https://github.com/ZAYOTEM                 |\n"
        " | Author : https://linkedin.com/in/baristural         |\n"
        " -------------------------------------------------------\n\n";

    if (argc < 2) {
        std::cout << ADBPatcher;
        std::cout << "Usage: .\\ADPatcher.exe [TARGET_PATH]\n\n";
        std::cout << "Example: .\\ADPatcher.exe C:\\Users\\%USERNAME%\\Desktop\\target.exe\n";
        return 1;
    }

    globalFilePath = argv[1];

    std::cout << ADBPatcher;

    std::fstream file(globalFilePath, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
    if (!file) {
        std::cerr << "[!] File could not be opened: " << globalFilePath << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    if (!is32BitExecutable(filePath)) {
        std::cout << "[!] The file is not a 32-bit executable." << std::endl;
        return 1;
    }

    std::streampos fileSize = file.tellg();
    std::vector<uint8_t> fileBuffer(fileSize);

    file.seekg(0, std::ios::beg); // Go to the beginning of the file
    file.read(reinterpret_cast<char*>(fileBuffer.data()), fileSize); // Read the file into the buffer

    searchOpCodes(fileBuffer, opCodes, 0x0, fileBuffer.size());

    if (nopOffsets.empty()) {
        std::cout << "[!] No offsets found for NOP patching." << std::endl;
    }
    else {
        std::cout << "[-] Starting patching process..." << std::endl;
        std::cout << "[-] Applying NOP instructions to " << nopOffsets.size() << " addresses..." << std::endl;

        // Apply scheduled NOP instructions
        applyNop(file, nopOffsets);

        std::cout << "[*] Output --> " << argv[1] << std::endl;
    }

    return 0;
}
