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
// finding 0xAA55 at location 0x2000 which is 55 AA (little endian) at 0x4000 in byte addressing

#include "Utility.hpp"

void inputs(ifstream &file, string &file_name, string &serial_num)
{
    while (true)
    {
        cout << "Enter your firmware hexfile path (without .hex extension): \n";
        getline(cin, file_name);
        if (check_file(file, file_name))
            break;
    }
    while (true)
    {
        cout << "Enter serial number hex in little endian (AA55 -> 55AA) : \n";
        cin >> serial_num;
        if (check_serial(serial_num))
            break;
    }
}

bool Warnings(ifstream &file)
{
    file.clear();
    file.seekg(0);
    string line;
    int line_num = 0;
    bool eof_found = false;
    bool crash = false;
    while (getline(file, line))
    {
        line_num++;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        
        if (line.length() < 11)
        {
            cout << "[Line "<< setw(5) << line_num << "] Error: Line too short\n";
            crash = true;
        }
        else
        {
            int byte_count = stoi(line.substr(1, 2), nullptr, 16);
            if (line.length() != 11 + byte_count * 2)
            {
                cout << "[Line "<< setw(5) << line_num << "] Error: Byte count mismatch\n";
                crash = true;
            }
            if (!verify_checksum(line))
            {
                cout << "[Line "<< setw(5) << line_num << "] Error: Checksum error\n";
                crash = true;
            }
            if (line.substr(7, 2) == "01")
                eof_found = true;
        }
    }
    if (!eof_found)
        cout << "[Line "<< setw(5) << line_num << "] Warning: End of file record not found\n";
    if (!crash && eof_found)
        cout << "No issues found.\n";
    return crash;
}

void process(ifstream &file, const string &file_name, string &serial_num)
{
    string line, address, record, upper_address = "";
    int byte_count, real_address,line_num=0, ph=0;

    ostringstream buffer;
    file.clear();
    file.seekg(0);

    while (getline(file, line))
    {
        line_num++;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        address      = line.substr(3, 4);
        record       = line.substr(7, 2);
        byte_count   = stoi(line.substr(1, 2)      , nullptr, 16);
        real_address = stoi(upper_address + address, nullptr, 16);

        if (record == "00" && real_address <= 0x4000 && 0x4000 < real_address + byte_count)
        {
            int bytes_pos = 9 + ((0x4000 - real_address) * 2);
            string bytes  = line.substr(bytes_pos, 8);
            if (bytes == "55aa0000" || bytes == "55AA0000")
            {
                ph++;
                for (auto &c : serial_num) (bytes == "55AA0000") ? c = toupper(c) : c = tolower(c);
                cout << "[Line " << setw(5) << line_num << "] original: " << line << "\n";
                line.replace(bytes_pos, 4, serial_num);
                line.replace(line.length() - 2, 2, recalc_checksum(line));
                cout << "[Line " << setw(5) << line_num << "] updated:  " << line << "\n";
            }
        }
        else if (record == "04")
            upper_address = line.substr(9, 4);
        buffer << line << "\n";
        if (record == "01")
            break;
    }
    print_status(ph, file_name, buffer);
}

int main(int argc, char *argv[])
{
    string file_name, serial_num;
    ifstream file;
    if (argc > 3)
    {
        cout << "Injecting: ./Hex_injector <firmware_hexfile_name> <serial_number_hex>\n"
             << "Verifying: ./Hex_injector <firmware_hexfile_name>\n";
        exit(1);
    }
    else if (argc == 3)
    {
        file_name = argv[1];
        serial_num = argv[2];
        if(!check_file(file, file_name) || !check_serial(serial_num) || Warnings(file))
        {
            cout << "Aborted.\n"; exit(1);
        }
        process (file, file_name, serial_num);
    }
    else if (argc == 2)
    {
        file_name = argv[1];
        if (!check_file(file, file_name) || Warnings(file))
        { 
            cout<< "Aborted.\n"; exit(1);
        }
    }
    else
    {
        inputs  (file, file_name, serial_num);
        if (Warnings(file))
        {
            cout<< "Aborted.\n"; exit(1);
        }
        process (file, file_name, serial_num);
    }
}