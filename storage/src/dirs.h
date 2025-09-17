#ifndef DIRS_H 
#define DIRS_H

#include <storage.h>

void crear_tag(char* file_path, char* tag);
char* concatenar_path(char* path, char* suma);
void crear_directorio(const char* path);
void crear_file(char* file, char* tag);
void copiar_tag(char* origen, char* destino);

#endif