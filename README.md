# ADPatcher

ADPatcher is a tool designed to detect and patch anti-disassemble techniques used by malicious software to obfuscate code and hinder analysis. By automatically replacing junk bytes with NOP (No Operation) instructions, it simplifies the reverse engineering process, eliminates the need for manual NOP insertion, and makes the application's flow easier to understand.


## Patching Process Example 
![alt text](<ADPatcher Tool.jpg>)

## Features

- **Detect Anti-Disassemble Techniques:** Identifies junk bytes and other anti-disassemble techniques used by malicious software to complicate code analysis.
- **Automatic Patching:** Replaces detected junk bytes with NOP instructions, automating the patching process.
- **32-bit Executable Support:** Specifically designed to work with 32-bit executables.

## Usage
- ```.\ADPatcher.exe [PATH]```

## How It Works
ADPatcher employs a multi-step process:

1. Input Validation: Verifies if a file path is provided. If not, usage instructions are displayed.
2. 32-bit Check: Ensures the tool only processes compatible 32-bit executables.
3. File Handling: Opens the specified file in binary mode and reads its contents.
4. Opcode Search: Scans for specific opcodes indicative of anti-disassembly techniques and stores their offsets.
5. Patching: Applies NOP instructions at the identified offsets, then writes the modified data back to the file.

## Contact
For questions, feedback, or contributions, reach out:

LinkedIn: [Baris Tural](https://www.linkedin.com/in/baristural/)