#include <dirs.h>

void crear_directorio(const char* path) {
    mkdirat(fd, path, 0777);
}

void crear_file(char* file, char* tag)
{
    char* path = concatenar_path("files", file);
    crear_directorio(path);

    crear_tag(path, tag);
}

void crear_tag(char* file_path, char* tag)
{
    char* path = concatenar_path(file_path, tag);
    crear_directorio(path);

    char* path_blocks = concatenar_path(path, "logical_blocks");
    crear_directorio(path_blocks);

    char* path_metadata = concatenar_path(path, "metadata.config");
    crear_metadata_config(path_metadata);

    free(path);
    free(path_blocks);
    free(path_metadata);
}

char* concatenar_path(char* path, char* suma)
{
    char* direccion_archivo = string_from_format("%s/%s", path, suma);
    return direccion_archivo;
}