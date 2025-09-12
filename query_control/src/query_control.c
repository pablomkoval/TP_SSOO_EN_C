#include <query_control.h>

t_log* logger;
t_config* config;
t_paquete* paquete_query

int socket_master;

int main(int argc, char* argv[]) {
    if(argc < 4){
        printf("Faltaron argumentos para iniciar el query control");
        return EXIT_FAILURE;
    }
    char* archivo_config = argv[1];
    char* archivo_query = argv[2];
    int prioridad = atoi(argv[3]);

    config = iniciar_config(archivo_config);
    logger = iniciar_logger();
    
    
    paquete_query = empaquetar_query(archivo_query, prioridad);

    socket_master = conectar_master(paquete_query);
    
    

    return 0;
}
