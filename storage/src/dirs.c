#include <dirs.h>

void crear_directorio(const char* path) {
    mkdirat(fd, path, 0777);
}

int crear_file(char* file, char* tag)
{
    char* path = concatenar_path("files", file);

    if(file_tag_existe(file)) return -1;

    crear_directorio(path);
    crear_tag(path, tag);

    free(path);
    
    return 1;
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

int copiar_tag(char* file_origen, char* tag_origen, char* file_destino, char* tag_destino)
{
    

    char* path_origen = string_from_format("%s/files/%s/%s", punto_montaje, file_origen, tag_origen);

    char* path_destino = string_from_format("%s/files/%s/%s", punto_montaje, file_destino, tag_destino);
    
    if(file_tag_existe(path_destino)) return -1;

    crear_file(file_destino, tag_destino);

    char* comando = string_from_format("cp -r '%s/.' '%s'", path_origen, path_destino);

    system(comando);

    free(comando);

    cambiar_estado_metadata(path_destino, "WORK_IN_PROGRESS");

    return 1;
}

//CASOS ERROR

bool file_tag_existe(char* path)
{
    if(access(path, F_OK) == 0)
    {
        return 1;
    }
    else return 0;
}





