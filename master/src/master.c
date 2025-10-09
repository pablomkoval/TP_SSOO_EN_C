#include <master.h>

pthread_t hilo_main_escucha;

t_dictionary* diccionario_workers;   // key: id_worker, valor: socket_worker
t_dictionary* diccionario_querys;   //key: id_worker, valor: socket del query a procesar
t_config* config_master;
t_log* logger;

int main(int argc, char* argv[]){
    
    if(argc < 2){
        printf("Faltaron argumentos para iniciar el modulo master");
        return EXIT_FAILURE;
    }
    char* archivo_config = argv[1];

    config_master = iniciar_config(archivo_config);
    logger = iniciar_logger();
    log_debug(logger, "se iniciaron logger y config");

    int socket_escucha = iniciar_servidor(puerto_escucha, logger);

    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_escucha;
    
    diccionario_workers = dictionary_create();
    diccionario_querys = dictionary_create();

    pthread_create(&hilo_main_escucha, NULL, funcion_main_escucha, socket_ptr);
    pthread_detach(hilo_main_escucha);

    pause();

    return 0;
}
