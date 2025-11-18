#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define STR_SIZE 100

#define POSSIBLE_DIRECTIONS_LEN 4
#define POSSIBLE_DIRECTIONS {(Direction) {-1, 0},(Direction) {0, 1},(Direction) {1, 0},(Direction) {0, -1},} // UP, RIGHT, LEFT DOWN
#define NO_DIRECTION (Direction) {INT_MIN, INT_MIN}
#define IT_HAS_DIRECTION(d) NO_DIRECTION.x != d.x && NO_DIRECTION.y != d.y

typedef enum {
    Rabbit,
    Fox,
    Rock,
    None
} Cell;

typedef struct {
    int x;
    int y;
} Direction;

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

int input_file_to_env(char* file_path, Environment *env_buf);

void print_environment(Environment e);
Cell** allocate_empty_cell_matrix(int r, int c);

Direction selecting_adjacent_cells(Environment e, int x, int y, Direction* d, int d_len);

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

void possible_adjacent_cells_rabbit(Environment e, int x, int y, Direction d_buf[POSSIBLE_DIRECTIONS_LEN]) {
    assert(e.m[x][y] == Rabbit);

    Direction d[POSSIBLE_DIRECTIONS_LEN] = POSSIBLE_DIRECTIONS;
    if (x == 0)       d[0] = NO_DIRECTION;
    if (y == e.c - 1) d[1] = NO_DIRECTION;
    if (x == e.r - 1) d[2] = NO_DIRECTION;
    if (y == 0)       d[3] = NO_DIRECTION;

    for (int i = 0; i < POSSIBLE_DIRECTIONS_LEN; i++) {
        if (IT_HAS_DIRECTION(d[i]) && e.m[x + d[i].x][y + d[i].y] != None) d[i] = NO_DIRECTION;
        d_buf[i] = d[i];
    }

    printf("\n");

    for (int i = 0; i < POSSIBLE_DIRECTIONS_LEN; i++) printf("x: %d, y: %d\n", d[i].x, d[i].y);
}

Direction selecting_adjacent_cells(Environment e, int x, int y, Direction* d, int d_len) {
    int choosing_value = (e.n_gen + x + y) % d_len;
    return d[choosing_value];
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
                    printf("_ ");
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
    Direction d[POSSIBLE_DIRECTIONS_LEN];
    possible_adjacent_cells_rabbit(e, 1, 5, d);
    return 0;
}
