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

    std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    void compare(std::string file1, std::string file2, std::map<std::string, std::string> signalMapping) {
        std::ifstream f1(file1), f2(file2);
        std::string line1, line2;

        if (!f1.is_open() || !f2.is_open()) {
            std::cerr << "Error opening CSV files." << std::endl;
            return;
        }

        std::getline(f1, line1);
        std::getline(f2, line2);
        std::vector<std::string> header1 = split(line1, ',');
        std::vector<std::string> header2 = split(line2, ',');

        std::map<std::string, int> idx1, idx2;
        for (int i = 0; i < (int)header1.size(); ++i) idx1[header1[i]] = i;
        for (int i = 0; i < (int)header2.size(); ++i) idx2[header2[i]] = i;

        // Map to track stats per signal pair
        std::map<std::string, Stats> reportCard;
        int cycle = 0;

        while (std::getline(f1, line1) && std::getline(f2, line2)) {
            std::vector<std::string> data1 = split(line1, ',');
            std::vector<std::string> data2 = split(line2, ',');

            for (auto const& [sig1, sig2] : signalMapping) {
                if (idx1.count(sig1) && idx2.count(sig2)) {
                    std::string key = sig1 + " vs " + sig2;
                    reportCard[key].checks++;

                    std::string val1 = data1[idx1[sig1]];
                    std::string val2 = data2[idx2[sig2]];

                    if (val1 != val2) {
                        std::cout << "[Mismatch] Cyc " << cycle << ": " << key 
                             << " (" << val1 << " != " << val2 << ")" << std::endl;
                        reportCard[key].mismatches++;
                    }
                }
            }
            cycle++;
        }

        printDetailedReport(cycle, reportCard);
    }

private:
    void printDetailedReport(int totalCycles, std::map<std::string, Stats>& reportCard) {
        long grandTotalChecks = 0;
        long grandTotalMismatches = 0;
        int spaces = 100;
        std::cout << "\n" << std::string(spaces, '=') << std::endl;
        std::cout << "                DETAILED SIGNAL COMPARISON REPORT" << std::endl;
        std::cout << std::string(spaces, '=') << std::endl;
        printf("%-100s | %-8s | %-8s\n", "Signal Comparison Pair", "Match", "Mismatch");
        std::cout << std::string(spaces, '-') << std::endl;
        for (auto const& [pairName, stat] : reportCard) {
            long matches = stat.checks - stat.mismatches;
            printf("%-100s | %-8ld | %-8ld\n", pairName.c_str(), matches, stat.mismatches);
            
            grandTotalChecks += stat.checks;
            grandTotalMismatches += stat.mismatches;
        }

        long grandTotalMatches = grandTotalChecks - grandTotalMismatches;
        double passRate = (grandTotalChecks > 0) ? ((double)grandTotalMatches / grandTotalChecks) * 100.0 : 0.0;

        std::cout << std::string(spaces, '=') << std::endl;
        std::cout << " SUMMARY STATISTICS" << std::endl;
        std::cout << " Total Cycles Processed : " << totalCycles << std::endl;
        std::cout << " Overall Pass Rate      : " << passRate << "%" << std::endl;
        std::cout << " Final Status           : " << (grandTotalMismatches == 0 ? "PASSED" : "FAILED") << std::endl;
        std::cout << std::string(spaces, '=') << std::endl;
    }
};