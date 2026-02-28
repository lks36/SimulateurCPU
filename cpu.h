#ifndef _CPU_
#define _CPU_

#include "segment.h"
#include "parser.h"
#include "hash.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct{
    MemoryHandler* memory_handler;
    HashMap* context;
    HashMap* constant_pool;
} CPU;


int search_and_replace(char** str, HashMap *values);
char* trim(char* str);

CPU *cpu_init(int memory_size);
CPU* cpu_destroy(CPU* cpu);
void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data);
void* load(MemoryHandler *handler, const char* segment_name, int pos);
void allocate_variables(CPU *cpu, Instruction** data_instructions, int data_count);
void print_data_segment(CPU* cpu);
CPU* setup_test_environnement();
int resolve_constants(ParserResult* result);
Instruction* fetch_next_instruction(CPU* cpu);
int push_value(CPU* cpu, int value);
int pop_value(CPU* cpu, int* dest);
#endif