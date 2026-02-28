#include "cpu.h"
#include <stdio.h>
#include <string.h>

CPU *cpu_init(int memory_size){
    /* cette fonction permet  d’initialiser le processeur simulé */
    CPU* cpu = (CPU*)malloc(sizeof(CPU));
    //RAM
    cpu->memory_handler = memory_init(memory_size);
    //registres
    cpu->context = hashmap_create();
    hashmap_insert(cpu->context, "AX", calloc(sizeof(int), 1));
    hashmap_insert(cpu->context, "BX", calloc(sizeof(int), 1));
    hashmap_insert(cpu->context, "CX", calloc(sizeof(int), 1));
    hashmap_insert(cpu->context, "DX", calloc(sizeof(int), 1));
    hashmap_insert(cpu->context, "IP", calloc(sizeof(int), 1));
    hashmap_insert(cpu->context, "ZF", calloc(sizeof(int), 1));
    hashmap_insert(cpu->context, "SF", calloc(sizeof(int), 1));
    hashmap_insert(cpu->context, "SP", calloc(sizeof(int), 1));
    hashmap_insert(cpu->context, "BP", calloc(sizeof(int), 1));
    hashmap_insert(cpu->context, "ES", calloc(sizeof(int), 1));
    cpu->constant_pool = hashmap_create();


    create_segment(cpu->memory_handler, "SS", 0, 128);
    create_segment(cpu->memory_handler, "DS", 128, 20);
    create_segment(cpu->memory_handler, "CS", 148, 20);

    int* sp = hashmap_get(cpu->context, "SP");
    int* bp = hashmap_get(cpu->context, "BP");
    Segment* ss = hashmap_get(cpu->memory_handler->allocated, "SS");
    *sp = ss->start;
    *bp = ss->start + ss->size -1; 

    int* es = hashmap_get(cpu->context, "ES");
    *es = -1;

    return cpu;
}

CPU* setup_test_environnement(){
    //Initialiser le CPU
    CPU* cpu = cpu_init(1024);
    if(!cpu){
        fprintf(stderr, "Error : CPU initialization failed\n");
        return NULL;
    }
    //Initialiser les registres avec des valeurs specifiques
    int *ax = (int *)hashmap_get(cpu->context, "AX");
    int *bx = (int *)hashmap_get(cpu->context, "BX");
    int *cx = (int *)hashmap_get(cpu->context, "CX");
    int *dx = (int *)hashmap_get(cpu->context, "DX");

    *ax = 3;
    *bx = 6;
    *cx = 100;
    *dx = 0;

    
    //Creer et initialiser le segment de données
    if(!hashmap_get(cpu->memory_handler->allocated, "DS")){
        create_segment(cpu->memory_handler, "DS", 128, 20);
    }
    //Initialiser le segment de donnees avec des valeurs de test
    for(int i=0; i<10; i++){
        int* value = (int*)malloc(sizeof(int));
        *value = i*10 + 5; //Valeurs 5, 15, 25, 35
        store(cpu->memory_handler, "DS", i, value);
    }


    //Initialiser les instructions dans le code
    if(!hashmap_get(cpu->memory_handler->allocated, "CS")){
        create_segment(cpu->memory_handler, "CS", 148, 20);
    }
    Segment* cs = hashmap_get(cpu->memory_handler->allocated, "CS");
    Instruction* instr = parse_code_instruction("start: MOV AX,x", cpu->memory_handler->allocated, 1);
        
    cpu->memory_handler->memory[cs->start] = instr;
    cpu->memory_handler->memory[cs->start+1] = parse_code_instruction("loop: ADD AX,y", cpu->memory_handler->allocated, 1);
    int *ip = (int *)hashmap_get(cpu->context, "IP");
    *ip = cs->start;


    printf("Test environnement initialized.\n");
    return cpu;
}

CPU* cpu_destroy(CPU* cpu){
    /* cette fonction permet de libérer toutes les ressources allouées par le processeur simulé */
    free_memory(cpu->memory_handler);
    hashmap_destroy(cpu->context);
    hashmap_destroy(cpu->constant_pool);
    free(cpu);
    return NULL;
}

void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data){
    /* cette fonction permet de stocker une donnée data à la position pos de segment name, supposons segment name est bien un segment alloué, et que pos ne
dépasse pas la taille du segment.*/
    Segment* seg = hashmap_get(handler->allocated, segment_name);
    if(seg){
        handler->memory[seg->start+pos] = data;
        return data;
    }
    return NULL;
    
}

void* load(MemoryHandler *handler, const char* segment_name, int pos){
    /* cette fonction permet de  de récupérer la donnée stockée à la position pos de segment name. */
    Segment* seg = hashmap_get(handler->allocated, segment_name);
    if(seg){
        return handler->memory[seg->start+pos];
    }
    return NULL;
}


void allocate_variables(CPU *cpu, Instruction** data_instructions, int data_count){
    /* cette foction permet d'allouer dynamiquement le segment de données en fonction des déclarations récupérées par le parser. */
    /*Calcul de l'espace nécessaire :*/
    int necessary_space = 0;
    
    /*Ce tableau servira a stocker temporairement les poineurs des éléments*/
    void** datas = calloc(cpu->memory_handler->total_size,sizeof(void*));

    /*On parcourt chaque instruction*/
    Instruction* curr;
    char* content;
    char* token;
    for(int i=0; i<data_count; i++){
        curr = data_instructions[i];
        content = strdup(curr->operand2);
        /*On compte les éléments et on ajoute le pointeur vers chaque élément dans la liste temporaire*/
        char* token = strdup(strtok(content, ","));
        while(token!=NULL){ 
            datas[necessary_space] = (void*)token;
            necessary_space++;
            token = strdup(strtok(NULL, ","));
        }
        free(content);
    }
    /* Declaration du segment DS nécessaire*/
    create_segment(cpu->memory_handler, "DS", last_adress_used, last_adress_used+necessary_space);
    last_adress_used+= necessary_space;
    /*Enregistrer les pointeurs dans le memory handler*/
    int i = 0;
    while(datas[i]){
        store(cpu->memory_handler, "DS", i, datas[i]);
        i++;
    }
    /*Libérer le tableau ayant servi a stocker les pointeurs temporairement*/
    free(datas);
}

void print_data_segment(CPU* cpu){
    /*  cette fonction permet d'afficher le contenu du segment de données (nom "DS") */
    Segment* data = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
    if(!data) return;
    printf("---- PRINTING .DATA SEGMENT ---- \n");
    for(int i=data->start; i<data->start + data->size; i++){
        if(cpu->memory_handler->memory[i]){
            printf("[%d] ", *(int*)cpu->memory_handler->memory[i]);
        }
    }
    printf("\nDimensions : [%d, %d]\n", data->start, data->start+data->size-1);
}

char* trim(char* str){
    while(*str == ' '|| *str == '\t' || *str == '\n' || *str == '\r') str++;

    char* end = str + strlen(str) - 1;
    while(end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')){
        *end = '\0';
        end--;
    }
    return str;
}

int search_and_replace(char** str, HashMap *values){
    if(!str || !*str || !values) return 0;

    int replaced = 0;
    char* input = *str;
    
    //Iterate throug all keys in the hashmap
    for(int i=0; i < values->size; i++){
        if(values->table[i].key && values->table[i].key != TOMBSTONE){
            char *key = values->table[i].key;
            int value = (int)(long)values->table[i].value;

            //Find potential substring match
            char* substr = strstr(input, key);
            if(substr){
                //Construct replacement buffer
                char replacement[64];
                snprintf(replacement, sizeof(replacement), "%d", value);

                //Calculate lengths
                int key_len = strlen(key);
                int repl_len = strlen(replacement);
                int remain_len = strlen(substr + key_len);

                //Create new string
                char *new_str = (char *)malloc(strlen(input) - key_len + repl_len + 1);
                strncpy(new_str, input, substr - input);
                new_str[substr - input] = '\0';
                strcat(new_str, replacement);
                strcat(new_str, substr + key_len);

                //free and update original string
                free(input);
                *str = new_str;
                input = new_str;

                replaced = 1;
            }
        }
    }

    //Trim the final string
    if(replaced){
        char *trimmed = trim(input);
        if(trimmed != input){
            memmove(input, trimmed, strlen(trimmed) + 1);
        }
    }
    return replaced;
}

int resolve_constants(ParserResult* result){
    /* cette fonction remplace les variables par leur adresse dans le segment de données et les étiquettes par leur adresse dans le code. */
    Instruction* curr;
    int res = 0;
    for(int i=0; i<result->data_count; i++){
        curr = result->data_instructions[i];
        res += search_and_replace(&curr->operand2, result->memory_locations);
    }

    for(int i=0; i<result->code_count; i++){
        curr = result->code_instructions[i];
        res += search_and_replace(&curr->operand1, result->labels);
    }
    return res;
}

void allocate_code_segment(CPU* cpu, Instruction** code_instructions, int code_count){
    /* cette fonction alloue et initialise le segment de codes (CS).  */
    //Créer le segment .CODE
    create_segment(cpu->memory_handler, "CS", last_adress_used, code_count);
    last_adress_used += code_count;

    //Inserer les instructions dans la mémoire
    for(int i=0; i<code_count; i++){
        cpu->memory_handler->memory[i+last_adress_used] = code_instructions[i];
    }

    //Initiliasier le pointeur d'instructions à 0
    int* ip = (int*)hashmap_get(cpu->context, "IP");
    *ip = 0;
}

Instruction* fetch_next_instruction(CPU* cpu){
    /* cette fonction récupère l'instruction suivante dans le segment de codes et incrémente le pointeur d'instruction (IP). */
    //Récupérer l'instruction actuelle
    int* ip = (int*)hashmap_get(cpu->context,"IP");
    
    Segment* cs = hashmap_get(cpu->memory_handler->allocated, "CS");
   
    //Verifier les dimensions
    if(!cs) return NULL;
    if((*ip >=cs->start + cs->size) || (*ip < cs->start)) return NULL;
    if(*ip+1 >= cs->start + cs->size) return NULL;
    (*ip)++;
    return (Instruction*)cpu->memory_handler->memory[*ip];
}

int push_value(CPU* cpu, int value){
    /* cette fonction empile une value */
    int* sp = hashmap_get(cpu->context, "SP");
    int* bp = hashmap_get(cpu->context, "BP");
    Segment* ss = hashmap_get(cpu->memory_handler->allocated, "SS");
    if(*sp == ss->size-1){
        printf("[CPU ERROR] Stack overflow\n");
        return -1;
    }
    store(cpu->memory_handler, "SS", *sp - ss->start, &value);
    return 1;
}

int pop_value(CPU* cpu, int* dest){
    /* cette fonction récuère la dernière value */
    int* sp = hashmap_get(cpu->context, "SP");
    int* bp = hashmap_get(cpu->context, "BP");
    Segment* ss = hashmap_get(cpu->memory_handler->allocated, "SS");
    if(*bp == ss->start){
        printf("[CPU ERROR] Stack underflow\n");
        return -1;
    }
    *dest = *(int*)(load(cpu->memory_handler, "SS", *bp - ss->start));
    return 1;
}
