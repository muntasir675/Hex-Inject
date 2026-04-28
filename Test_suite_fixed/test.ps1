$tests = @(
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
    "crlf_endings"
)
Write-Host "=== VERIFY ==="
foreach ($t in $tests) { ./Hex_injector "$t" }
Write-Host "=== INJECT ==="
foreach ($t in $tests) { ./Hex_injector "$t" aaee }
Write-Host "=== INTERACTIVE ==="
foreach ($t in $tests) {
    cmd /c "(echo $t && echo aaee) | Hex_injector.exe"
}