#ifndef SEG
#define SEG
#include "hash.h"


typedef struct segment{
    int start; //Position du début (adresse) du segment dans la mémoire
    int size; // Taille du segement (en unités de memoire)
    struct segment* next; //pointeur vers le segment suivant 
}Segment;

typedef struct memoryhandler{
    void **memory; //Tableau de pointeurs vers la memoire allouée
    int total_size;
    Segment *free_list;
    HashMap *allocated; // Table de Hachage (nom, segment)
} MemoryHandler;

MemoryHandler *memory_init(int size);
void free_memory(MemoryHandler* mem);
Segment* find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev);
int create_segment(MemoryHandler *handler, const char *name, int start, int size);
void fusion_segments(Segment* s1, Segment*s2);
int remove_segment(MemoryHandler* handler, const char *name);
#endif