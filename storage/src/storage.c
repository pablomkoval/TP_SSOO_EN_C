#include <storage.h>

t_log *logger;
t_config *config;
t_config *config_superblock;

int main(int argc, char *argv[])
{
    
    config = iniciar_config();
    logger = iniciar_logger();
    config_superblock = iniciar_config_superblock();

    crear_directorios_y_archivos();

    int socket_worker = iniciar_servidor(puerto_escucha, logger);
    lanzar_servidor(socket_worker);

    pause();
    return 0;
}