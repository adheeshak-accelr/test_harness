#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct Instr {
    int core=0, thread=0;
    uint32_t PC=0, instruction_hex=0;
    string mnemonic;
    int reg=0;
    uint32_t reg_val=0; // use uint_32 becuase some reg use full range due to storing mem addr
    uint32_t mem_addr=0;
    uint32_t mem_val=0;
};

uint32_t hex_to_int(const string &s) {
    if (s.empty()) return 0;
    return static_cast<uint32_t>(stoul(s.substr(2, 8), nullptr, 16));
}

int str_to_int(const string &s) {
    if (s.empty()) return 0;
    return stoi(s);
}

int xreg_to_int(const string &reg) {
    if (reg.empty() || reg[0] != 'x') return 0;
    return static_cast<int>(stoul(reg.substr(1)));
}

vector<string> split(const string &s, char delim = ' ') {
    vector<string> tokens;
    string token;
    stringstream ss(s);
    while (getline(ss, token, delim)) {
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
}

string remove_paren(const string &s) {
    if (s.size() >= 2 && s.front() == '(' && s.back() == ')')
        return s.substr(1, s.size() - 2);
    return s;
}

int main() {
    auto start = high_resolution_clock::now();

    ifstream commit_file("commit_trace.log");
    ifstream trace_file("instruction_trace.log");
    ofstream outfile("spike_outv2.csv");

    if (!commit_file.is_open() || !trace_file.is_open() || !outfile.is_open()) {
        cerr << "Error opening file!\n";
        return 1;
    }

    outfile << "core,thread,PC,instruction_hex,mnemonic,reg,reg_val,mem_addr,mem_val\n";

    vector<Instr> instr_list;

    string commit_line, trace_line;
    int pt = 0;

    // --------------------- SINGLE WHILE LOOP ----------------------
    while (getline(commit_file, commit_line) || getline(trace_file, trace_line)) {
        // --------- process commit line ---------
        if (!commit_line.empty()) {
            vector<string> t = split(commit_line);
            Instr instr;
            instr.core = str_to_int(t[1].substr(0, t[1].size() - 1));
            instr.thread = str_to_int(t[2]);
            instr.PC = hex_to_int(t[3]);
            instr.instruction_hex = hex_to_int(remove_paren(t[4]));

            for (size_t i = 5; i < t.size(); ++i) {
                if (t[i][0] == 'x') {
                    instr.reg = xreg_to_int(t[i]);
                    if (i + 1 < t.size()) instr.reg_val = hex_to_int(t[i + 1]);
                }
                if (t[i] == "mem") {
                    if (i + 1 < t.size()) instr.mem_addr = hex_to_int(t[i + 1]);
                    if (i + 2 < t.size()) instr.mem_val = hex_to_int(t[i + 2]);
                }
            }
            instr_list.push_back(instr);
        }

        // --------- process trace line ---------
        if (!trace_line.empty()) {
            vector<string> t = split(trace_line);
            Instr instr;
            instr.PC = hex_to_int(t[2]);
            instr.instruction_hex = hex_to_int(remove_paren(t[3]));
            for (size_t i = 4; i < t.size(); i++) {
                instr.mnemonic += t[i];
                if (i != t.size() - 1) instr.mnemonic += " ";
            }

            // Merge with commit instr if PC matches
            if (pt < instr_list.size() && instr_list[pt].PC == instr.PC) {
                instr_list[pt].mnemonic = instr.mnemonic;
            } else {
                instr_list.push_back(instr);
            }
            pt++;
        }
    }

    // --------------------- WRITE CSV ----------------------
    if (instr_list.size() > 5) // remove first 5 instructions which run begining in the cpu 
        instr_list.erase(instr_list.begin(), instr_list.begin() + 5);

    for (const Instr &i : instr_list) {
        outfile << i.core << ","
                << i.thread << ","
                << i.PC << ","
                << i.instruction_hex << ","
                << "\"" << i.mnemonic << "\"" << ","
                << i.reg << ","
                << i.reg_val << ","
                << i.mem_addr << ","
                << i.mem_val << "\n";
    }

    cout << "Merged CSV created: spike_out.csv\n";

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "Time taken: " << duration.count() << " us" << endl;

    return 0;
}
