#ifndef DIRS_H 
#define DIRS_H

#include <storage.h>

void crear_tag(char* tag_path);
char* concatenar_path(char* path, char* suma);
void crear_directorio(const char* path);
int crear_file(char* file, char* tag);
int copiar_tag(int query_id, char* origen, char* destino, char* a, char* b);
bool file_tag_existe(char* path);
int eliminar_tag(int query_id, char* file, char* tag);
void dupear_hard_links(int query_id, char *file_origen, char *tag_origen, char *file_destino, char *tag_destino);
void eliminar_de_block_hash_index(char* bloque_logico);

#endif