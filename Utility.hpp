#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <queue>
#include <filesystem>

using namespace std;

bool verify_checksum(string line)
{
    int sum = 0;
    for (int i = 1; i < line.length(); i += 2)
        sum += stoi((line.substr(i, 2)), nullptr, 16);
    return (sum & 0xFF) == 0;
}

string recalc_checksum(string line)
{
    int sum = 0;
    ostringstream ss;
    for (int i = 1; i < line.length() - 2; i += 2)
        sum += stoi((line.substr(i, 2)), nullptr, 16);
    ss << hex << setfill('0') << setw(2) << ((~sum + 1) & 0xFF);
    return ss.str();
}

bool check_file(ifstream &file, const string &file_name)
{
    file.close();
    file.clear();
    file.open(file_name + ".hex");
    if (!file)
    {
        cout << "Error: file not found\n";
        return false;
    }
    return true;
}

bool check_serial(const string &serial_num)
{
    if (serial_num.length() != 4 || !all_of(serial_num.begin(), serial_num.end(), ::isxdigit))
    {
        cout << "Error: invalid hex, enter 4 hex digits\n";
        return false;
    }
    return true;
}

void print_status(int ph, const string &file_name, const ostringstream &buffer)
{
    if (ph == 0)
    {
        cout << "Error: placeholder 0xAA55 not found at 0x2000\n";
        cout << "Aborted.\n"; exit(1);
    }
    else if (ph == 1)
    {
        // Path logic stays strictly within the success case
        filesystem::path input_path(file_name + ".hex");
        filesystem::path output_path = input_path.parent_path() / ("_Injected_" + input_path.filename().string());

        bool overwrite = filesystem::exists(output_path);

        ofstream injected_file(output_path);
        injected_file << buffer.str();
        
        cout << "Saved " << output_path.string() << (overwrite ? " (OVERWRITTEN)" : "") << "\n";
    }
    else if (ph > 1)
    {
        cout << "Error: duplicate records with address 0x2000\n";
        cout << "Aborted.\n"; exit(1);
    }
}