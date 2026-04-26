# Hex Injector v1.2

A utility to inject unit-specific serial numbers into compiled firmware hex files for Microchip dsPIC33 devices.

## Requirements

- C++11 or later
- Any standard C++ compiler (GCC, Clang, MSVC)

## Inputs

1. **Firmware hex file name** — file name without the `.hex` extension, must exist in the same directory.
2. **Serial number** — 4 digit hex value in little endian format (e.g. `AA55` becomes `55AA`). Input is case insensitive and converted to lowercase internally.

## Example

```
Enter your firmware hexfile name:
CrystalFontz_04.production
Enter serial number hex in little endian (AA55 -> 55AA):
ffff
Unedited line:  :0440000055aa0000bd
Edited line:    :044000000000ffffbc
Serial number injected successfully!
```

Output file will be saved as `Injected_<filename>.hex` in the same directory.

## Error Cases

- File not found
- Invalid hex input
- Placeholder missing
- Checksum error