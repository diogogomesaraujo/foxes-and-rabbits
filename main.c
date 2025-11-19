#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define STR_SIZE 100
#define STARTING_AGE 0

#define POSSIBLE_DIRECTIONS_LEN 4
#define POSSIBLE_DIRECTIONS {(Direction) {-1, 0},(Direction) {0, 1},(Direction) {1, 0},(Direction) {0, -1}} // UP, RIGHT, LEFT DOWN
#define NO_DIRECTION (Direction) {INT_MIN, INT_MIN}
#define IT_HAS_DIRECTION(d) NO_DIRECTION.x != d.x && NO_DIRECTION.y != d.y

typedef enum {
    Rabbit,
    Fox,
    Rock,
    None
} CellID;

typedef struct {
    CellID id;
    int age;
    int gens_without_food;
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
    int g;
    int r;
    int c;
    int n;
    Cell **m;
} Environment;

int input_file_to_env(char* file_path, Environment *env_buf);

void print_environment(Environment e);
Cell** allocate_empty_cell_matrix(int r, int c);

Direction selecting_adjacent_cells(Environment e, int x, int y, Direction* d);
Direction select_fox_direction(Environment e, int x, int y);
Direction select_rabbit_direction(Environment e, int x, int y);

Cell cell_from_id(CellID id);

int max(int a, int b) {
    if (a > b) return a; else return b;
}

Cell cell_from_id(CellID id) {
    return (Cell) { id, STARTING_AGE, STARTING_AGE };
}

Cell** allocate_empty_cell_matrix(int r, int c) {
    Cell **m = (Cell**) malloc(sizeof(Cell*) * r);
    for (int i = 0; i < r; i++) {
        m[i] = (Cell*) malloc(sizeof(Cell) * c);
        for (int j = 0; j < c; j++) {
            m[i][j] = cell_from_id(None);
        }
    }
    return m;
}

int destroy_cell_matrix(Cell **m, int r) {
    for (int i = 0; i < r; i++) {
        free(m[i]);
    }
    free(m);
    return 0;
}

int input_file_to_env(char* file_path, Environment *env_buf) {
    FILE* file = fopen(file_path, "r");

    if (file == NULL) {
        fprintf(stderr, "Couldn't open the file.\n");
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
        fprintf(stderr, "Couldn't parse the initial variables.\n");
        return 1;
    }

    (*env_buf).g = 0;
    (*env_buf).m = allocate_empty_cell_matrix((*env_buf).r, (*env_buf).c);
    char* line_temp = malloc(sizeof(int) * STR_SIZE);
    int x_temp, y_temp;

    while (fscanf(file, "%s %d %d", line_temp, &x_temp, &y_temp) != EOF) {
        if (strcmp(line_temp, "ROCK") == 0) (*env_buf).m[x_temp][y_temp] = cell_from_id(Rock);
        if (strcmp(line_temp, "RABBIT") == 0) (*env_buf).m[x_temp][y_temp] = cell_from_id(Rabbit);
        if (strcmp(line_temp, "FOX") == 0) (*env_buf).m[x_temp][y_temp] = cell_from_id(Fox);
    }

    return 0;
}

Direction selecting_adjacent_cells(Environment e, int x, int y, Direction d[POSSIBLE_DIRECTIONS_LEN]) {
    int d_len = 0;
    for (int i = 0; i < 4; i++) if (IT_HAS_DIRECTION(d[i])) d_len++;

    if (d_len == 0) return NO_DIRECTION;

    int choosing_value = (e.g + x + y) % d_len;

    int count = 0, cv_count = 0;
    while (count < POSSIBLE_DIRECTIONS_LEN) {
        if (cv_count == choosing_value && IT_HAS_DIRECTION(d[count])) break;
        if (IT_HAS_DIRECTION(d[count])) cv_count++;

        count++;
    }

    return d[count];
}

Direction select_rabbit_direction(Environment e, int x, int y) {
    assert(e.m[x][y].id == Rabbit);

    Direction d[POSSIBLE_DIRECTIONS_LEN] = POSSIBLE_DIRECTIONS;
    if (x == 0)       d[0] = NO_DIRECTION;
    if (y == e.c - 1) d[1] = NO_DIRECTION;
    if (x == e.r - 1) d[2] = NO_DIRECTION;
    if (y == 0)       d[3] = NO_DIRECTION;

    for (int i = 0; i < POSSIBLE_DIRECTIONS_LEN; i++)
        if (IT_HAS_DIRECTION(d[i]) && e.m[x + d[i].x][y + d[i].y].id != None) d[i] = NO_DIRECTION;

    return selecting_adjacent_cells(e, x, y, d);
}


Direction select_fox_direction(Environment e, int x, int y) {
    assert(e.m[x][y].id == Fox);

    // logic for rabbits
    Direction rabbit_dirs[POSSIBLE_DIRECTIONS_LEN] = POSSIBLE_DIRECTIONS;
    if (x == 0)       rabbit_dirs[0] = NO_DIRECTION;
    if (y == e.c - 1) rabbit_dirs[1] = NO_DIRECTION;
    if (x == e.r - 1) rabbit_dirs[2] = NO_DIRECTION;
    if (y == 0)       rabbit_dirs[3] = NO_DIRECTION;

    for (int i = 0; i < POSSIBLE_DIRECTIONS_LEN; i++)
        if (IT_HAS_DIRECTION(rabbit_dirs[i]) && e.m[x + rabbit_dirs[i].x][y + rabbit_dirs[i].y].id != Rabbit) rabbit_dirs[i] = NO_DIRECTION;

    Direction rd = selecting_adjacent_cells(e, x, y, rabbit_dirs);
    if (IT_HAS_DIRECTION(rd)) return rd;

    // logic for empty
    Direction empty_dirs[POSSIBLE_DIRECTIONS_LEN] = POSSIBLE_DIRECTIONS;
    if (x == 0)       empty_dirs[0] = NO_DIRECTION;
    if (y == e.c - 1) empty_dirs[1] = NO_DIRECTION;
    if (x == e.r - 1) empty_dirs[2] = NO_DIRECTION;
    if (y == 0)       empty_dirs[3] = NO_DIRECTION;

    for (int i = 0; i < POSSIBLE_DIRECTIONS_LEN; i++)
        if (IT_HAS_DIRECTION(empty_dirs[i]) && e.m[x + empty_dirs[i].x][y + empty_dirs[i].y].id != None) empty_dirs[i] = NO_DIRECTION;

    Direction ed = selecting_adjacent_cells(e, x, y, empty_dirs);
    return ed;
}

int next_gen(Environment *e_buf) {
    Direction d_temp;
    Cell **new_m = allocate_empty_cell_matrix((*e_buf).r, (*e_buf).c);

    for (int i = 0; i < (*e_buf).r; i++) {
        for (int j = 0; j < (*e_buf).c; j++) {
            if ((*e_buf).m[i][j].id == None || (*e_buf).m[i][j].id == Rock) new_m[i][j] = new_m[i][j];
            // rabbit logic
            if ((*e_buf).m[i][j].id == Rabbit) {
                d_temp = select_rabbit_direction(*e_buf, i, j);
                printf("x: %d, y: %d, d_x: %d, d_y: %d age: %d\n", i, j, d_temp.x, d_temp.y, (*e_buf).m[i][j].age);
                // check logic
                if (IT_HAS_DIRECTION(d_temp)) {
                    new_m[i + d_temp.x][j + d_temp.y] = (*e_buf).m[i][j];
                    if (new_m[i + d_temp.x][j + d_temp.y].id == Rabbit) {
                        new_m[i + d_temp.x][j + d_temp.y].age = max(new_m[i + d_temp.x][j + d_temp.y].age, (*e_buf).m[i + d_temp.x][j + d_temp.y].age);

                        if (new_m[i + d_temp.x][j + d_temp.y].age >= (*e_buf).gen_proc_rabbits) {
                            new_m[i][j] = cell_from_id(Rabbit);
                        }
                    }
                    new_m[i + d_temp.x][j + d_temp.y].age++;
                } else new_m[i][j].age++;
            }
            // fox logic
        }
    }

    destroy_cell_matrix((*e_buf).m, (*e_buf).r);
    (*e_buf).m = new_m;
    (*e_buf).g++;

    return 0;
}

void print_environment(Environment e) {
    printf("Gen: %d", e.g);
    for (int i = 0; i < e.r; i++) {
        printf("\n");
        for (int j = 0; j < e.c; j++) {
            switch (e.m[i][j].id) {
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
    printf("\n\n");
}

int main(int argc, char** argv) {
    Environment e;
    if (argc != 2) return 1;
    if (input_file_to_env(argv[1], &e) == 1) {
        return 1;
    }
    printf("Gen proc rabbits: %d\n", e.gen_proc_rabbits);
    print_environment(e);
    for (int i = 0; i < 5; i++) {
        next_gen(&e);
        print_environment(e);
    }
    return 0;
}
