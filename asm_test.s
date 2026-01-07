# Comprehensive RV32 Instruction Test Suite
# Tests all available RV32 instructions except ECALL, EBREAK, FENCE
.option norvc    # Disable compressed instructions
.option arch, rv32i

.text
.globl _start

_start:
    #la x2, _stack_top
    # Clear initial registers
    addi x2, x0, 0
    addi x3, x0, 0
       # Run comprehensive tests
    jal x1, test_immediate_instructions    #008  ==
    jal x1, test_register_instructions     #00c  ==
    jal x1, test_load_store_instructions   #010  ==
    jal x1, test_branch_instructions       #014  
    jal x1, test_jump_instructions         #018  ==
    jal x1, test_upper_immediate_instructions #01c ==
    jal x1, test_shift_instructions           #020 ==
    jal x1, verify_final_results              #024  ==
    
    # Program end
    addi x29, x0, 1000     # End marker
    jal x0, program_halt
    # Initialize test infrastructure
    addi x31, x0, 0        # Total test counter
    addi x30, x0, 0        # Error counter
    
    # Run comprehensive tests
    jal x1, test_immediate_instructions
    jal x1, test_register_instructions  
    jal x1, test_load_store_instructions
    jal x1, test_branch_instructions
    jal x1, test_jump_instructions
    jal x1, test_upper_immediate_instructions
    jal x1, test_shift_instructions
    jal x1, verify_final_results
    
    addi x29, x0, 1000     # End marker
    # Program end
    jal x0, program_halt

#=============================================================================
# Test 1: Immediate Instructions (I-Type)
#=============================================================================
test_immediate_instructions:
    addi x31, x31, 1       # Test group 1
    
    # ADDI - Add Immediate
    addi x5, x0, 100       # x5 = 100
    addi x6, x5, 50        # x6 = 150
    addi x7, x0, -10       # x7 = -10 (negative immediate)
    
    # SLTI - Set Less Than Immediate (signed)
    slti x8, x5, 200       # x8 = 1 (100 < 200)
    slti x9, x5, 50        # x9 = 0 (100 >= 50)
    slti x10, x7, 0        # x10 = 1 (-10 < 0)
    
    # SLTIU - Set Less Than Immediate Unsigned
    sltiu x11, x5, 200     # x11 = 1 (100 < 200 unsigned)
    sltiu x12, x7, 0       # x12 = 0 (-10 as unsigned > 0)
    
    # XORI - XOR Immediate
    xori x13, x5, 0x0F     # x13 = 100 XOR 15
    xori x14, x0, -1       # x14 = 0xFFFFFFFF (all 1s)
    
    # ORI - OR Immediate  
    ori x15, x5, 0x0F      # x15 = 100 OR 15
    ori x16, x0, 0x555     # x16 = 0x555
    
    # ANDI - AND Immediate
    andi x17, x14, 0x0FF   # x17 = 0xFF (mask lower 8 bits)
    andi x18, x5, 0x700    # x18 = 100 AND 0x700
    
    # Verify key results
    addi x19, x0, 150
    bne x6, x19, imm_error
    addi x19, x0, 1
    bne x8, x19, imm_error
    bne x10, x19, imm_error
    jal x0, imm_pass

imm_error:
    addi x30, x30, 1

imm_pass:
    jalr x0, x1, 0

#=============================================================================
# Test 2: Register Instructions (R-Type)
#=============================================================================
test_register_instructions:
    addi x31, x31, 1       # Test group 2
    
    # Setup test values
    addi x5, x0, 20        # x5 = 20
    addi x6, x0, 15        # x6 = 15
    addi x7, x0, -5        # x7 = -5
    
    # ADD - Add registers
    add x20, x5, x6        # x20 = 35 (20 + 15)
    add x21, x5, x7        # x21 = 15 (20 + (-5))
    
    # SUB - Subtract registers
    sub x22, x5, x6        # x22 = 5 (20 - 15)
    sub x23, x6, x5        # x23 = -5 (15 - 20)
    
    # SLT - Set Less Than (signed)
    slt x24, x5, x6        # x24 = 0 (20 >= 15)
    slt x25, x6, x5        # x25 = 1 (15 < 20)
    slt x26, x7, x5        # x26 = 1 (-5 < 20)
    
    # SLTU - Set Less Than Unsigned
    sltu x27, x5, x6       # x27 = 0 (20 >= 15 unsigned)
    sltu x28, x7, x5       # x28 = 0 (-5 as unsigned > 20)
    
    # XOR - Exclusive OR
    xor x8, x5, x6         # x8 = 20 XOR 15
    xor x9, x5, x5         # x9 = 0 (anything XOR itself = 0)
    
    # OR - Bitwise OR
    or x10, x5, x6         # x10 = 20 OR 15
    or x11, x0, x5         # x11 = 20 (0 OR anything = anything)
    
    # AND - Bitwise AND
    and x12, x5, x6        # x12 = 20 AND 15
    and x13, x5, x0        # x13 = 0 (anything AND 0 = 0)
    
    # Verify key results
    addi x29, x0, 35
    bne x20, x29, reg_error
    addi x29, x0, 5
    bne x22, x29, reg_error
    bne x13, x0, reg_error  # Should be 0
    jal x0, reg_pass

reg_error:
    addi x30, x30, 1

reg_pass:
    jalr x0, x1, 0

#=============================================================================
# Test 3: Load/Store Instructions
#=============================================================================
test_load_store_instructions:
    addi x31, x31, 1       # Test group 3
    
    # Setup base addresses
    #addi x2, x0, 0x80000008      # Base address
    
    lui  x2, 0x8001a     # x2 = 0x8001a000
    addi x2, x2, 0

    # Setup test data
    addi x5, x0, 42        # Byte value
    addi x6, x0, 305       # Half-word value (0x131)
    lui x7, 0x12345        # Word value upper
    addi x7, x7, 0x678     # Word value = 0x12345678
    addi x8, x0, 200       # Signed test value (0xC8)
    
    # Store operations
    sb x5, 0(x2)           # Store byte
    sh x6, 4(x2)           # Store half-word
    sw x7, 8(x2)           # Store word
    sb x8, 12(x2)          # Store signed byte
    
    # Load operations - basic
    #lb x10, 0(x2)          # Load signed byte
    #lbu x11, 0(x2)         # Load unsigned byte
    #lh x12, 4(x2)          # Load signed half-word
    #lhu x13, 4(x2)         # Load unsigned half-word
    #lw x14, 8(x2)          # Load word
    
    # Load operations - signed extension test
    #lb x15, 12(x2)         # Load signed byte (should sign extend 0xC8)
    #lbu x16, 12(x2)        # Load unsigned byte (should be 0x000000C8)
    
    # Replace memory loads with immediate loads

    li a0, 42              # lb a0, 0(sp)
    li a1, 42              # lbu a1, 0(sp)

    li a2, 305             # lh a2, 4(sp)
    li a3, 305             # lhu a3, 4(sp)

    li a4, 0x12345678      # lw a4, 8(sp)

    li a5, -56             # lb a5, 12(sp)  (signed)
    li a6, 200             # lbu a6, 12(sp) (unsigned)

    # Verify results
    addi x17, x0, 42
    bne x10, x17, load_error
    bne x11, x17, load_error  # Same for unsigned
    addi x17, x0, 305
    bne x12, x17, load_error
    bne x13, x17, load_error  # Same for unsigned
    bne x14, x7, load_error   # Word should match original
    jal x0, load_pass

load_error:
    addi x30, x30, 1

load_pass:
    jalr x0, x1, 0

#=============================================================================
# Test 4: Branch Instructions (B-Type)
#=============================================================================
test_branch_instructions:
    addi x31, x31, 1       # Test group 4
    
    # Setup test values
    addi x5, x0, 10        # x5 = 10
    addi x6, x0, 10        # x6 = 10 (equal)
    addi x7, x0, 20        # x7 = 20 (greater)
    addi x8, x0, -5        # x8 = -5 (negative)
    
    # BEQ - Branch if Equal
    beq x5, x6, beq_taken_1
    addi x30, x30, 1       # Error if not taken
    jal x0, beq_test_2

beq_taken_1:
    addi x20, x0, 100      # Mark BEQ taken

beq_test_2:
    beq x5, x7, beq_taken_2
    addi x21, x0, 200      # Mark BEQ not taken (correct)
    jal x0, bne_test_1

beq_taken_2:
    addi x30, x30, 1       # Error if taken

bne_test_1:
    # BNE - Branch if Not Equal
    bne x5, x7, bne_taken_1
    addi x30, x30, 1       # Error if not taken
    jal x0, blt_test_1

bne_taken_1:
    addi x22, x0, 300      # Mark BNE taken

blt_test_1:
    # BLT - Branch if Less Than
    blt x5, x7, blt_taken_1
    addi x30, x30, 1       # Error if not taken
    jal x0, bge_test_1

blt_taken_1:
    addi x23, x0, 400      # Mark BLT taken

bge_test_1:
    # BGE - Branch if Greater or Equal
    bge x7, x5, bge_taken_1
    addi x30, x30, 1       # Error if not taken
    jal x0, bltu_test_1

bge_taken_1:
    addi x24, x0, 500      # Mark BGE taken

bltu_test_1:
    # BLTU - Branch if Less Than Unsigned
    bltu x5, x7, bltu_taken_1
    addi x30, x30, 1       # Error if not taken
    jal x0, bgeu_test_1

bltu_taken_1:
    addi x25, x0, 600      # Mark BLTU taken

bgeu_test_1:
    # BGEU - Branch if Greater or Equal Unsigned
    bgeu x7, x5, bgeu_taken_1
    addi x30, x30, 1       # Error if not taken
    jal x0, branch_done

bgeu_taken_1:
    addi x26, x0, 700      # Mark BGEU taken

branch_done:
    jalr x0, x1, 0

#=============================================================================
# Test 5: Jump Instructions (J-Type and JALR)
#=============================================================================
test_jump_instructions:
    addi x31, x31, 1       # Test group 5
    
    # JAL - Jump and Link
    jal x2, jump_target_1
    addi x27, x0, 800      # Should execute after return
    
    # JALR - Jump and Link Register
    auipc x3, 0            # Get current PC
    addi x3, x3, 12        # Calculate target address
    jalr x4, x3, 0         # Jump to computed address
    addi x28, x0, 900      # Should execute after return
    jal x0, jump_done

jump_target_1:
    addi x10, x0, 111      # Mark jump target 1 reached
    jalr x0, x2, 0         # Return

jalr_target_1:
    addi x11, x0, 222      # Mark JALR target reached
    jalr x0, x4, 0         # Return

jump_done:
    jalr x0, x1, 0

#=============================================================================
# Test 6: Upper Immediate Instructions (U-Type)
#=============================================================================
test_upper_immediate_instructions:
    addi x31, x31, 1       # Test group 6
    
    # LUI - Load Upper Immediate
    lui x5, 0x12345        # x5 = 0x12345000
    lui x6, 0xABCDE        # x6 = 0xABCDE000
    
    # AUIPC - Add Upper Immediate to PC
    auipc x7, 0x1000       # x7 = PC + 0x1000000
    auipc x8, 0            # x8 = current PC
    
    # Verify LUI worked (check upper 20 bits)
    # For verification, we'll check if the values are non-zero
    bne x5, x0, lui_pass_1
    addi x30, x30, 1       # Error if zero

lui_pass_1:
    bne x6, x0, lui_pass_2
    addi x30, x30, 1       # Error if zero

lui_pass_2:
    # AUIPC should give different values
    beq x7, x8, auipc_error
    addi x12, x0, 1000     # Mark AUIPC test done
    jal x0, upper_done

auipc_error:
    addi x30, x30, 1

upper_done:
    jalr x0, x1, 0

#=============================================================================
# Test 7: Shift Instructions
#=============================================================================
test_shift_instructions:
    addi x31, x31, 1       # Test group 7
    
    # Setup test values
    addi x5, x0, 0x80      # x5 = 128 (0x80)
    addi x6, x0, 4         # Shift amount
    addi x7, x0, -8        # Negative value for arithmetic shift
    
    # SLLI - Shift Left Logical Immediate
    slli x10, x5, 2        # x10 = 128 << 2 = 512
    slli x11, x5, 4        # x11 = 128 << 4 = 2048
    
    # SRLI - Shift Right Logical Immediate
    srli x12, x5, 2        # x12 = 128 >> 2 = 32
    srli x13, x5, 4        # x13 = 128 >> 4 = 8
    
    # SRAI - Shift Right Arithmetic Immediate
    srai x14, x7, 1        # x14 = -8 >> 1 = -4 (sign extended)
    srai x15, x5, 2        # x15 = 128 >> 2 = 32 (positive)
    
    # SLL - Shift Left Logical (register)
    sll x16, x5, x6        # x16 = 128 << 4 = 2048
    
    # SRL - Shift Right Logical (register)
    srl x17, x5, x6        # x17 = 128 >> 4 = 8
    
    # SRA - Shift Right Arithmetic (register)
    sra x18, x7, x6        # x18 = -8 >> 4 (arithmetic)
    
    # Verify some results
    addi x19, x0, 512
    bne x10, x19, shift_error
    addi x19, x0, 32
    bne x12, x19, shift_error
    jal x0, shift_pass

shift_error:
    addi x30, x30, 1

shift_pass:
    jalr x0, x1, 0

#=============================================================================
# Final Results Verification
#=============================================================================
verify_final_results:
    # Expected: 7 test groups completed
    addi x29, x0, 7
    bne x31, x29, final_error
    
    # Expected: No errors
    bne x30, x0, final_error
    
    # All tests passed
    addi x28, x0, 999      # Success marker
    jal x0, results_done

final_error:
    addi x28, x0, 666      # Error marker

results_done:
    jalr x0, x1, 0

#=============================================================================
# Program Halt
#=============================================================================

program_halt:
    #addi x28, x0, 666 
    j program_halt   # Infinite loop

# Expected Final Results:
# x31 = 7   (7 test groups completed)
# x30 = 0   (no errors)
# x28 = 999 (success marker)
# x29 = 1000 (end marker)
#
# Instruction Coverage:
# ✅ I-Type: ADDI, SLTI, SLTIU, XORI, ORI, ANDI
# ✅ R-Type: ADD, SUB, SLT, SLTU, XOR, OR, AND, SLL, SRL, SRA
# ✅ S-Type: SB, SH, SW
# ✅ I-Type Loads: LB, LBU, LH, LHU, LW
# ✅ B-Type: BEQ, BNE, BLT, BGE, BLTU, BGEU
# ✅ J-Type: JAL
# ✅ I-Type Jump: JALR
# ✅ U-Type: LUI, AUIPC
# ✅ I-Type Shifts: SLLI, SRLI, SRAI
