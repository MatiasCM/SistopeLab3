#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funciones.h"

int main(int argc, char *argv[]) {
    char *input = NULL;
    int numproc;
    float quantum;
    float prob;
    
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "--data")){
            input = argv[i+1];
        }
        else if(!strcmp(argv[i], "--numproc")){
            numproc = atoi(argv[i+1]);
        }
        else if(!strcmp(argv[i], "--quantum")){
            quantum = atof(argv[i+1]);
        }
        else if(!strcmp(argv[i], "--prob")){
            prob = atof(argv[i+1]);
        }
    }

    Proceso *cola_procesos = leer_archivo_de_procesos(input);

    inicializacion_sistema();

    sistema(numproc, quantum, prob);

    return 0;

}