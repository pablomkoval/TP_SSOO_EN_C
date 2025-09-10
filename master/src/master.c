#include <master.h>

pthread_t hilo_main_escucha;

int main(int argc, char* argv[]) {
    


    socket_escucha = iniciar_servidor(puerto_escucha);

    pthread_create(&hilo_main_escucha, NULL, funcion_main_escucha, NULL);
    pthread_detach(hilo_main_escucha);

    return 0;
}
