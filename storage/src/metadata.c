#include <metadata.h>

t_config* metadata;

void crear_metadata_config(char* path)
{
    char* config = concatenar_path(punto_montaje, path);
    FILE* archivo = fopen(config, "w+");
    fclose(archivo);

    metadata = config_create(config);

    config_set_value(metadata, "TAMAÑO", "0");
    config_set_value(metadata, "BLOCKS", "[]");
    config_set_value(metadata, "ESTADO", "WORK_IN_PROGRESS");

    config_save(metadata);
}

void cambiar_estado_metadata(char* path, char* nuevo_estado)
{
    t_config* meta = config_create(path);
    config_set_value (meta, "ESTADO", nuevo_estado);
    config_save(meta);
    config_destroy(meta);
}

int cambiar_tamanio_metadata(char* path, char* nuevo_tamanio)
{
    t_config* meta = config_create(path);
    int tamanio_viejo = config_get_int_value(meta, "TAMAÑO");
    config_set_value (meta, "TAMAÑO", nuevo_tamanio);
    config_save(meta);
    config_destroy(meta);
    return tamanio_viejo;
}

int agregar_bloque_metadata(char* path, int bloque)
{
    t_config* meta = config_create(path);
    char** blocks = config_get_array_value(meta, "BLOCKS");

    int contador = 0;

    while (blocks && blocks[contador]) contador++;

    char** nuevos = malloc(sizeof(char*) * (contador + 2));
    for (int i = 0; i < contador; i++) {
        nuevos[i] = string_duplicate(blocks[i]);
    }

    nuevos[contador] = string_from_format("%d", bloque);
    nuevos[contador + 1] = NULL;

    char* blocks_str = string_new();
    string_append(&blocks_str, "[");
    for (int i = 0; i <= contador; i++) {
        string_append(&blocks_str, nuevos[i]);
        if (i < contador) string_append(&blocks_str, ",");
    }
    string_append(&blocks_str, "]");

    config_set_value(metadata, "BLOCKS", blocks_str);
    config_save(metadata);

    
    free(blocks_str);
    for (int i = 0; i <= contador; i++) free(nuevos[i]);
    free(nuevos);
    string_array_destroy(blocks);
}


