================================================================================
                         HEX INJECTOR
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

  Verify:
    ./Hex_injector <filename>

  Verify and Inject:
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
    No issues found.
    [Line  1252] original: :0440000055aa0000bd
    [Line  1252] updated:  :04400000ff2200009b
    Saved Injected_CrystalFontz_04.production.hex

  Verify only:
    > ./Hex_injector CrystalFontz_04.production
    No issues found.

  Verify and Inject:
    > ./Hex_injector CrystalFontz_04.production ff22
    No issues found.
    [Line  1252] original: :0440000055aa0000bd
    [Line  1252] updated:  :04400000ff2200009b
    Saved Injected_CrystalFontz_04.production.hex

  Compile and run (Windows PowerShell):
    g++ Hex_injector.cpp -o Hex_injector; ./Hex_injector CrystalFontz_04.production ff22

  Compile and run (Linux/macOS):
    g++ Hex_injector.cpp -o Hex_injector && ./Hex_injector CrystalFontz_04.production ff22

OUTPUT
-------
  The injected file is saved in the same directory as:
    Injected_<filename>.hex

  Example:
    Input:   CrystalFontz_04.production.hex
    Output:  Injected_CrystalFontz_04.production.hex

  If the output file already exists it is overwritten and noted:
    Saved Injected_CrystalFontz_04.production.hex (overwritten)

ERROR CASES
------------
  - File not found:
      Error: file not found

  - Invalid serial number (not 4 hex digits):
      Error: invalid hex, enter 4 hex digits

  - Placeholder not found at 0x2000:
      Error: placeholder 0xAA55 not found at 0x2000

  - Duplicate records at 0x2000:
      Error: duplicate records with address 0x2000

  - Byte count mismatch or checksum error:
      [Line     1] Error: Byte count mismatch
      [Line     1] Error: Checksum error
      Aborted.

  - Missing EOF record (warning, injection continues):
      [Line     1] Warning: End of file record not found

TESTING
-------
Run: python3 Testbench.py

Validates edge cases, error handling, and all usage modes.

NOTES
------
  - The firmware must contain the placeholder value 0xAA55 (55 AA 00 00 in
    little-endian INHX32 format) at word address 0x2000 (byte address 0x4000).
  - Supports Extended Linear Address records (type 04) for multi-segment firmware.
  - Original record ordering and addressing are preserved in the output file.
  - Checksum is automatically recalculated after injection.
  - Files with checksum errors or byte count mismatches are rejected before injection.

================================================================================