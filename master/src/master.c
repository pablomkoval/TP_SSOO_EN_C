#include <master.h>

pthread_t hilo_main_escucha;
pthread_t hilo_planificador;

t_dictionary* diccionario_querys;   // key: qid, valor: qcb
t_dictionary* diccionario_workers;  // key: id_worker, valor: socket_worker
t_dictionary* diccionario_exec;     //key: id_worker, valor: qcb
//El diccionario de exec es el representante del estado running, asigna cada worker a su query que esta laburando

pthread_mutex_t mutex_diccionario_querys;
pthread_mutex_t mutex_diccionario_workers;
pthread_mutex_t mutex_diccionario_exec;

t_config* config_master;
t_log* logger;

int main(int argc, char* argv[]){
    
    inicializar_diccionarios_y_semaforos();

    if(argc < 2){
        printf("Faltaron argumentos para iniciar el modulo master");
        return EXIT_FAILURE;
    }

    char* archivo_config = argv[1];
    // char* archivo_config = "master.config"; !! No va, solo para debugeo

    config_master = iniciar_config(archivo_config);
    logger = iniciar_logger();
    log_debug(logger, "se iniciaron logger y config");
    inicializar_planificador();

    int socket_escucha = iniciar_servidor(puerto_escucha, logger);

    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_escucha;
    
    pthread_create(&hilo_main_escucha, NULL, funcion_main_escucha, socket_ptr);
    pthread_detach(hilo_main_escucha);

    
    pthread_create(&hilo_planificador, NULL, planificador, NULL);
    pthread_detach(hilo_planificador);

    pause();

    return 0;
}

void inicializar_diccionarios_y_semaforos(){
    diccionario_workers = dictionary_create();
    diccionario_exec = dictionary_create();
    diccionario_querys = dictionary_create();
    pthread_mutex_init(&mutex_diccionario_workers, NULL);
    pthread_mutex_init(&mutex_diccionario_exec, NULL);
    pthread_mutex_init(&mutex_diccionario_querys, NULL);
}