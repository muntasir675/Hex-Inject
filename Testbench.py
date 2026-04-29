import subprocess
import sys
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BINARY     = "./Hex_injector.exe" if sys.platform == "win32" else "./Hex_injector"
CYAN       = "\033[96m"
YELLOW     = "\033[93m"
RESET      = "\033[0m"

tests = [
    "CrystalFontz_04.production",
    "valid_placeholder",
    "valid_late_placeholder",
    "valid_extended_address",
    "placeholder_in_large_record",
    "missing_placeholder",
    "placeholder_wrong_value",
    "wrong_address",
    "bad_bytecount",
    "bad_checksum",
    "multiple_placeholders",
    "multiple_placeholders_same_addr",
    "missing_eof",
    "empty_file",
    "only_eof",
    "crlf_endings",
]

def header(title):
    print(f"\n{CYAN}=== {title} ==={RESET}")

def label(cmd):
    print(f"{YELLOW}> {cmd}{RESET}")

def run(args, stdin=None):
    subprocess.run(
        args,
        input=stdin,
        text=True,
        capture_output=False,
        cwd=SCRIPT_DIR,
    )
    print()

def run_interactive(name):
    run([BINARY], stdin=f"{name}\naaee\n")

# ── Compile ───────────────────────────────────────────────────────────────────

header("COMPILING")
result = subprocess.run(
    ["g++", "Hex_injector.cpp", "-o", "Hex_injector"],
    cwd=SCRIPT_DIR,
    capture_output=False,
    text=True,
)
if result.returncode != 0:
    print("Compilation failed, aborting.")
    sys.exit(1)
print("Compiled successfully.\n")

# ── Scenario tests ────────────────────────────────────────────────────────────

header("File not found")
label(f"{BINARY} nonexistent aaee")
run([BINARY, "nonexistent", "aaee"])

header("Invalid serial")
label(f"{BINARY} CrystalFontz_04.production afsd")
run([BINARY, "CrystalFontz_04.production", "afsd"])

header("Too many args")
label(f"{BINARY} CrystalFontz_04.production aaee extra")
run([BINARY, "CrystalFontz_04.production", "aaee", "extra"])

header("Interactive")
label(f"{BINARY} (interactive: CrystalFontz_04.production / aaee)")
run_interactive("CrystalFontz_04.production")

# ── Verify ────────────────────────────────────────────────────────────────────

header("VERIFY")
for t in tests:
    label(f"{BINARY} {t}")
    run([BINARY, t])

# ── Inject ────────────────────────────────────────────────────────────────────

header("INJECT")
for t in tests:
    label(f"{BINARY} {t} aaee")
    run([BINARY, t, "aaee"])

# ── Interactive ───────────────────────────────────────────────────────────────

header("INTERACTIVE")
for t in tests:
    label(f"{BINARY} (interactive: {t} / aaee)")
    run_interactive(t)