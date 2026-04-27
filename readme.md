================================================================================
                         HEX INJECTOR v1.2
        Serial Number Injection Utility for Microchip dsPIC33 Devices
================================================================================

REQUIREMENTS
------------
  - C++11 or later
  - Any standard C++ compiler (GCC, Clang, MSVC)


COMPILING
----------
  Windows (GCC/MinGW):
    g++ Hex_injector.cpp -o Hex_injector

  Linux/macOS:
    g++ Hex_injector.cpp -o Hex_injector


USAGE
------
  Interactive mode:
    ./Hex_injector

  Command-line mode (batch/production):
    ./Hex_injector <filename> <serial_number>

  Arguments:
    <filename>       Firmware hex file name WITHOUT the .hex extension.
                     File must exist in the same directory.
    <serial_number>  4-digit hex value in little-endian format.
                     Case insensitive. (e.g. AA55 entered as 55AA)


EXAMPLES
---------
  Interactive mode:
    > ./Hex_injector
    Enter your firmware hexfile name (without .hex extension):
    CrystalFontz_04.production
    Enter serial number hex in little endian (AA55 -> 55AA):
    ff22
    Unedited line:  :0440000055aa0000bd
    Edited line:    :04400000ff2200009b
    Serial number injected successfully!

  Command-line mode:
    > ./Hex_injector CrystalFontz_04.production ff22
    Unedited line:  :0440000055aa0000bd
    Edited line:    :04400000ff2200009b
    Serial number injected successfully!

  Compile and run (Windows PowerShell):
    g++ Hex_injector.cpp -o Hex_injector; ./Hex_injector CrystalFontz_04.production ff22

  Compile and run (Linux/macOS):
    g++ Hex_injector.cpp -o Hex_injector && ./Hex_injector CrystalFontz_04.production ff22


OUTPUT
-------
  The injected file is saved as:
    Injected_<filename>.hex

  Example:
    Input:   CrystalFontz_04.production.hex
    Output:  Injected_CrystalFontz_04.production.hex

  If the output file already exists, a warning is printed before overwriting:
    Warning: overwriting existing file: Injected_CrystalFontz_04.production.hex


ERROR CASES
------------
  - File not found:
      Error: file not found, try again

  - Invalid serial number (not 4 hex digits):
      Error: invalid hex, enter 4 hex digits

  - Placeholder missing from firmware:
      Injection failed: Placeholder missing from address 0x2000

  - Checksum error in hex file:
      Injection failed: Checksum error at address:
         0x1dc8
         0x1e28


NOTES
------
  - The firmware must contain the placeholder value 0xAA55 (55 AA 00 00 in
    little-endian INHX32 format) at word address 0x2000 (byte address 0x4000).
  - Supports Extended Linear Address records (type 04) for multi-segment firmware.
  - Original record ordering and addressing are preserved in the output file.
  - Checksum is automatically recalculated after injection.

================================================================================