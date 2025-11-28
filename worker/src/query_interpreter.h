#ifndef QUERY_INTERPRETER_H_
#define QUERY_INTERPRETER_H_

#include <stdbool.h>
#include <utils/utils.h>
#include <worker.h>
#include <queries.h>
typedef enum {
    CREATE_Q,
    TRUNCATE_Q,
    WRITE_Q,
    READ_Q,
    TAG_Q,
    COMMIT_Q,
    FLUSH_Q,
    DELETE_Q,
    END_Q
} id_query_t;

typedef struct {
    int identificador;
    char* file_tag;
    char* param1;
    char* param2;
} query_t;

typedef struct
{
    int qid;
    char* archivo;
    int pc;
} t_args_query_interpreter;


int check_interrupt(int qid, int pc);
query_t* parsear_query(char* query_raw, char** instruccion);
query_t* leer_query();
int ejecutar_query(query_t* query, int qid);
int manejar_respuesta(int respuesta);
void ciclo_ejecucion(char* nombre_archivo, int pc, int qid);
void* iniciar_query_interpreter(void* args);
char** separar_file_tag(char* file_tag);

#endif