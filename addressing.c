#include "addressing.h"
#include <string.h>

int matches(const char *pattern, const char *string){
    regex_t regex;
    int result = regcomp(&regex, pattern, REG_EXTENDED);
    if(result) {
        fprintf(stderr, "Regex compilation failed for pattern : %s\n", pattern);
        return 0;
    }
    result = regexec(&regex, string, 0, NULL, 0);
    regfree(&regex);
    return result == 0;
}

void* immediate_addressing(CPU* cpu, char* operand){
    /* cette fonction permet de traiter l'adressage immédiat */
    if(matches("[0-9]*", operand)){
        void* data = hashmap_get(cpu->constant_pool, operand);
        if(!data){
            /*Cas où la valeur n'y est pas déjà stockée*/
            int* constant = (int*)malloc(sizeof(int));
            *constant = atoi(operand);
            hashmap_insert(cpu->constant_pool, operand, constant);
            return operand;
        }
        else{
            return data;
        }
    }else{
        fprintf(stderr, "Erreur dans immediate_addressing(): %s ne match pas avec le pattern [0-9]*\n", operand);
        return NULL;
    }
}

void* register_addressing(CPU* cpu, const char* operand){
    /* cette fonction permet de traiter l'adressage par registre. */
    if(matches("[A-Z]{2}", operand)){
        void* data = hashmap_get(cpu->context, operand);
        if(data){
            return data;
        }
        else{
            return NULL;
        }
    }
    return NULL;
}

void* memory_direct_addressing(CPU* cpu, const char* operand){
    /* cette fonction permet de traiter l'adressage direct par mémoire. */
    if(matches("\[[0-9]*\]", operand)){
        /*Extraction et conversion de l'adresse*/
        char* tmp = strdup(operand);
        char* symbol = strdup(strtok(tmp, "[]"));
        int index = atoi(symbol);
        free(tmp);
        free(symbol);

        /*Récupérer le data segment si il existe*/
        Segment* data_segment = (Segment*)hashmap_get(cpu->memory_handler->allocated, "DS");
        if(!data_segment){
            printf("Pas de data segment\n");
            return NULL;
        }

        /*Trouver l'adresse dans le segment data*/
        void* data = cpu->memory_handler->memory[data_segment->start+index];

        /*renvoyer le pointeur*/
        return data;
    }
    return NULL;
}

void* register_indirect_addressing(CPU* cpu, const char* operand){
    /* cette fonction permet de traiter l'adressage indirect par registre. */
    if(matches("\[[A-Z]{2}\]", operand)){
        char* tmp = strdup(operand);
        char* nom_registre = strdup(strtok(tmp, "[]"));
        void* data = hashmap_get(cpu->context, nom_registre);
        if(data){
            return data;
        }
        return NULL;
    }
    return NULL;
}

void handle_MOV(CPU* cpu, void* src, void* dest){
    /* cette fonction permet de simuler le comportement de l'instruction MOV en pseudo-assembleur. */
    /*Copier le contenu (supposé ici un entier) à l'emplacement src vers dest*/
    if(dest && src){
        memcpy(dest, src, sizeof(int));
    }
}

void* resolve_addressing(CPU* cpu, const char* operand){
    /* cette fonction permet d'identifier automatiquement le mode d'adressage d’un opérande et de résoudre sa valeur. */
    if(matches("[0-9]*", operand)) return immediate_addressing(cpu, operand);
    if(matches("[A-Z]{2}", operand)) return register_addressing(cpu, operand);
    if(matches("\[[0-9]*\]", operand)) return memory_direct_addressing(cpu, operand);
    if(matches("\[[A-Z]{2}\]", operand)) return register_indirect_addressing(cpu, operand);
    if(matches("\[[A-Z]{1}S:[A-Z]{2}\]", operand)) return segment_override_addressing(cpu, operand);
    return NULL;
}

int handle_instruction(CPU* cpu, Instruction* instr, void* src, void* dest){
    /* cette fonction généralise la fonction handle MOV en permettant d'exécuter une instruction dans le CPU en fonction de son mnémonique. */
    if(strcmp(instr->mnemonic, "MOV")==0){
        handle_MOV(cpu, src, dest);
        return 1;
    }
    if(strcmp(instr->mnemonic, "ADD")==0){
        *(int*)dest = *(int*)src + *(int*)dest;
        return 1;
    }
    if(strcmp(instr->mnemonic, "CMP")==0){
        int* zf = (int*)hashmap_get(cpu->context,"ZF");
        int* sf = (int*)hashmap_get(cpu->context,"SF");
        int res = *(int*)dest - *(int*)src;
        if(res == 0){
            *zf = 1;
        }else if (res < 0){
            *sf = 1;
        }
        return 1;
    }
    if(strcmp(instr->mnemonic, "JMP")==0){
        int address = atoi(instr->operand1);
        int* ip = (int*)hashmap_get(cpu->context,"IP");
        *ip = address;
        return 1;
    }
    if(strcmp(instr->mnemonic, "JZ")==0){
        int address = atoi(instr->operand1);
        int* zf = (int*)hashmap_get(cpu->context,"ZF");
        int* ip = (int*)hashmap_get(cpu->context,"IP");
        if(*zf == 1){
            *ip = address;
        }
        return 1;
    }
    if(strcmp(instr->mnemonic, "JNZ")==0){
        int address = atoi(instr->operand1);
        int* zf = (int*)hashmap_get(cpu->context,"ZF");
        int* ip = (int*)hashmap_get(cpu->context,"IP");
        if(*zf == 0){
            *ip = address;
        }
        return 1;
    }
    if(strcmp(instr->mnemonic, "HALT")==0){
        int* ip = (int*)hashmap_get(cpu->context,"IP");
        Segment* cs = hashmap_get(cpu->memory_handler->allocated, "CS");
        *ip = cs->start+cs->size;
        return 1;    
    }
    if(strcmp(instr->mnemonic, "PUSH")==0){
        if(instr->operand1 == NULL){
            int* ax = (int*)hashmap_get(cpu->context,"AX");
            push_value(cpu, *ax);
        }
        else{
            int* constant = (int*)malloc(sizeof(int));
            *constant = atoi(instr->operand2);
            push_value(cpu, *constant);
        }
        return 1;
    }
    if(strcmp(instr->mnemonic, "POP")==0){
        if(instr->operand1 == NULL){
            int* ax = (int*)hashmap_get(cpu->context,"AX");
            pop_value(cpu, ax);
        }
        else{
            pop_value(cpu, (int*)hashmap_get(cpu->context, instr->operand1));  
        }
        return 1;
    }
    if(strcmp(instr->mnemonic, "ALLOC")==0){
        alloc_es_segment(cpu);
        return 1;
    }
    if(strcmp(instr->mnemonic, "FREE")==0){
        free_ES_segment(cpu);
    }
    return 0;
}

int execute_instruction(CPU* cpu, Instruction *instr){
    /* cette fonction permet de résoudre les adresses des opérandes en fonction du type d'adressage, puis délègue l'exécution proprement dite à la fonction handle instruction. */
    void* addr1 = resolve_addressing(cpu, instr->operand1);
    void* addr2 = resolve_addressing(cpu, instr->operand2);
    handle_instruction(cpu, instr, addr1, addr2);
    return 0;
}


void* segment_override_addressing(CPU* cpu, const char* operand){
    /* cette fonction récupère et retourne la donnée stockée dans le segment à la position spécifiée. */
    if(matches("\[[A-Z]{1}S:[A-Z]{2}\]", operand)){
        char* tmp = strdup(operand);
        char* segment_name = strdup(strtok(tmp, "[:]"));
        char* registre_name = strdup(strtok(NULL, "[:]"));
        free(tmp);

        Segment* segment = hashmap_get(cpu->memory_handler->allocated, segment_name);
        int* registre = hashmap_get(cpu->context, registre_name);

        return cpu->memory_handler->memory[segment->start+*registre];
    }
    return NULL;
}

int find_free_address_strategy(MemoryHandler* handler, int size, int strategy){
    /* cette fonction permet de trouver le segment libre à utiliser selon la stratégie (0, 1 ou 2) spécifiée. */
    Segment* list = handler->free_list;
    

    switch (strategy)
    {
    case 0:
        //On recherche la premiere taille valide
        while(list){
            if(list->size > size){
                return list->start;
            }
            list = list->next;
        }
        return -1;
    case 1:
        //On recherche la taille minimum
        int addr_min = -1;
        int size_min = list->size;
        while(list){
            if((list->size <= size_min) && (list->size >= size)){
                //On a trouvé une nouvelle taille minimale et valide
                addr_min = list->start;
            }
            list = list->next;
        }
        return addr_min;
    case 2:
        // On recherche la plus grande taille
        int addr_max = list->start;
        int size_max = list->size;
        while(list){
            if((list->size > size_max)){
                addr_max = list->start;
            }
            list = list->next;
        }
        return addr_max;
    default:
        return -1;
    }
}

int alloc_es_segment(CPU* cpu){
    /* cette fonction alloue dynamiquement un segment ES */
    int *ax = (int *)hashmap_get(cpu->context, "AX");
    int *bx = (int *)hashmap_get(cpu->context, "BX");
    int *zf = (int *)hashmap_get(cpu->context, "ZF");
    int address = find_free_address_strategy(cpu->memory_handler, *ax, *bx);
    if(address == -1){
        //Cas d'échec de l'allocation
        *zf = 1;
        return -1;
    }else{
        //Allocation réussie
        *zf = 0;
    }

    if(!create_segment(cpu->memory_handler, "ES", address, ax)) printf("Erreur lors de l'allocation de ES.\n");
    
    int *es = (int *)hashmap_get(cpu->context, "ES");
    *es = address;  
    return address;
}

int free_ES_segment(CPU* cpu){
    /* cette fonction r libère le segment ES  */
    Segment* es = hashmap_get(cpu->memory_handler->allocated, "ES");
    if(!es) return 0;
    for(int i=es->start; i<es->start+es->size; i++){
        if(cpu->memory_handler->memory[i]) free(cpu->memory_handler->memory[i]);
    }
    remove_segment(cpu->memory_handler, "ES");
    int *esr = (int *)hashmap_get(cpu->context, "ES");
    *esr = -1;
    return 1;
}
