#include <estructura_storage.h>

t_bitarray* bitmap;

t_config* metadata;

int fd;

void crear_directorios_y_archivos()
{
    fd = open(punto_montaje, O_RDONLY | O_DIRECTORY);

    if(fresh_start)
    {
        formatear_volumen();
    }
    crear_directorio("files");
    
    crear_directorio("physical_blocks");
    crear_bloques_fisicos();
    crear_file("initial_file", "/BASE");
    crear_bloque_logico("raiz/files/initial_file/BASE/logical_blocks", "/0000.dat");

    void *mapeo = inicializar_bitmap();

    crear_archivo_hash_bloques();

    log_trace(logger, "se terminaron de crear todos los archivos y directorios necesarios");
}

void* inicializar_bitmap()
{
    
    char* direccion_archivo = concatenar_path(punto_montaje,"bitmap.bin");
    int tamanio = ( cant_blocks + 7 )  /  8;

    FILE *archivo;
    
    if(fresh_start)
    {
        archivo = fopen(direccion_archivo, "w+"); //lo crea, pero si existe lo sobreescribe

    }else
    {
        archivo = fopen(direccion_archivo, "a+"); //lo crea, pero si existe no lo sobreescribe
    }
    
    int fildes = fileno(archivo);
    ftruncate(fildes, tamanio) ;
    void *mapeo = mmap(0, tamanio, PROT_WRITE | PROT_READ, MAP_SHARED, fildes, 0);
    bitmap = bitarray_create_with_mode (mapeo, tamanio, LSB_FIRST);

    log_trace(logger, "Se inicializó correctamente el BITMAP");
    
    return mapeo;
}

void crear_archivo_hash_bloques()
{
    char* direccion_archivo = string_from_format("%s%s", punto_montaje, "blocks_hash_index.config");

    if(fresh_start)
    {
        FILE *archivo = fopen(direccion_archivo, "w+"); //lo crea, pero si existe lo sobreescribe
        if(archivo) fclose(archivo);
    }else
    {
        FILE *archivo = fopen(direccion_archivo, "a+"); //lo crea, pero si existe no lo sobreescribe
        if(archivo) fclose(archivo);
    }

    log_trace(logger, "Se inicializó correctamente el archvivo BLOCKS_HASH_INDEX");

}

void crear_bloque_logico(char* path, char* numero)
{

    char* path_bloque = concatenar_path(path, numero);

    FILE *archivo = fopen(path_bloque, "w+");
    if(archivo) fclose(archivo);
}

void crear_directorio(const char* path) {
    mkdirat(fd, path , 0777);
}

void crear_file(char* file, char* tag)
{
    char* path = concatenar_path("files/", file);
    crear_directorio(path);

    crear_tag(path, tag);
}

void crear_tag(char* file_path, char* tag)
{
    char* path = concatenar_path(file_path, tag);
    crear_directorio(path);

    char* path_blocks = concatenar_path(path, "/logical_blocks");
    crear_directorio(path_blocks);

    char* path_metadata = concatenar_path(path, "/metadata.config");
    crear_metadata_config(path_metadata);

    free(path);
    free(path_blocks);
    free(path_metadata);
}

char* concatenar_path(char* path, char* suma)
{
    char* direccion_archivo = string_from_format("%s%s", path, suma);
    return direccion_archivo;
}

void asociar_hash_block()
{
    
}

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

void crear_bloques_fisicos()
{
    for (int i = 0; i < cant_blocks; i++) {
        
        char* nombre_bloque = string_from_format("%sphysical_blocks/bloque%04d.dat", punto_montaje, i); //el %04d hace que tenga 4 digitos

        FILE* archivo = fopen(nombre_bloque, "w+");
        if (archivo == NULL) {
            perror("fopen");
            free(nombre_bloque);
            exit(EXIT_FAILURE);
        }
        fclose(archivo);

        free(nombre_bloque);
    }
}

void formatear_volumen()
{
    char* dir_files  = concatenar_path(punto_montaje, "files");
    char* dir_blocks = concatenar_path(punto_montaje, "physical_blocks");

    char* borrar_files  = string_from_format("rm -rf '%s'", dir_files);
    char* borrar_blocks = string_from_format("rm -rf '%s'", dir_blocks);

    system(borrar_files);
    system(borrar_blocks);

    free(borrar_files);
    free(borrar_blocks);

    log_trace(logger, "Se formateó correctamente el volumen");
}
