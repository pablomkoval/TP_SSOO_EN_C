#include <master.h>

//hacer una funcion que arme una query, como la pcb de antes
// y de ahi hacer lo de cambiar estado y todo ese chiche ¿¿¿¿
// preguntar!!


pthread_t hilo_main_escucha;

t_dictionary* diccionario_querys;    // key: qid, valor: qcb
t_dictionary* diccionario_workers;   // key: id_worker, valor: socket_worker
t_dictionary* diccionario_exec;   //key: id_worker, valor: socket del query a procesar
//El diccionario de exec es el representante del estado running, asigna cada worker a su query que esta laburando

pthread_mutex_t mutex_diccionario;

t_config* config_master;
t_log* logger;

int id_query = 0;

int main(int argc, char* argv[]){
    
    diccionario_workers = dictionary_create();
    diccionario_exec = dictionary_create();
    diccionario_querys = dictionary_create();
    pthread_mutex_init(&mutex_diccionario, NULL);

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
    
    pthread_create(&hilo_main_escucha, NULL, funcion_main_escucha, socket_ptr);
    pthread_detach(hilo_main_escucha);

    pause();

    return 0;
}

//void cambiar_estado(query,nuevo estado) {
   // pcb->estado_actual = nuevo_estado;
   // pcb->metricas_estado[nuevo_estado]++;
//}