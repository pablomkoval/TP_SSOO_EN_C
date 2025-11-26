#include <storage.h>

t_log *logger;
t_config *config;
t_config *config_superblock;

int main(int argc, char *argv[])
{
    if(argc < 2){
    printf("Faltaron argumentos para iniciar el worker");
    return EXIT_FAILURE;
    }
    char* config_name = argv[1];

    worker_id_por_socket = dictionary_create();
    inicializar_mutexes();
    config = iniciar_config(config_name);
    logger = iniciar_logger();
    config_superblock = iniciar_config_superblock();

    crear_directorios_y_archivos();
 
    int socket_worker = iniciar_servidor(puerto_escucha, logger);
    lanzar_servidor(socket_worker);

    pause();

    return 0;
}