#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int pid;
    int tiempo_servicio; // tiempo total requerido (ms)
    int tiempo_llegada; // instante de llegada (ms desde t=0)
    int tiempo_restante; // decrementa en cada ejecuci´on
    int esta_bloqueado; // 0 o 1
    } Proceso;

typedef struct {
    Proceso *cola_listos;
    int procesos_en_cola_listos;

    Proceso *cola_bloqueados;
    int procesos_en_cola_bloqueados;

    pthread_mutex_t mutex_cola_listos;
    pthread_cond_t condicion_cola_listos;
    pthread_mutex_t mutex_cola_bloqueados;

    } Sistema; 

typedef struct {
    int num_procesador;
    int procesos_ejecutados; // asumo que si se ejecuta un mismo proceso 2 veces cuenta como 2

    int utilizacion;

    long tiempo_espera_promedio; // tiempo promedio de procesos en el que se ejecutaron por primera vez

    long tiempo_bloqueado_promedio;


} Procesador;

void *planificador_cola_bloqueados();

void *planificador(void*procesador);

int leer_archivo_de_procesos(char *nombre_archivo, Proceso *cola_procesos);

void inicializacion_sistema(Proceso *cola_procesos, int numero_procesos);

void sistema(int procesadores, float quantum, float probabilidad);