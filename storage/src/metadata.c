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
    config_set_value (meta, "ESTADO", nuevo_estado;
    config_save(meta);
    config_destroy(meta);
}

int cambiar_tamanio_metadata(char* path, char* nuevo_tamanio)
{
    t_config* meta = config_create(path);
    int tamanio_viejo = config_get_int_value(meta, "TAMAÑO")
    config_set_value (meta, "TAMAÑO", nuevo_tamanio;
    config_save(meta);
    config_destroy(meta);
    return tamanio_viejo;
}



