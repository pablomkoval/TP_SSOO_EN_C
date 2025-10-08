#ifndef DIRS_H 
#define DIRS_H

#include <storage.h>

void crear_tag(char* file_path, char* tag);
char* concatenar_path(char* path, char* suma);
void crear_directorio(const char* path);
int crear_file(char* file, char* tag);
int copiar_tag(char* origen, char* destino, char* a, char* b);
bool file_tag_existe(char* path);

#endif