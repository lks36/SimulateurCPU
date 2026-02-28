#include "hash.h"
#include <stdlib.h>
#include <string.h>


unsigned long simple_hash(const char* str){
    /*cette fonction permet de convertir une chaˆıne de caract`eres en un indice dans la table de hachage.*/
    int sum=0;
    int i=0;
    while(str[i] != '\0'){
        sum+=(int)(str[i])*(int)(str[i]);
        i++;
    }
    return sum%TABLESIZE;
}

HashMap* hashmap_create(){
    /*cette fonction permet d’allouer dynamiquement une table de hachage et d’initialiser ses cases à 0.*/
    HashMap* new = (HashMap*)malloc(sizeof(HashMap));
    new->size = TABLESIZE;
    new->table = calloc(new->size, sizeof(HashEntry));
    return new;
}

int hashmap_insert(HashMap* map, const char* key, void *value){
    /*cette fonction permet d’insérer un élément dans la table de hachage.*/
    int index = simple_hash(key);
    int i = index;
    //si cases pas vide, on applique le mécanisme de probing linéaire
    while(map->table[i].key != 0){
        if(i=index){return 0;}
        i=(i+1)%map->size;
    }
    //sinon
    map->table[i].key = strdup(key);
    map->table[i].value = value;
    return i;
}

void* hashmap_get(HashMap* map, const char* key){
    /*cette fonction permet de récupérer un élément à partir de sa clé.*/
    int index = simple_hash(key);
    //si l'index ne déborde pas
    if(map->size > index){
        void *data = map->table[index].value;
        return data;
    }
    //sinon
    return NULL;
}

int hashmap_remove(HashMap *map, const char* key){
    /*cette fonction permet de supprimer un élément de la table de hachage tout en assurant la continuité du sondage linéaire.*/
    int index = simple_hash(key);
    free(map->table[index].key);
    free(map->table[index].value);
    map->table[index].key = NULL;
    map->table[index].value = NULL;
    return 1;
}

void hashmap_destroy(HashMap* map){
    /*cette fonction permet de libérer toute la mémoire allouée à la table de hachage.*/
    for(int i; i<map->size; i++){
        free(map->table[i].key);
        free(map->table[i].value);
    }
    free(map->table);
    free(map);
}
