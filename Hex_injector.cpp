// Intel HEX :LLAAAATT[DD...]CC
// : — Start code
// LL — Byte count
// AAAA — Address
// TT — Record type (00=data, 01=EOF, 04=extended linear address, 05=extended linear start address)
// DD — Data bytes
// CC — Checksum

// dsPIC33 is little-endian
// dsPIC33 uses 24 bit instructions and hex is formatted with 32 bits so extra 00 at the end
// of each instruction in the data ex: 32 4f 5f 00 <-

// INHX32 format valid record types: 00, 01, 04, 05
// finding 0xAA55 at location 0x2000 which is 55 AA (little endian) at 0x4000


/*
As the next step in the process, we’d like you to complete a small 
project called Hex Injector in the language you are most familiar with.
The challenge is designed to demonstrate your ability to write clear,
functional code; please also consider how this tool might be used in a real production setting.

Please see the attached project description for details on the task and deliverables. Once complete,
send back your solution along with any notes or supporting files you’d like to include.

Please take a week to complete this. Let us know if you need more time or have any questions.
We look forward to reviewing your work.
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
    ss << hex << setfill('0') << setw(2) << ((~sum + 1) & 0xFF);
    return ss.str();
}

void inputs(ifstream &file, string &file_name, string &serial_num, int argc, char *argv[])
{
    if (argc == 3)
    {
        file_name = argv[1];
        serial_num = argv[2];
    }
    while (true)
    {
        if (argc == 1)
        {
            cout << "Enter your firmware hexfile file_name (without .hex extension): \n";
            cin >> file_name;
        }
        file.open(file_name + ".hex");
        if (file)
            break;
        cout << "Error: file not found, try again\n";
        if (argc == 3)
            exit(1);
    }
    while (true)
    {
        if (argc ==1)
        {
            cout << "Enter serial number hex in little endian (AA55 -> 55AA) : \n";
            cin >> serial_num;
        }
        if (serial_num.length() == 4 && all_of(serial_num.begin(), serial_num.end(), ::isxdigit))
            break;
        cout << "Error: invalid hex, enter 4 hex digits\n";
        if (argc == 3)
            exit(1);
    }
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
            cout << "\nNote: overwriting existing file <Injected_" << file_name << ".hex>\n";
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
    int byte_count, Total_address;
    while (getline(file, line))
    {
        address = line.substr(3, 4);
        record = line.substr(7, 2);
        byte_count = stoi(line.substr(1, 2), nullptr, 16);
        Total_address = stoi(upper_address + address, nullptr, 16);

        if (!verify_checksum(line))
        {
            error_addresses.push(Total_address / 2);
            checksum = 1;
        }

        if (record == "00")
        {
            if (Total_address <= 0x4000 && 0x4000 < Total_address + byte_count)
            {
                int bytes_pos = 9 + ((0x4000 - Total_address) * 2);
                string bytes = line.substr(bytes_pos, 8);
                if (bytes == "55aa0000" || bytes == "55AA0000")
                {
                    for (auto &c : serial_num) (bytes == "55AA0000") ? c = toupper(c) : c = tolower(c);
                    unedited = line;
                    line.replace(bytes_pos, 4, serial_num);
                    line.replace(line.length() - 2, 2, recalc_checksum(line));
                    edited = line;
                    placeholder = 1;
                }
            }
        }
        else if (record == "04")
            upper_address = line.substr(9, 4);
        buffer << line << "\n";
        if(record == "01")
            break;
    }
}

void Warnings(string file_name)
{
    string line;
    int line_num = 0;
    bool eof_found = false;
    ifstream file(file_name + ".hex");
    while (getline(file, line))
    {
        line_num++;
        int byte_count = stoi(line.substr(1, 2), nullptr, 16);

        if (line.length() < 11)
        {
            cout << "Warning: line too short at line " << line_num << "\n"; continue;
        }
        if (eof_found)
            cout << "Warning: data after EOF record at line " << line_num << "\n";
        if (line.length() != 11 + byte_count * 2)
            cout << "Warning: byte count mismatch at line " << line_num << "\n";
        if (!verify_checksum(line))
            cout << "Warning: checksum error at line " << line_num << "\n";
        if (line.substr(7, 2) == "01")
            eof_found = true;
    }
    if (!eof_found)
        cout << "Warning: no EOF record found\n";
}

int main(int argc, char *argv[])
{
    ifstream file;
    ostringstream buffer;
    queue<int> error_addresses;
    bool placeholder = 0, checksum = 0;
    string file_name, serial_num, unedited, edited;

    if (argc > 3)
    {
        cout << "Injecting: ./Hex_injector <firmware_hexfile_name> <serial_number_hex>\n"
             << "Verifying: ./Hex_injector <firmware_hexfile_name>\n";
        exit(1);
    }    
    else if (argc == 2)
    {
        Warnings(argv[1]);
        exit(0);
    }

    // Handle user inputs
    inputs(file, file_name, serial_num, argc, argv);

    // Process hex file
    process(file, buffer, unedited, edited, serial_num, placeholder, checksum, error_addresses);

    // report results
    report(placeholder, checksum, unedited, edited, file_name, buffer, error_addresses);

    // Warnings hex file before processing
    Warnings(file_name);
}