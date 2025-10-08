#include <queries.h>

void ejecutar_create(char* nombre_file, char* tag){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, CREATE);

    agregar_a_paquete(paquete, nombre_file, strlen(nombre_file));
    agregar_a_paquete(paquete, tag, strlen(tag));

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
}

void ejecutar_trucate(char* nombre_file, char* tag, int tamanio){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, TRUNCATE);
    
    agregar_a_paquete(paquete, nombre_file, strlen(nombre_file));
    agregar_a_paquete(paquete, tag, strlen(tag));
    agregar_a_paquete(paquete, &tamanio, sizeof(int));

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
}

void ejecutar_write(char* nombre_file, char* tag, int direccion_base, char* contenido){

}


void ejecutar_read(char* nombre_file, char* tag, int direccion_base, int tamanio){
    
}

void ejecutar_tag(char* file_origen, char* tag_origen, char* file_destino, char* tag_destino){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, TAG);

    agregar_a_paquete(paquete, file_origen, strlen(file_origen));
    agregar_a_paquete(paquete, tag_origen, strlen(tag_origen));
    agregar_a_paquete(paquete, file_destino, strlen(file_destino));
    agregar_a_paquete(paquete, tag_destino, strlen(tag_destino));

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
}

void ejecutar_commit(char* nombre_file, char* tag){

    ejecutar_flush(nombre_file, tag);

    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, COMMIT);
    
    agregar_a_paquete(paquete, nombre_file, strlen(nombre_file));
    agregar_a_paquete(paquete, tag, strlen(tag));

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
}

void ejecutar_flush(char* nombre_file, char* tag){
    //falta agregarlo antes de realizar el desalojo del query del worker

    //bajar file:tag de memoria interna a storage
}

void ejecutar_delete(char* nombre_file, char* tag){
    t_paquete* paquete = crear_paquete();
    cambiar_opcode_paquete(paquete, DELETE);

    agregar_a_paquete(paquete, nombre_file, strlen(nombre_file));
    agregar_a_paquete(paquete, tag, strlen(tag));

    enviar_paquete(paquete, socket_storage, logger);
    borrar_paquete(paquete);
}

void ejecutar_end(){
    //finalizar query
    //provocar interrupcion(mediante una variable quizas(?))
    enviar_cod_op(socket_master, END);
}