#ifndef _ADDR_
#define _ADDR_
#include <regex.h>
#include "cpu.h"

int matches(const char* pattern, const char* string);
void* immediate_addressing(CPU* cpu, char* operand);
void* register_addressing(CPU* cpu, const char* operand);
void* memory_direct_addressing(CPU* cpu, const char* operand);
void* register_indirect_addressing(CPU* cpu, const char* operand);
void* handle_MOV(CPU* cpu, void* src, void* dest);
int handle_instruction(CPU* cpu, Instruction* instr, void* src, void* dest);
int execute_instruction(CPU* cpu, Instruction *instr);
Instruction* fetch_next_instruction(CPU* cpu);
int run_program(CPU* cpu);
void* segment_override_addressing(CPU* cpu, const char* operand);
int find_free_address_strategy(MemoryHandler* handler, int size, int strategy);

#endif