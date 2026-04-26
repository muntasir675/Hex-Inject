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


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

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

void manage_inputs(ifstream &file, string &name, string &replacement)
{
    while(true)
    {
        cout << "Enter your firmware hexfile name: \n";
        cin >> name;
        file.open(name + ".hex");
        if(file)
            break;
        cout << "Error: file not found, try again\n";
    }
    while(true)
    {
        cout << "Enter serial number hex in little endian (AA55 -> 55AA) : \n";
        cin >> replacement;
        if(replacement.length() == 4 && all_of(replacement.begin(), replacement.end(), ::isxdigit))
            break;
        cout << "Error: invalid hex, enter 4 hex digits\n";
    }
    for(auto &c : replacement) c = tolower(c);
}


int main()
{
    ifstream file;
    ostringstream buffer;
    int error_address = 0;
    bool placeholder = 0, checksum = 0;
    string line, address, record, name, replacement, unedited, edited;

    manage_inputs(file, name, replacement);

    while (getline(file, line))
    {
        address = line.substr(3, 4);
        record = line.substr(7, 2);
        if (!verify_checksum(line))
        {
            error_address = stoi(address, nullptr, 16) / 2;
            checksum = 1;
        }
        if (record == "00" && stoi(address, nullptr, 16) == 0x4000 && line.find("55aa0000") != string::npos)
        {
            unedited = line;
            line.replace(line.find("55aa"), 4, replacement);
            line.replace(line.length() - 2, 2, recalc_checksum(line));
            edited = line;
            placeholder = 1;
        }
        buffer << line << "\n";
    }

    if (placeholder && !checksum)
    {
        cout << "Unedited line:  " << unedited << "\n";
        cout << "Edited line:    " << edited << "\n";
        cout << "Serial number injected successfully!\n";
        ofstream file2("Injected_" + name + ".hex");
        file2 << buffer.str();
    }
    else
    {
        if (!placeholder)
            cout << "Injection failed: Placeholder missing from address 0x2000\n";
        if (checksum)
            cout << "Injection failed: Checksum error at address 0x" << std::hex << error_address << "\n";
    }
}