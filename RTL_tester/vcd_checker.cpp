#include "sim_core_vcd_conv.hpp"
#include "rtl_core_vcd_conv.hpp"
#include "signal_comparator.hpp"
#include <iostream>
// 1. Define the signals you want to extract
std::set<std::string> sim_signals = {
    "Module.Clock",
    "Module.u_writeback.writeback2memory_pc_in",
    "Module.u_writeback.writeback2memory_rd_data_in",
    "Module.u_writeback.writeback2memory_rd_addr_in",
    "Module.u_memory.memory2exec_mem_wr_data_in"
};
std::set<std::string> rtl_signals = {
    "dut.clk",   // need to match clock signal for rising edge detection
    "u_writeback.pc_in",
    "u_writeback.rd_data_in",
    "u_writeback.rd_addr_in",
    "u_memory.mem_wr_data_in"
    // Fetch Stage (u_fetch)
};
bool csv_generated = true;
int main() {
    
    // 2. Initialize the Setup: (InputVCD, OutputCSV, SignalSet, GroupSize)
    if (csv_generated == false) {
        std::cout << "--- Generating CSV Files from VCDs ---" << std::endl;
        sim_core_vcd_conv mySimParser("dump_2.vcd", "simulation_core.csv", sim_signals, 1);
        rtl_core_vcd_conv myRtlParser("cpu_top_tb4.vcd", "rtl_core.csv", rtl_signals, 1);

        // 3. Run the parsers
        mySimParser.run();
        myRtlParser.run();
        csv_generated = true;
    }
    // 2. Define which signals should match
    // Map: { "Simulation_Signal_Name", "RTL_Signal_Name" }
    std::map<std::string, std::string> compareMap = {
        {"Module.u_writeback.writeback2memory_rd_data_in","cpu_top_tb.dut.u_cpu.u_writeback.rd_data_in"},
        {"Module.u_writeback.writeback2memory_pc_in","cpu_top_tb.dut.u_cpu.u_writeback.pc_in"},
        {"Module.u_writeback.writeback2memory_rd_addr_in","cpu_top_tb.dut.u_cpu.u_writeback.rd_addr_in"},
        {"Module.u_memory.memory2exec_mem_wr_data_in","cpu_top_tb.dut.u_cpu.u_memory.mem_wr_data_in"}
    };

    // 3. Run Comparison
    signal_comparator myComparator;
    std::cout << "\n--- Starting Signal Comparison ---" << std::endl;
    myComparator.compare("simulation_core.csv", "rtl_core.csv", compareMap);
    return 0;
}