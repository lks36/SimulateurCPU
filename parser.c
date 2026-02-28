#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//start
int last_adress_used = 0;

Instruction* parse_data_instruction(const char *line, HashMap* memory_locations){
    /* Cette fonction permer d'analyser et stocker une ligne de la section .DATA d’un programme en pseudo-assembleur. */
    Instruction* inst = (Instruction*)malloc(sizeof(Instruction));
    //créer un copie de la chaîne line, car strtok modifie la chaîne originale.
    char* tmp = strdup(line);
    //appeller strtok une seule fois avec la tmp(la chaîne), ensuite on appelle avec NULL pour continuer.(plus expluquation : https://koor.fr/C/cstring/strtok.wp)
    inst->mnemonic = strdup(strtok(tmp, " "));
    inst->operand1 = strdup(strtok(NULL, " "));
    inst->operand2 = strdup(strtok(NULL, " "));
    //libérer tmp
    free(tmp);

    //parser un potentiel array (trouver le nombre d'éléments)
    int elements_count = 0;
 
    //les instruction suivantes calcule l'adresse de mémoire de la variable :
    //créer un copie de la chaîne line, car strtok modifie la chaîne originale.
    tmp = strdup(inst->operand2);
    char* token = strdup(strtok(tmp, ","));
    //trouver le nombre d'éléments
    while(token!=NULL){
        free(token);
        elements_count++;
        token = strdup(strtok(NULL, ","));
        free(token);
    }
    //libérer tmp.
    free(tmp);

    hashmap_insert(memory_locations, inst->mnemonic, &last_adress_used);
    return inst;
}


Instruction* parse_code_instruction(const char* line, HashMap *labels, int code_count){
    /* cette fonction permet d’analyser et stocker une ligne de la section .CODE. */
    Instruction* inst = (Instruction*)malloc(sizeof(Instruction));
    inst->mnemonic = NULL;

    //trouver si étiquette présente
    int i=0;
    char c = line[i];
    int is_label = 0;
    while((c != ' ') && (c != '\0')){
        c = line[i];
        if(c == ':') {
            is_label = 1;
            break;
        }
        i++;
    }

    const char* separateurs = " :,";
    //créer un copie de la chaîne line, car strtok modifie la chaîne originale.
    char* tmp = strdup(line);
    //si étiquette présente
    if(is_label){
        //Parser le reste de la ligne
        //appeller strtok une seule fois avec la tmp(la chaîne), ensuite on appelle avec NULL pour continuer.(plus expluquation : https://koor.fr/C/cstring/strtok.wp)
        char* label = strtok(tmp, separateurs);
        hashmap_insert(labels, label, &code_count);
        inst->mnemonic = strdup(strtok(NULL, separateurs));
        inst->operand1 = strdup(strtok(NULL, separateurs));
        inst->operand2 = strdup(strtok(NULL, separateurs));
        printf("Ligne parsee avec label : %s %s %s %s\n", label, inst->mnemonic, inst->operand1, inst->operand2);
    }
    //sinon
    else{
        //appeller strtok une seule fois avec la tmp(la chaîne), ensuite on appelle avec NULL pour continuer.
        inst->mnemonic = strdup(strtok(tmp, separateurs));
        inst->operand1 = strdup(strtok(NULL, separateurs));
        inst->operand2 = strdup(strtok(NULL, separateurs));
        printf("Ligne parsee sans label : %s %s %s\n", inst->mnemonic, inst->operand1, inst->operand2);
    }
    //libérer tmp
    free(tmp);
    return inst;
}


ParserResult* init_result(){
    /* cette fonction permet d'initialiser une structure ParserResult. */
    ParserResult* result = (ParserResult*)malloc(sizeof(ParserResult));
    result->data_instructions = (Instruction**)malloc(sizeof(Instruction*)*20);
    result->data_count = 0;
    result->code_instructions = (Instruction**)malloc(sizeof(Instruction*)*20);
    result->code_count = 0;
    result->memory_locations = hashmap_create();
    result->labels = hashmap_create();
    return result;
}

ParserResult* parse(const char* filename){
    /* cette fonction analyse un fichier assembleur complet en identifiant les sections .DATA et .CODE et en traitant chaque ligne de la manière appropriée. */
    FILE* file = fopen(filename, "r");

    ParserResult* result = init_result();
    char buffer[256];
    fgets(buffer, 7, file);
    
    if(strcmp(buffer, ".DATA\n")==0){
        /*Après la ligne .DATA*/
        fgets(buffer, 256, file);
        printf("Entree dans .DATA\n");
         while(strcmp(buffer, ".CODE\n")!=0){
            
            result->data_instructions[result->data_count] = parse_data_instruction(buffer, result->memory_locations);
            result->data_count++;
            fgets(buffer, 256, file);
        }
        printf("Entree dans .CODE\n");
    }
    else{
        fprintf(stderr, "Erreur dans parse(%s)\n", filename);
        return NULL;
    }
    /*Après la ligne .CODE*/
    while(fgets(buffer, 256, file)){
        result->code_instructions[result->code_count] = parse_code_instruction(buffer, result->labels, result->code_count);
        result->code_count++;
    }
    fclose(file);
    return result;
}

void free_instruction(Instruction* inst){
    /* cette fonction permet de supprimer un élément de type Instruction */
    if(inst){
        free(inst->mnemonic);
        free(inst->operand1);
        free(inst->operand2);
    }
}

void free_parser_result(ParserResult *result){
    /* cette fonction permet de supprimer un élément de type ParserResult. */
    for(int i=0; i<result->data_count; i++) free_instruction(result->data_instructions[i]);
    free(result->data_instructions);
    for(int i=0; i<result->code_count; i++) free_instruction(result->code_instructions[i]);
    free(result->code_instructions);
    hashmap_destroy(result->labels);
    hashmap_destroy(result->memory_locations);
}
