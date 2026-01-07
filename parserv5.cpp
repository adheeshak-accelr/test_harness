#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

string file_path = "/home/adheesha/riscv-toolchain/"; // Adjust this path as needed
string output_filepath = "/home/adheesha/Documents/test_harness/";

struct instruction_trace {
    int core=0, thread=0;
    uint32_t proramming_cnt=0, instruction_hex=0;
    int      reg_addr=0;
    uint32_t reg_data=0; // use uint_32 becuase some reg use full range due to storing mem addr
    uint32_t mem_addr=0;
    uint32_t mem_data=0;
    string   mnemonic;
};

vector<instruction_trace> instr_list;

string commit_line, trace_line;
int pt = 0;
int line_count = 0;

uint32_t hex_to_int(const string &s) {
    if (s.empty()) return 0;
    return static_cast<uint32_t>(stoul(s.substr(2, 8), nullptr, 16));
}

int str_to_int(const string &s) {
    if (s.empty()) return 0;
    return stoi(s);
}

int xreg_to_int(const string &reg_addr) {
    if (reg_addr.empty() || reg_addr[0] != 'x') return 0;
    return static_cast<int>(stoul(reg_addr.substr(1)));
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

    string commit_path = file_path + "commit_trace.log";
    string trace_path = file_path + "instruction_trace.log";
    string output_path = output_filepath + "spike_outv2.csv";
    
    ifstream commit_file(commit_path);
    ifstream trace_file(trace_path);
    ofstream outfile(output_path);
    if (!commit_file.is_open() || !trace_file.is_open() || !outfile.is_open()) {
        cerr << "Error opening file!\n";
        return 1;
    }

    //outfile << "core,thread,PC,instruction_hex,reg,reg_data,mem_addr,mem_data,mnemonic\n";

    // --------------------- SINGLE WHILE LOOP ----------------------
    while (getline(commit_file, commit_line) && getline(trace_file, trace_line)) {
        
        if(line_count< 6) {    // skip first 6 lines that caused to bootloader instructions in the spike simulation
            line_count++;
            continue; 
        }
        if (!commit_line.empty() ) { // skip first line
            vector<string> commit = split(commit_line);
            instruction_trace instr;
            instr.core = str_to_int(commit[1].substr(0, commit[1].size() - 1));
            instr.thread = str_to_int(commit[2]);
            instr.proramming_cnt = hex_to_int(commit[3]);
            instr.instruction_hex = hex_to_int(remove_paren(commit[4]));

            for (size_t i = 5; i < commit.size(); ++i) {
                if (commit[i][0] == 'x') {
                    instr.reg_addr = xreg_to_int(commit[i]);
                    if (i + 1 < commit.size()) instr.reg_data = hex_to_int(commit[i + 1]);
                }
                if (commit[i] == "mem") {
                    if (i + 1 < commit.size()) instr.mem_addr = hex_to_int(commit[i + 1]);
                    if (i + 2 < commit.size()) instr.mem_data = hex_to_int(commit[i + 2]);
                }
            }
            instr_list.push_back(instr);
        }

        // --------- process trace line ---------&& line_count >= 6
        if (!trace_line.empty() ) { // skip first line
            vector<string> commit = split(trace_line);
            instruction_trace instr;
            instr.proramming_cnt = hex_to_int(commit[2]);
            instr.instruction_hex = hex_to_int(remove_paren(commit[3]));
            for (size_t i = 4; i < commit.size(); i++) {
                instr.mnemonic += commit[i];
                if (i != commit.size() - 1) instr.mnemonic += " ";
            }

            // Merge with commit instr if proramming_cnt matches pt < instr_list.size() && 
            if (instr_list[pt].proramming_cnt == instr.proramming_cnt) {
                instr_list[pt].mnemonic = instr.mnemonic;
            } else {
                instr_list.push_back(instr);
            }
            pt++;
        }
        
    }

    // --------------------- WRITE CSV ----------------------
    // if (instr_list.size() > 5) // remove first 5 instructions which cause for the cpu bootloader 
    //     instr_list.erase(instr_list.begin(), instr_list.begin() + 5);

    for (const instruction_trace &i : instr_list) {
        outfile << i.core << ","
                << i.thread << ","
                << i.proramming_cnt << ","
                << i.instruction_hex << ","
                << i.reg_addr << ","
                << i.reg_data << ","
                << i.mem_addr << ","
                << i.mem_data << ","
                << "\"" << i.mnemonic << "\"" << "\n";
    }

    cout << "Merged CSV created: spike_outv2.csv\n";

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "Time taken: " << duration.count() << " us" << endl;

    return 0;
}
