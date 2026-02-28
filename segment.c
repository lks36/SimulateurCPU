#include "segment.h"
#include <stdlib.h>
#include <stdio.h>


MemoryHandler *memory_init(int size){
    /* cette fonction permet  d’initialiser le gestionnaire de mémoire. */
    MemoryHandler* handler = (MemoryHandler*)malloc(sizeof(MemoryHandler));
    handler->total_size = size;
    handler->free_list = (Segment*)malloc(sizeof(Segment));
    handler->free_list->start = 0;
    handler->free_list->size = size;
    handler->free_list->next = NULL;
    handler->allocated = hashmap_create();
    handler->allocated->size = size;
    handler->memory = (void**)calloc(size, sizeof(void*));
    return handler;
}

void free_memory(MemoryHandler* mem){
    /* cette fonction permet de libérer l'ensemble de mémoires */
    Segment* tmp = mem->free_list;
    while(tmp){
        mem->free_list = mem->free_list->next;
        free(tmp);
        tmp = mem->free_list;
    }
    hashmap_destroy(mem->allocated);
    
    for(int i=0; i<mem->allocated->size; i++){
        if(mem->memory[i]) free(mem->memory[i]);
    }
    free(mem->memory);
}

Segment* find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev){
    /* cette fonction permet de savoir s'il est possible d'allouer un segment commençant à une position donnée */
    Segment* tmp = handler->free_list;
    //s'il existe un segment libre
    while(tmp->next){
        if((tmp->next->start <= start) && (tmp->next->size >= start+size)){
            (*prev) = tmp;
            return tmp->next;
        }
        tmp = tmp->next;
    }
    if(tmp) return tmp;
    //sinon
    return NULL;
}

int create_segment(MemoryHandler *handler, const char *name, int start, int size){
    /*  cette fonction permet d'allouer dynamiquement un segment de mémoire de taille size à l'adresse mémoire start. */
    Segment* prev = NULL;
    Segment* freeSeg = find_free_segment(handler, start, size, &prev);
    int total_free_size;
    if(freeSeg == NULL){
        return 0;
    }
    Segment* newSeg = (Segment*)malloc(sizeof(Segment));
    newSeg->start = start;
    newSeg->size = size;
    newSeg->next = NULL;
    
    //en fonction des cas
    if((freeSeg->start == start) && (freeSeg->size == size)){
        
        //supprimer le free seg et relier
        if(prev){
            prev->next = freeSeg->next;
        }else{
            handler->free_list = freeSeg->next;
        }
        free(freeSeg);
        //inserer dans la hashmap
        hashmap_insert(handler->allocated, name, newSeg);
        return 1;
    }
    if((freeSeg->start < start) && (freeSeg->size == size)){
        //reduire la taille du free seg à droite
        freeSeg->size = newSeg->start - freeSeg->start;
        //inserer dans la hashmap
        hashmap_insert(handler->allocated, name, newSeg);
        return 1;
    }
    if((freeSeg->start == start) && (freeSeg->size > size)){
        //reduire la taille du free seg à gauche
        freeSeg->start = freeSeg->start + newSeg->size;
        freeSeg->size -= freeSeg->start;
        //inserer dans la hashmap
        hashmap_insert(handler->allocated, name, newSeg);
        return 1;
    }
    if((freeSeg->start == start) && (freeSeg->size > size)){
        //split le free seg (créer deux free seg et supprimer le seg)
            total_free_size = freeSeg->size;
            freeSeg->size = newSeg->start - freeSeg->start;

            Segment* rightSeg = (Segment*)malloc(sizeof(Segment));
            rightSeg->start = start + size;
            rightSeg->size = total_free_size - size - freeSeg->size;

            //inserer nouveau free seg (relier les pointeurs)
            rightSeg->next = freeSeg->next;
            freeSeg->next = rightSeg;
        //inserer dans la hashmap
        hashmap_insert(handler->allocated, name, newSeg);
        return 1;
    }
    printf("segment non alloue");
    return 0;
}

void fusion_segments(Segment* s1, Segment*s2){
    /* cette fonction permet de fusionnner deux segements, on suppose que la fonction est correctement utilisée */
    s1->size = s1->size + s2->size;
    s1->next = s2->next;
    //libérer un segment
    free(s2);
}

int remove_segment(MemoryHandler* handler, const char *name){
    /* cette fonction permet de liérer un segment de mémoire alloué et de l'ajouter à la liste des segments libres  */
    Segment* toDelete = (Segment*)hashmap_get(handler->allocated, name);
    int delStart = toDelete->start;
    int delSize = toDelete->size;
    
    Segment* listeSeg = handler->free_list;
    while(listeSeg){
        if((listeSeg->start + listeSeg->size) == (toDelete->start-1)){
            //on a trouvé un segment libre fusionnable AVANT le segment a supprimer
            //on relie le suivant pour pouvoir fusionner
            toDelete->next = listeSeg->next;
            fusion_segments(listeSeg, toDelete); /*ici : on fusionne le segment libre avec le segment désalloué en ayant relié avec le segment suivant*/
            return 1;
        }
        listeSeg = listeSeg->next;
    }
    /* On n'a pas trouvé de segment libre fusionnable
    on ajoute le segment libre à la liste des segments libres */
    toDelete->next = handler->free_list;
    handler->free_list = toDelete;
    return 1;
}