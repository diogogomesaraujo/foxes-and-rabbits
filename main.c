#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_SIZE 100
#define STARTING_AGE 0
#define STARTING_GENS_WITHOUT_FOOD 0
#define STARTING_GEN 0

#define POSSIBLE_DIRECTIONS_LEN 4
#define POSSIBLE_DIRECTIONS                                                    \
    {(Direction){-1, 0}, (Direction){0, 1}, (Direction){1, 0},                 \
     (Direction){0, -1}} // UP, RIGHT, LEFT, DOWN
#define NO_DIRECTION                                                           \
    (Direction) { INT_MIN, INT_MIN }
#define IT_HAS_DIRECTION(d) NO_DIRECTION.x != d.x &&NO_DIRECTION.y != d.y

typedef enum { Rabbit, Fox, Rock, None } CellID;

typedef struct {
    CellID id;
    int age;
    int gens_without_food;
    int gen_updated;
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
    int g;    // current gen
    int r;    // rows
    int c;    // cols
    int n;    // starting num of enteties
    Cell **m; // cell matrix
} Environment;

int input_file_to_env(char *file_path, Environment *env_buf);
void print_environment(Environment e, bool is_output);
void print_matrix(Cell **m, int r, int c);
Cell **allocate_empty_cell_matrix(int r, int c);
int assert_environment_equals(Environment e1, Environment e2);

Cell cell_from_id(CellID id, int gen);

bool cell_equals(Cell c1, Cell c2);



Direction selecting_adjacent_cells(Environment e, int x, int y, Direction *d);
Direction select_fox_direction(Environment e, int x, int y);
Direction select_rabbit_direction(Environment e, int x, int y);

int assert_environment_equals(Environment e1, Environment e2){
    if (e1.gen_food_foxes != e2.gen_food_foxes ||
        e1.gen_proc_foxes != e2.gen_proc_foxes ||
        e1.gen_proc_rabbits != e2.gen_proc_rabbits ||
        e1.c != e2.c ||
        e1.r != e2.r ||
        e1.n != e2.n){
            fprintf(stderr, "Assertion failed\n");
            fprintf(stderr, "e1: gpr=%d gpf=%d gff=%d r=%d c=%d n=%d\n",
                    e1.gen_proc_rabbits, e1.gen_proc_foxes, e1.gen_food_foxes,
                    e1.r, e1.c, e1.n);
            fprintf(stderr, "e2: gpr=%d gpf=%d gff=%d r=%d c=%d n=%d\n",
                    e2.gen_proc_rabbits, e2.gen_proc_foxes, e2.gen_food_foxes,
                    e2.r, e2.c, e2.n);
            return 1;
    }

    for (int r = 0; r<e1.r; r++) {
        for (int c = 0; c<e1.c; c++) {
            if (e1.m[r][c].id != e2.m[r][c].id){
                fprintf(stderr, "Assertion failed at [%d][%d] -> environment matrices didn't match\n", r, c);
                return 1;
            }
        }
    }
    printf("Assertion passed -> input matches output\n");
    return 0;
}

Cell cell_from_id(CellID id, int gen) {
    return (Cell){id, STARTING_AGE, STARTING_GENS_WITHOUT_FOOD, gen};
}

bool cell_equals(Cell c1, Cell c2){
    return (c1.age == c2.age && c1.id == c2.id && c1.gens_without_food == c2.gens_without_food);
}

Cell **allocate_empty_cell_matrix(int r, int c) {
    Cell **m = (Cell **)malloc(sizeof(Cell *) * r);
    for (int i = 0; i < r; i++) {
        m[i] = (Cell *)malloc(sizeof(Cell) * c);
        for (int j = 0; j < c; j++) {
            m[i][j] = cell_from_id(None, STARTING_GEN);
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

int copy_cell_matrix(Cell **m, Cell **m_buf, int r, int c) {
    for (int i = 0; i < r; i++)
        memcpy(m_buf[i], m[i], sizeof(Cell) * c);
    return 0;
}

int env_destroy(Environment e) {
    if (e.m != NULL) {
        destroy_cell_matrix(e.m, e.r);
    }
    return 0;
}

int input_file_to_env(char *file_path, Environment *env_buf) {
    FILE *file = fopen(file_path, "r");

    if (file == NULL) {
        fprintf(stderr, "Couldn't open the file.\n");
        return 1;
    }

    if (fscanf(file, "%d %d %d %d %d %d %d", &(*env_buf).gen_proc_rabbits,
               &(*env_buf).gen_proc_foxes, &(*env_buf).gen_food_foxes,
               &(*env_buf).n_gen, &(*env_buf).r, &(*env_buf).c,
               &(*env_buf).n) == EOF) {
        fprintf(stderr, "Couldn't parse the initial variables.\n");
        return 1;
    }

    (*env_buf).g = 0;
    (*env_buf).m = allocate_empty_cell_matrix((*env_buf).r, (*env_buf).c);
    char *line_temp = malloc(sizeof(char) * STR_SIZE);
    int x_temp, y_temp;

    while (fscanf(file, "%s %d %d", line_temp, &x_temp, &y_temp) != EOF) {
        if (strcmp(line_temp, "ROCK") == 0)
            (*env_buf).m[x_temp][y_temp] = cell_from_id(Rock, STARTING_GEN);
        if (strcmp(line_temp, "RABBIT") == 0)
            (*env_buf).m[x_temp][y_temp] = cell_from_id(Rabbit, STARTING_GEN);
        if (strcmp(line_temp, "FOX") == 0)
            (*env_buf).m[x_temp][y_temp] = cell_from_id(Fox, STARTING_GEN);
    }

    fclose(file);
    return 0;
}

Direction selecting_adjacent_cells(Environment e, int x, int y,
                                   Direction d[POSSIBLE_DIRECTIONS_LEN]) {
    int d_len = 0;
    for (int i = 0; i < 4; i++)
        if (IT_HAS_DIRECTION(d[i]))
            d_len++;

    if (d_len == 0)
        return NO_DIRECTION;

    int choosing_value = (e.g + x + y) % d_len;

    int count = 0, cv_count = 0;
    while (count < POSSIBLE_DIRECTIONS_LEN) {
        if (cv_count == choosing_value && IT_HAS_DIRECTION(d[count]))
            break;
        if (IT_HAS_DIRECTION(d[count]))
            cv_count++;

        count++;
    }

    return d[count];
}

Direction select_rabbit_direction(Environment e, int x, int y) {
    assert(e.m[x][y].id == Rabbit);

    Direction d[POSSIBLE_DIRECTIONS_LEN] = POSSIBLE_DIRECTIONS;
    if (x == 0)
        d[0] = NO_DIRECTION;
    if (y == e.c - 1)
        d[1] = NO_DIRECTION;
    if (x == e.r - 1)
        d[2] = NO_DIRECTION;
    if (y == 0)
        d[3] = NO_DIRECTION;

    for (int i = 0; i < POSSIBLE_DIRECTIONS_LEN; i++)
        if (IT_HAS_DIRECTION(d[i]) && e.m[x + d[i].x][y + d[i].y].id != None)
            d[i] = NO_DIRECTION;

    return selecting_adjacent_cells(e, x, y, d);
}

Direction select_fox_direction(Environment e, int x, int y) {
    assert(e.m[x][y].id == Fox);

    // logic for rabbits
    Direction rabbit_dirs[POSSIBLE_DIRECTIONS_LEN] = POSSIBLE_DIRECTIONS;
    if (x == 0)
        rabbit_dirs[0] = NO_DIRECTION;
    if (y == e.c - 1)
        rabbit_dirs[1] = NO_DIRECTION;
    if (x == e.r - 1)
        rabbit_dirs[2] = NO_DIRECTION;
    if (y == 0)
        rabbit_dirs[3] = NO_DIRECTION;

    for (int i = 0; i < POSSIBLE_DIRECTIONS_LEN; i++)
        if (IT_HAS_DIRECTION(rabbit_dirs[i]) &&
            e.m[x + rabbit_dirs[i].x][y + rabbit_dirs[i].y].id != Rabbit)
            rabbit_dirs[i] = NO_DIRECTION;

    Direction rd = selecting_adjacent_cells(e, x, y, rabbit_dirs);
    if (IT_HAS_DIRECTION(rd))
        return rd;

    // logic for empty
    Direction empty_dirs[POSSIBLE_DIRECTIONS_LEN] = POSSIBLE_DIRECTIONS;
    if (x == 0)
        empty_dirs[0] = NO_DIRECTION;
    if (y == e.c - 1)
        empty_dirs[1] = NO_DIRECTION;
    if (x == e.r - 1)
        empty_dirs[2] = NO_DIRECTION;
    if (y == 0)
        empty_dirs[3] = NO_DIRECTION;

    for (int i = 0; i < POSSIBLE_DIRECTIONS_LEN; i++)
        if (IT_HAS_DIRECTION(empty_dirs[i]) &&
            e.m[x + empty_dirs[i].x][y + empty_dirs[i].y].id != None)
            empty_dirs[i] = NO_DIRECTION;

    Direction ed = selecting_adjacent_cells(e, x, y, empty_dirs);
    return ed;
}

int single_rabbit_move(Environment e, Cell **copy, int x, int y) {
    Direction d = select_rabbit_direction(e, x, y);

    bool can_procreate = (e.m[x][y].age >= e.gen_proc_rabbits) && IT_HAS_DIRECTION(d);

    if (IT_HAS_DIRECTION(d)) {
        int dest_x = x + d.x;
        int dest_y = y + d.y;

        bool has_conflict = (copy[dest_x][dest_y].id == Rabbit);

        bool wins = has_conflict == false;

        if (has_conflict) {
            int other_age = copy[dest_x][dest_y].age - 1;
            wins = e.m[x][y].age > other_age;
            if (can_procreate) wins = false;
        }

        if (wins) {
            copy[dest_x][dest_y] = e.m[x][y];

            copy[dest_x][dest_y].age++;
            copy[dest_x][dest_y].gen_updated = e.g;
        }

        if (cell_equals(copy[x][y], e.m[x][y])) {
            if (can_procreate) {
                copy[x][y] = cell_from_id(Rabbit, e.g);
                if (!has_conflict) copy[dest_x][dest_y].age = 0;
            } else {
                copy[x][y] = cell_from_id(None, e.g);
            }
        }
    }
    else {
        copy[x][y].age++;
        copy[x][y].gen_updated = e.g;
    }

    int dest_x = x + d.x;
    int dest_y = y + d.y;


    return 0;
}

//updates copy matrix with specific cell move [x][y] --> uses e for refference
//
// die and stay in place are handled
//
// only update age and food when explicit move so we dont crank up the values on an early winner on a conflicting cell
int single_fox_move(Environment e, Cell **copy, int x, int y) {
    Direction d = select_fox_direction(e, x, y);

    bool should_die = (e.m[x][y].gens_without_food >= e.gen_food_foxes - 1) &&
                      (IT_HAS_DIRECTION(d) == false ||
                       (IT_HAS_DIRECTION(d) && e.m[x+d.x][y+d.y].id != Rabbit));

    bool can_procreate = (e.m[x][y].age >= e.gen_proc_foxes) && IT_HAS_DIRECTION(d);

    if (should_die) {
        if (cell_equals(e.m[x][y], copy[x][y])) {
            copy[x][y] = cell_from_id(None, e.g);
        }
        return 0;
    }

    if (IT_HAS_DIRECTION(d)) {
        int dest_x = x + d.x;
        int dest_y = y + d.y;

        bool is_eating = (e.m[dest_x][dest_y].id == Rabbit);
        bool has_conflict = (copy[dest_x][dest_y].id == Fox);

        bool wins = !has_conflict;

        bool special_conflict = false;

        if (has_conflict) {
            int other_age = copy[dest_x][dest_y].age - 1;
            int other_hunger = copy[dest_x][dest_y].gens_without_food - (is_eating ? 0 : 1);

            wins = (e.m[x][y].age > other_age) ||
                   (e.m[x][y].age == other_age &&
                    e.m[x][y].gens_without_food < other_hunger);
            if (can_procreate) {
                wins = (0 == other_age && e.m[x][y].gens_without_food < other_hunger);
                special_conflict = true;
            }
        }

        if (wins) {
            copy[dest_x][dest_y] = e.m[x][y];
            copy[dest_x][dest_y].age++;
            if (special_conflict == false) copy[dest_x][dest_y].gens_without_food = is_eating ? 0 : (e.m[x][y].gens_without_food + 1);
            copy[dest_x][dest_y].gen_updated = e.g;
        }

        if (cell_equals(copy[x][y], e.m[x][y])) {
            if (can_procreate) {
                copy[x][y] = cell_from_id(Fox, e.g);
                if (!has_conflict) copy[dest_x][dest_y].age = 0;
            } else {
                copy[x][y] = cell_from_id(None, e.g);
            }
        }
    }
    else {
        if (!cell_equals(e.m[x][y], copy[x][y])) {
            if (e.m[x][y].age > copy[x][y].age - 1 ||
                (e.m[x][y].age == copy[x][y].age - 1 &&
                 e.m[x][y].gens_without_food < copy[x][y].gens_without_food - 1)) {
                bool eaten_rabbit = (copy[x][y].gens_without_food == 0);
                copy[x][y] = e.m[x][y];
                copy[x][y].gens_without_food++;
                if (eaten_rabbit) {
                    copy[x][y].gens_without_food = 0;
                }
                copy[x][y].age++;
                copy[x][y].gen_updated = e.g;
            }
        }
        else {
            copy[x][y].gens_without_food++;
            copy[x][y].age++;
            copy[x][y].gen_updated = e.g;
        }
    }

    return 0;
}

int next_gen(Environment *e_buf) {
    Cell **new_m = allocate_empty_cell_matrix((*e_buf).r, (*e_buf).c);
    copy_cell_matrix((*e_buf).m, new_m, (*e_buf).r, (*e_buf).c);

    // rabbit
    for (int i = 0; i < (*e_buf).r; i++) {
        for (int j = 0; j < (*e_buf).c; j++) {
            if ((*e_buf).m[i][j].id == Rabbit) {
                single_rabbit_move((*e_buf), new_m, i, j);
            }
        }
    }

    copy_cell_matrix(new_m, (*e_buf).m, (*e_buf).r, (*e_buf).c);

    // fox
    for (int i = 0; i < (*e_buf).r; i++) {
        for (int j = 0; j < (*e_buf).c; j++) {
            if ((*e_buf).m[i][j].id == Fox) {
                single_fox_move((*e_buf), new_m, i, j);
            }
        }
    }

    destroy_cell_matrix((*e_buf).m, (*e_buf).r);
    (*e_buf).m = new_m;

    (*e_buf).g++;
    return 0;
}

int update_n(Environment *e_buf) {
    (*e_buf).n = 0;
    for (int i = 0; i < (*e_buf).r; i++) {
        for (int j = 0; j < (*e_buf).c; j++) {
            if ((*e_buf).m[i][j].id != None) (*e_buf).n++;
        }
    }
    return 0;
}

void print_environment(Environment e, bool is_output) {
    if (is_output) printf("Correct Output\n\n");

    else printf("Generation %d\n\n", e.g);

    for (int i = 0; i < e.r; i++) {
        for (int j = 0; j < e.c; j++) {
            switch (e.m[i][j].id) {
            case Rabbit:
                printf("R");
                break;
            case Rock:
                printf("*");
                break;
            case Fox:
                printf("F");
                break;
            default:
                printf(" ");
                break;
            }
        }
        printf("  ");

        for (int j = 0; j < e.c; j++) {
            switch (e.m[i][j].id) {
            case Rabbit:
                printf("%d", e.m[i][j].age);
                break;
            case Rock:
                printf("*");
                break;
            case Fox:
                printf("%d", e.m[i][j].age);
                break;
            default:
                printf(" ");
                break;
            }
        }
        printf("  ");

        for (int j = 0; j < e.c; j++) {
            switch (e.m[i][j].id) {
            case Rabbit:
                printf("R");
                break;
            case Rock:
                printf("*");
                break;
            case Fox:
                printf("%d", e.m[i][j].gens_without_food);
                break;
            default:
                printf(" ");
                break;
            }
        }
        printf("\n");
    }

    for (int i = 0; i < 3 * e.r + 4; i++) printf("_");
    printf("\n\n");
}

int main(int argc, char **argv) {
    Environment e;
    if (argc != 3)
        return 1;

    if (input_file_to_env(argv[1], &e) == 1) {
        return 1;
    }

    print_environment(e, false);
    for (int i = 0; i < e.n_gen; i++) {
        next_gen(&e);
    }
    update_n(&e);

    print_environment(e, false);

    Environment out;
    if (input_file_to_env(argv[2], &out) == 1) {
        return 1;
    }
    print_environment(out, true);

    assert_environment_equals(e,out);

    return 0;
}
