# HEX INJECTOR

Serial Number Injection Utility for Microchip dsPIC33 Devices

## Requirements
- C++17 or later (Required for `<filesystem>` support)
- Any standard C++ compiler (GCC, Clang, MSVC)

## Compiling
Windows (GCC/MinGW) and Linux/macOS:
```
g++ -std=c++17 Hex_injector.cpp -o Hex_injector
```

## Usage
Interactive mode:
```
./Hex_injector
```

Verify:
```
./Hex_injector <filepath>
```

Verify and Inject:
```
./Hex_injector <filepath> <serial_number>
```

Arguments:
- `<filename>`       Firmware hex file path without .hex extension.
                     Command-line mode: Paths with spaces require quotes "path with spaces\file"
                     Interactive mode: Enter paths directly without quotes
- `<serial_number>`  4-digit hex value in little-endian format.
                     Case is adapted automatically to file format. (e.g. AA55 entered as 55AA)

## Examples
Interactive mode:
```
> ./Hex_injector
Enter your firmware hexfile name (without .hex extension):
Hex/CrystalFontz_04.production
Enter serial number hex in little endian (AA55 -> 55AA):
ff22 
No issues found.
[Line  1252] original: :0440000055aa0000bd
[Line  1252] updated:  :04400000ff2200009b
Saved Hex\Injected_CrystalFontz_04.production.hex
```

Verify only:
```
> ./Hex_injector Hex\CrystalFontz_04.production
No issues found.
```

Verify and Inject:
```
> ./Hex_injector Hex\CrystalFontz_04.production ff22
No issues found.
[Line  1252] original: :0440000055aa0000bd
[Line  1252] updated:  :04400000ff2200009b
Saved Hex\Injected_CrystalFontz_04.production.hex
```

## Output
The injected file is saved in the same directory as:
```
path\Injected_<filename>.hex
```

Example:
- Input:   CrystalFontz_04.production.hex
- Output:  _Injected_CrystalFontz_04.production.hex

If the output file already exists it is overwritten and noted:
```
Saved _Injected_CrystalFontz_04.production.hex (OVERWRITTEN)
```

## Error Cases
- File not found:
  ```
  Error: file not found
  ```
- Invalid serial number (not 4 hex digits):
  ```
  Error: invalid hex, enter 4 hex digits
  ```
- Placeholder not found at 0x2000:
  ```
  Error: placeholder 0xAA55 not found at 0x2000
  ```
- Duplicate records at 0x2000:
  ```
  Error: duplicate records with address 0x2000
  ```
- Byte count mismatch or checksum error:
  ```
  [Line     1] Error: Byte count mismatch
  [Line     1] Error: Checksum error
  Aborted.
  ```
- Missing EOF record (warning, injection continues):
  ```
  [Line     1] Warning: End of file record not found
  ```
- Line too short:
  ```
  [Line     1] Error: Line too short
  ```

## Testing
A Python test script (Testbench.py) is included to validate the injector's functionality, error handling, and file parsing.

Usage:
```
> python Testbench.py
```

Successful validation:
```
✓ Valid placeholder
✓ Valid extended address
✓ Placeholder in large record
✓ CRLF endings
✓ Missing placeholder
✓ Bad bytecount
✓ Bad checksum
✓ Multiple placeholders same addr
✓ Missing EOF
✓ Verify valid
✓ Verify bad checksum
✓ Interactive valid
✓ Interactive bad file
All 13 passed
```

Failure detection (Example):
```
✗ Bad checksum
    → exit code 0, expected 1
    → File was saved but should not have been
      No issues found.
      [Line     1] original: :0440000055AA0000BD
      [Line     1] updated:  :04400000AAEE000024
      Saved Injected_bad_checksum.hex
```

## Notes
- The firmware must contain the placeholder value 0xAA55 (55 AA 00 00 in little-endian INHX32 format) at word address 0x2000 (byte address 0x4000).
- Supports Extended Linear Address records (type 04) for multi-segment firmware.
- Original record ordering and addressing are preserved in the output file.
- Checksum is automatically recalculated after injection.
- Files with checksum errors or byte count mismatches are rejected.
