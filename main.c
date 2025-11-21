#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_SIZE 100
#define STARTING_AGE 0

#define POSSIBLE_DIRECTIONS_LEN 4
#define POSSIBLE_DIRECTIONS                                                    \
  {(Direction){-1, 0}, (Direction){0, 1}, (Direction){1, 0},                   \
   (Direction){0, -1}} // UP, RIGHT, LEFT DOWN
#define NO_DIRECTION                                                           \
  (Direction) { INT_MIN, INT_MIN }
#define IT_HAS_DIRECTION(d) NO_DIRECTION.x != d.x &&NO_DIRECTION.y != d.y

typedef enum { Rabbit, Fox, Rock, None } CellID;

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
  int g;    // current gen
  int r;    // rows
  int c;    // cols
  int n;    // starting num of enteties
  Cell **m; // cell matrix
} Environment;

int input_file_to_env(char *file_path, Environment *env_buf);

void print_environment(Environment e);
Cell **allocate_empty_cell_matrix(int r, int c);

Direction selecting_adjacent_cells(Environment e, int x, int y, Direction *d);
Direction select_fox_direction(Environment e, int x, int y);
Direction select_rabbit_direction(Environment e, int x, int y);

Cell cell_from_id(CellID id);

int max(int a, int b) {
  if (a > b)
    return a;
  else
    return b;
}

Cell cell_from_id(CellID id) { return (Cell){id, STARTING_AGE, STARTING_AGE}; }

Cell **allocate_empty_cell_matrix(int r, int c) {
  Cell **m = (Cell **)malloc(sizeof(Cell *) * r);
  for (int i = 0; i < r; i++) {
    m[i] = (Cell *)malloc(sizeof(Cell) * c);
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
  char *line_temp = malloc(sizeof(int) * STR_SIZE);
  int x_temp, y_temp;

  while (fscanf(file, "%s %d %d", line_temp, &x_temp, &y_temp) != EOF) {
    if (strcmp(line_temp, "ROCK") == 0)
      (*env_buf).m[x_temp][y_temp] = cell_from_id(Rock);
    if (strcmp(line_temp, "RABBIT") == 0)
      (*env_buf).m[x_temp][y_temp] = cell_from_id(Rabbit);
    if (strcmp(line_temp, "FOX") == 0)
      (*env_buf).m[x_temp][y_temp] = cell_from_id(Fox);
  }

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

int single_fox_move(Environment e, Cell **copy, int x, int y) {
  Direction d = select_fox_direction(e, x, y);
  if (IT_HAS_DIRECTION(d)) {
    switch (copy[x + d.x][y + d.y].id) {

    case Rabbit:
      // eat --> replace and loose hunger
      copy[x + d.x][y + d.y] = e.m[x][y];
      copy[x + d.x][y + d.y].gens_without_food = 0;

      break;

    case Fox:
      // if has greater proc age or same proc age and has less --> replace
      if (e.m[x][y].age > copy[x + d.x][y + d.y].age ||
          (e.m[x][y].age == copy[x + d.x][y + d.y].age &&
           e.m[x][y].gens_without_food <
               copy[x + d.x][y + d.y].gens_without_food)) {
        copy[x + d.x][y + d.y] = e.m[x][y]; // replace
      }
      // else --> keep the one already occupying the Cell (the one that is
      // moving disappears)

      copy[x + d.x][y + d.y]
          .gens_without_food++; // increase gens_without_food (hunger)
      break;

    case None:
      // if about to die --> die and don't move
      if (e.m[x][y].age >= e.gen_food_foxes) {
        return 0;
      }

      // else --> take position and increase (hunger)
      else {
        copy[x + d.x][y + d.y] = e.m[x][y];
        copy[x + d.x][y + d.y].gens_without_food++;
      }
      break;

    default:
      printf("single_fox_move entered unexpected case \n");
      return 1;
    }

    // copy[x + d.x][y + d.y] is already correctly updated at this step
    // and erase always prev pos

    copy[x][y] = cell_from_id(None);

    // if can procriate --> leave fox in place --> both procriation ages go to 0
    if (e.m[x][y].age >= e.gen_proc_foxes) {
      copy[x + d.x][y + d.y].age = 0;
      copy[x][y] = cell_from_id(Fox);
    }

    copy[x + d.x][y + d.y].age++; // increase age --> update copy matrix
  }

  // if does not have direction
  else {
    // age or die logic for fox that stayed in-place
    if (e.m[x][y].gens_without_food >= e.gen_food_foxes) {
      copy[x][y] = cell_from_id(None); // die
    } else {
      copy[x][y] = e.m[x][y];
      copy[x][y].age++; // increase age --> update copy matrix
    }
  }

  return 0;
}

int next_gen(Environment *e_buf) {
  // Direction d_temp;
  Cell **new_m = allocate_empty_cell_matrix((*e_buf).r, (*e_buf).c);
  // use it_has_dir before moving
  // fox

  // rabit

  destroy_cell_matrix((*e_buf).m, (*e_buf).r);
  (*e_buf).m = new_m;
  (*e_buf).g++;

  // check fox died against original matrix pos (if rabbit --> survive, else -->
  // die)

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

int main(int argc, char **argv) {
  Environment e;
  if (argc != 2)
    return 1;
  if (input_file_to_env(argv[1], &e) == 1) {
    return 1;
  }
  print_environment(e);
  return 0;
}
