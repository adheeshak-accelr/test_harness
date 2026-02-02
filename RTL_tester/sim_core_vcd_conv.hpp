#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>

using namespace std;

class sim_core_vcd_conv {
private:
    string inputVcd;
    string outputCsv;
    set<string> targetSignals;
    int cyclesPerRow;

    // Internal helper for string matching
    bool endsWith(const string& fullString, const string& ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        }
        return false;
    }

    // Binary string to Unsigned Integer string
    string binToUnsignedStr(string bin) {
        if (bin.find('x') != string::npos || bin.find('z') != string::npos) return "0";
        try {
            string cleanBin = (bin[0] == 'b' || bin[0] == 'B') ? bin.substr(1) : bin;
            unsigned long long val = stoull(cleanBin, nullptr, 2);
            return to_string(val);
        } catch (...) { return "0"; }
    }

    struct SignalInfo {
        string fullName;
        string lastValue = "0";
    };

public:
    sim_core_vcd_conv(string vcd, string csv, set<string> signals, int groupSize = 1) 
        : inputVcd(vcd), outputCsv(csv), targetSignals(signals), cyclesPerRow(groupSize) {}

    void run() {
        ifstream vcdFile(inputVcd);
        ofstream csvFile(outputCsv);

        if (!vcdFile.is_open()) {
            cerr << "Error: Could not open " << inputVcd << endl;
            return;
        }

        string line;
        map<string, SignalInfo> symbolMap;
        vector<string> activeSymbols;
        vector<string> scopeStack;
        vector<string> rowBuffer;
        
        string clkSymbol = "";
        int cycleCounter = 0;
        bool headerWritten = false;

        while (getline(vcdFile, line)) {
            if (line.empty()) continue;

            // 1. Hierarchy Tracking
            if (line.find("$scope") == 0) {
                stringstream ss(line);
                string tmp, type, name;
                ss >> tmp >> type >> name;
                scopeStack.push_back(name);
            } 
            else if (line.find("$upscope") == 0) {
                if (!scopeStack.empty()) scopeStack.pop_back();
            }
            // 2. Variable Mapping
            else if (line.find("$var") == 0) {
                stringstream ss(line);
                string tmp, type, size, sym, name;
                ss >> tmp >> type >> size >> sym >> name;

                string fullPath = "";
                for (size_t i = 0; i < scopeStack.size(); ++i) {
                    fullPath += scopeStack[i] + (i == scopeStack.size() - 1 ? "" : ".");
                }
                fullPath += "." + name;

                for (const string& target : targetSignals) {
                    if (endsWith(fullPath, target)) {
                        symbolMap[sym] = {fullPath, "0"};
                        activeSymbols.push_back(sym);
                        if (name == "Clock" || name == "clk" || name == "clk_i") clkSymbol = sym;
                        break;
                    }
                }
            }
            // 3. Signal Value Extraction
            else {
                string val, sym;
                if (line[0] == 'b' || line[0] == 'B') {
                    stringstream ss(line);
                    ss >> val >> sym;
                } else if (line[0] != '#' && line[0] != '$') {
                    val = line.substr(0, 1);
                    sym = line.substr(1);
                } else continue;

                if (symbolMap.count(sym)) {
                    string prevVal = symbolMap[sym].lastValue;
                    symbolMap[sym].lastValue = (line[0] == 'b' || line[0] == 'B') ? binToUnsignedStr(val) : val;

                    // 4. Rising Edge Logic
                    if (sym == clkSymbol && prevVal == "0" && symbolMap[sym].lastValue == "1") {
                        if (!headerWritten) {
                            for (int c = 0; c < cyclesPerRow; ++c) {
                                for (const auto& s : activeSymbols) {
                                    csvFile << symbolMap[s].fullName << (cyclesPerRow > 1 ? "_C" + to_string(c) : "") << (s == activeSymbols.back() && c == cyclesPerRow - 1 ? "" : ",");
                                }
                            }
                            csvFile << "\n";
                            headerWritten = true;
                        }

                        for (const auto& s : activeSymbols) rowBuffer.push_back(symbolMap[s].lastValue);
                        cycleCounter++;

                        if (cycleCounter % cyclesPerRow == 0) {
                            for (size_t i = 0; i < rowBuffer.size(); ++i) {
                                csvFile << rowBuffer[i] << (i == rowBuffer.size() - 1 ? "" : ",");
                            }
                            csvFile << "\n";
                            rowBuffer.clear();
                        }
                    }
                }
            }
        }
        cout << "Parsing complete. " << cycleCounter << " cycles processed into " << outputCsv << endl;
    }
};

