import subprocess
import sys
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
HEX_DIR = os.path.join(SCRIPT_DIR, "Hex")
BINARY = "./Hex_injector.exe" if sys.platform == "win32" else "./Hex_injector"
GREEN, RED, YELLOW, RESET = "\033[92m", "\033[91m", "\033[93m", "\033[0m"
SERIAL = "aaee"

def check_serial_in_file(base_name, serial):
    path = os.path.join(HEX_DIR, f"_Injected_{base_name}.hex")
    try:
        with open(path) as f:
            for line in (l.strip() for l in f if len(l.strip()) >= 9 and l[7:9] == "00"):
                addr, byte_count = int(line[3:7], 16), int(line[1:3], 16)
                if addr <= 0x4000 < addr + byte_count:
                    offset = 9 + (0x4000 - addr) * 2
                    found = line[offset:offset+4].lower()
                    return (True, None) if found == serial.lower() else (False, f"Expected {serial} at 0x4000, found {found}")
        return False, "Address 0x4000 not found in injected file"
    except FileNotFoundError:
        return False, "Injected file does not exist"

tests = [
    # (name, args, expected_exit, expect_saved, expect_warnings, expect_errors, stdin)
    ("Valid placeholder",               ["valid_placeholder", SERIAL],      0, True,  0, 0, None),
    ("Valid extended address",          ["valid_extended_address", SERIAL], 0, True,  0, 0, None),
    ("Placeholder in large record",     ["placeholder_in_large_record", SERIAL], 0, True,  0, 0, None),
    ("CRLF endings",                    ["crlf_endings", SERIAL],           0, True,  0, 0, None),
    ("Missing placeholder",             ["missing_placeholder", SERIAL],    1, False, 0, 0, None),
    ("Bad bytecount",                   ["bad_bytecount", SERIAL],          1, False, 0, 2, None),
    ("Bad checksum",                    ["bad_checksum", SERIAL],           1, False, 0, 1, None),
    ("Multiple placeholders same addr", ["multiple_placeholders_same_addr", SERIAL], 1, False, 0, 0, None),
    ("Missing EOF",                     ["missing_eof", SERIAL],            0, True,  1, 0, None),
    ("Verify valid",                    ["valid_placeholder"],              0, False, 0, 0, None),
    ("Verify bad checksum",             ["bad_checksum"],                   1, False, 0, 1, None),
    ("Interactive valid",               [],                                 0, True,  0, 0, "Hex/valid_placeholder\naaee\n"),
    ("Interactive bad file",            [],                                 0, True,  0, 0, "Hex/nonexistent\nHex/valid_placeholder\naaee\n"),
]

# Ensure the C++17 flag is present for std::filesystem
subprocess.run(["g++", "-std=c++17", "Hex_injector.cpp", "-o", "Hex_injector"], cwd=SCRIPT_DIR, check=True)
print("Compiled.\n")

passed = failed = 0
for name, args, exp_exit, exp_saved, exp_warn, exp_err, stdin in tests:
    base_name = args[0] if args else "valid_placeholder"
    
    # Delete previous injected file in the Hex/ directory
    inj_path = os.path.join(HEX_DIR, f"_Injected_{base_name}.hex")
    if os.path.exists(inj_path): os.remove(inj_path)

    # Prepend Hex/ to the file argument for command line tests
    run_args = [BINARY]
    if args:
        run_args.append(f"Hex/{args[0]}")
        if len(args) > 1:
            run_args.append(args[1])

    res = subprocess.run(run_args, capture_output=True, text=True, input=stdin, cwd=SCRIPT_DIR)
    errors = []

    if res.returncode != exp_exit:
        errors.append(f"exit code {res.returncode}, expected {exp_exit}")

    if exp_saved:
        if "Saved" not in res.stdout:
            errors.append("Expected 'Saved' in output but not found")
        else:
            ok, err = check_serial_in_file(base_name, SERIAL)
            if not ok: errors.append(err)
    elif "Saved" in res.stdout:
        errors.append("File was saved but should not have been")

    act_warn = res.stdout.count("Warning:")
    if act_warn != exp_warn:
        errors.append(f"Expected {exp_warn} warning(s), got {act_warn}")

    act_err = sum(1 for l in res.stdout.splitlines() if "Error:" in l and "[Line" in l)
    if act_err != exp_err:
        errors.append(f"Expected {exp_err} [Line] error(s), got {act_err}")

    if errors:
        print(f"{RED}✗{RESET} {name}")
        for e in errors: print(f"    {YELLOW}→ {e}{RESET}")
        if res.stdout.strip():
            for line in res.stdout.strip().splitlines(): print(f"      {line}")
        failed += 1
    else:
        print(f"{GREEN}✓{RESET} {name}")
        passed += 1

print(f"\n{RED}{failed} failed{RESET}, {GREEN}{passed} passed{RESET}" if failed else f"\n{GREEN}All {passed} passed{RESET}")
sys.exit(1 if failed else 0)