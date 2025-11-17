#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define STR_SIZE 100

typedef enum {
    Rabbit,
    Fox,
    Rock,
    None
} Cell;

typedef struct {
    int gen_proc_rabbits;
    int gen_proc_foxes;
    int gen_food_foxes;
    int n_gen;
    int r;
    int c;
    int n;
    Cell **m;
} Environment;

Cell** allocate_empty_cell_matrix(int r, int c) {
    Cell **m = (Cell**) malloc(sizeof(Cell*) * r);
    for (int i = 0; i < r; i++) {
        m[i] = (Cell*) malloc(sizeof(Cell) * c);
        for (int j = 0; j < c; j++) {
            m[i][j] = None;
        }
    }
    return m;
}

int input_file_to_env(char* file_path, Environment *env_buf) {
    FILE* file = fopen(file_path, "r");

    if (file == NULL) {
        fprintf(stderr, "Couldn't open the file.");
        return 1;
    }

    if (fscanf(file,
        "%d %d %d %d %d %d %d",
        &(*env_buf).gen_proc_rabbits,
        &(*env_buf).gen_proc_foxes,
        &(*env_buf).gen_food_foxes,
        &(*env_buf).n_gen,
        &(*env_buf).r,
        &(*env_buf).c,
        &(*env_buf).n
    ) == EOF) {
        fprintf(stderr, "Couldn't parse the initial variables.");
        return 1;
    }

    (*env_buf).m = allocate_empty_cell_matrix((*env_buf).r, (*env_buf).c);
    char* line_temp = malloc(sizeof(int) * STR_SIZE);
    int x_temp, y_temp;

    while (fscanf(file, "%s %d %d", line_temp, &x_temp, &y_temp) != EOF) {
        if (strcmp(line_temp, "ROCK") == 0) (*env_buf).m[x_temp][y_temp] = Rock;
        if (strcmp(line_temp, "RABBIT") == 0) (*env_buf).m[x_temp][y_temp] = Rabbit;
        if (strcmp(line_temp, "FOX") == 0) (*env_buf).m[x_temp][y_temp] = Fox;
    }

    return 0;
}

void print_environment(Environment e) {
    for (int i = 0; i < e.r; i++) {
        printf("\n");
        for (int j = 0; j < e.c; j++) {
            switch (e.m[i][j]) {
                case Rabbit:
                    printf("R ");
                    break;
                case Rock:
                    printf("* ");
                    break;
                case Fox:
                    printf("F ");
                    break;
                default:
                    printf("  ");
                    break;
            }
        }
    }
    printf("\n");
}

int main(int argc, char** argv) {
    Environment e;
    if (argc != 2) return 1;
    input_file_to_env(argv[1], &e);
    print_environment(e);
    return 0;
}
