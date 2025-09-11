#include <estructura_storage.h>

t_bitarray* bitmap;

t_config* metadata;

t_config* hash;

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
    crear_file("initial_file", "BASE");
    crear_bloque_logico("files/initial_file/BASE/logical_blocks", 0);

    void *mapeo = inicializar_bitmap();

    crear_archivo_hash_bloques();

    char* bloque_fisico = concatenar_path(punto_montaje,"physical_blocks/bloque0000.dat" );

    escribir_archivo(bloque_fisico, "hola mundo");

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
    char* config_block_hash = concatenar_path(punto_montaje,"blocks_hash_index.config");

    if(fresh_start)
    {
        FILE *archivo = fopen(config_block_hash, "w+"); //lo crea, pero si existe lo sobreescribe
        if(archivo) fclose(archivo);
    }else
    {
        FILE *archivo = fopen(config_block_hash, "a+"); //lo crea, pero si existe no lo sobreescribe
        if(archivo) fclose(archivo);
    }

    hash = config_create(config_block_hash);

    log_trace(logger, "Se inicializó correctamente el archvivo BLOCKS_HASH_INDEX");

}



void crear_bloque_logico(char* path, int numero)
{
    char* aux = string_from_format("bloque%04d.dat", numero);

    char* bloque_logico = string_from_format("%s/%s/%s",punto_montaje, path, aux);

    char* bloque_fisico = concatenar_path(punto_montaje,"physical_blocks/bloque0000.dat");

    link(bloque_fisico, bloque_logico);
}

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

void asociar_hash_block(char* bloque_fisico)
{
    char* contenido = leer_archivo(bloque_fisico);
    int largo= strlen(contenido);
    char* md5 = crypto_md5(contenido, largo);

    free(contenido);

    int numero = obtener_numero_bloque(bloque_fisico);

    char* bloque = string_from_format("BLOCK_%04d", numero);

    if(config_has_property(hash, md5))
    {

    }else
    {
        config_set_value(hash, md5, bloque);
        config_save(hash);
    }

    free(md5);
    free(bloque);
}

int obtener_numero_bloque(char* path) {
    const char* nombre = strrchr(path, '/');  
    nombre++;                            
    int numero;
    sscanf(nombre, "bloque%d.dat", &numero);
    return numero;
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
        
        char* nombre_bloque = string_from_format("%s/physical_blocks/bloque%04d.dat", punto_montaje, i); //el %04d hace que tenga 4 digitos

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

char* leer_archivo(char* path)
{
    FILE* f = fopen(path, "r");

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* buffer = malloc(size + 1);

    size_t leidos = fread(buffer, 1, size, f);
    buffer[leidos] = '\0';  

    fclose(f);
    return buffer;
}

int escribir_archivo(char* path, char* contenido)
{
    FILE* f = fopen(path, "w");

    size_t escritos = fwrite(contenido, 1, strlen(contenido), f);
    fclose(f);

    if (escritos < strlen(contenido)) {
        fprintf(stderr, "Error: no se escribieron todos los bytes\n");
        return -1;
    }
    return 0;
}
