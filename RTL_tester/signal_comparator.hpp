#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>

class signal_comparator {
public:
    struct Stats {
        long checks = 0;
        long mismatches = 0;
    };

    vector<string> split(const string& s, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(s);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    void compare(string file1, string file2, map<string, string> signalMapping) {
        ifstream f1(file1), f2(file2);
        string line1, line2;

        if (!f1.is_open() || !f2.is_open()) {
            cerr << "Error opening CSV files." << endl;
            return;
        }

        getline(f1, line1);
        getline(f2, line2);
        vector<string> header1 = split(line1, ',');
        vector<string> header2 = split(line2, ',');

        map<string, int> idx1, idx2;
        for (int i = 0; i < (int)header1.size(); ++i) idx1[header1[i]] = i;
        for (int i = 0; i < (int)header2.size(); ++i) idx2[header2[i]] = i;

        // Map to track stats per signal pair
        map<string, Stats> reportCard;
        int cycle = 0;

        while (getline(f1, line1) && getline(f2, line2)) {
            vector<string> data1 = split(line1, ',');
            vector<string> data2 = split(line2, ',');

            for (auto const& [sig1, sig2] : signalMapping) {
                if (idx1.count(sig1) && idx2.count(sig2)) {
                    string key = sig1 + " vs " + sig2;
                    reportCard[key].checks++;

                    string val1 = data1[idx1[sig1]];
                    string val2 = data2[idx2[sig2]];

                    if (val1 != val2) {
                        std::cout << "[Mismatch] Cyc " << cycle << ": " << key 
                             << " (" << val1 << " != " << val2 << ")" << endl;
                        reportCard[key].mismatches++;
                    }
                }
            }
            cycle++;
        }

        printDetailedReport(cycle, reportCard);
    }

private:
    void printDetailedReport(int totalCycles, map<string, Stats>& reportCard) {
        long grandTotalChecks = 0;
        long grandTotalMismatches = 0;
        int spaces = 100;
        std::cout << "\n" << string(spaces, '=') << endl;
        std::cout << "                DETAILED SIGNAL COMPARISON REPORT" << endl;
        std::cout << string(spaces, '=') << endl;
        printf("%-100s | %-8s | %-8s\n", "Signal Comparison Pair", "Match", "Mismatch");
        std::cout << string(spaces, '-') << endl;
        for (auto const& [pairName, stat] : reportCard) {
            long matches = stat.checks - stat.mismatches;
            printf("%-100s | %-8ld | %-8ld\n", pairName.c_str(), matches, stat.mismatches);
            
            grandTotalChecks += stat.checks;
            grandTotalMismatches += stat.mismatches;
        }

        long grandTotalMatches = grandTotalChecks - grandTotalMismatches;
        double passRate = (grandTotalChecks > 0) ? ((double)grandTotalMatches / grandTotalChecks) * 100.0 : 0.0;

        std::cout << string(spaces, '=') << endl;
        std::cout << " SUMMARY STATISTICS" << endl;
        std::cout << " Total Cycles Processed : " << totalCycles << endl;
        std::cout << " Overall Pass Rate      : " << passRate << "%" << endl;
        std::cout << " Final Status           : " << (grandTotalMismatches == 0 ? "PASSED" : "FAILED") << endl;
        std::cout << string(spaces, '=') << endl;
    }
};