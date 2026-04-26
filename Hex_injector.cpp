// Intel HEX :LLAAAATT[DD...]CC
// : — Start code
// LL — Byte count
// AAAA — Address
// TT — Record type (00=data, 01=EOF, 02=extended segment address, 03=extended segment start address, 04=extended linear address, 05=extended linear start address)
// DD — Data bytes
// CC — Checksum

// dsPIC33 is little-endian
// dsPIC33 is 24 bits and hex is formatted with 32 bits so extra 00 at the end
// of each instruction in the data ex: 32 4f 5f 00 <-

// INHX32 format valid record types: 0, 1, 4, 5

// finding 0xAA55 at location 0x2000 which is 55 AA (little endian) at 0x4000

/*
    Make placeholder search case-insensitive (convert line to lowercase or use case-insensitive compare).

    Track Extended Linear Address (type 04) and compute absolute byte address (base + record_addr).

    Locate placeholder by calculating byte offset from record’s start address to 0x4000, not by find().

    Verify the full 4-byte sequence 55 AA 00 00 (case‑insensitive) at that offset before replacing.

    Report checksum error addresses as raw HEX byte address (remove the /2).

    Avoid double .hex extension if user enters filename with extension.

    Wrap stoi in try/catch to handle malformed lines gracefully.

    Add command‑line argument support for batch/production use.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <queue>

using namespace std;

bool verify_checksum(string line)
{
    int sum = 0;
    for (int i = 1; i < line.length(); i += 2)
        sum += stoi(line.substr(i, 2), nullptr, 16);
    return (sum & 0xFF) == 0;
}

string recalc_checksum(string line)
{
    int sum = 0;
    ostringstream ss;
    for (int i = 1; i < line.length() - 2; i += 2)
        sum += stoi(line.substr(i, 2), nullptr, 16);
    int checksum = (~sum + 1) & 0xFF;
    ss << hex << setfill('0') << setw(2) << checksum;
    return ss.str();
}

void manage_inputs(ifstream &file, string &file_name, string &serial_num, int argc, char *argv[])
{
    if (argc == 3)
    {
        file_name = argv[1];
        serial_num = argv[2];
    }
    while (true)
    {
        if (argc != 3)
        {
            cout << "Enter your firmware hexfile file_name (without .hex extension): \n";
            cin >> file_name;
        }
        file.open(file_name + ".hex");
        if (file)
            break;
        cout << "Error: file not found, try again\n";
    }
    while (true)
    {
        if (argc != 3)
        {
            cout << "Enter serial number hex in little endian (AA55 -> 55AA) : \n";
            cin >> serial_num;
        }
        if (serial_num.length() == 4 && all_of(serial_num.begin(), serial_num.end(), ::isxdigit))
            break;
        cout << "Error: invalid hex, enter 4 hex digits\n";
    }
    for (auto &c : serial_num)
        c = tolower(c);
}

void report(bool placeholder, bool checksum, string unedited, string edited, string file_name, ostringstream &buffer, queue<int> &error_addresses)
{
    if (placeholder && !checksum)
    {
        cout << "Unedited line:  " << unedited << "\n";
        cout << "Edited line:    " << edited << "\n";
        cout << "Serial number injected successfully!\n";
        ifstream check("Injected_" + file_name + ".hex");
        if (check)
            cout << "Warning: overwriting existing file: Injected_" << file_name << ".hex\n";
        check.close();
        ofstream file2("Injected_" + file_name + ".hex");
        file2 << buffer.str();
    }
    else
    {
        if (!placeholder)
            cout << "Injection failed: Placeholder missing from address 0x2000\n";
        if (checksum)
        {
            cout << "Injection failed: Checksum error at address: \n";
            while (!error_addresses.empty())
            {
                cout << "   0x" << hex << error_addresses.front() << "\n";
                error_addresses.pop();
            }
        }
    }
}


void process(ifstream &file, ostringstream &buffer, string &unedited, string &edited, string &serial_num, bool &placeholder, bool &checksum, queue<int> &error_addresses)
{
    string line, address, record, upper_address = "";
    while (getline(file, line))
    {
        address = line.substr(3, 4);
        record = line.substr(7, 2);
        if (!verify_checksum(line))
        {
            error_addresses.push(stoi(address, nullptr, 16) / 2);
            checksum = 1;
        }
        if (record == "00" && stoi(upper_address + address, nullptr, 16) == 0x4000 && (line.find("55aa0000") != string::npos || line.find("55AA0000") != string::npos))
        {
            unedited = line;
            line.replace(line.find("55aa0000") != string::npos ? line.find("55aa0000") : line.find("55AA0000"), 4, serial_num);
            line.replace(line.length() - 2, 2, recalc_checksum(line));
            edited = line;
            placeholder = 1;
        }
        else if (record == "04")
            upper_address = line.substr(9, 4);
        buffer << line << "\n";
    }
}


int main(int argc, char *argv[])
{
    ifstream file;
    ostringstream buffer;
    queue<int> error_addresses;
    bool placeholder = 0, checksum = 0;
    string file_name, serial_num, unedited, edited;
    
    // Handle user inputs
    manage_inputs(file, file_name, serial_num, argc, argv);

    // Process hex file
    process(file, buffer, unedited, edited, serial_num, placeholder, checksum, error_addresses);

    // Error checking
    report(placeholder, checksum, unedited, edited, file_name, buffer, error_addresses);
}