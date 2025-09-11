#include <metadata.h>

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