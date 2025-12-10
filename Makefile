CC = gcc
CFLAGS_BASE = -O3
CFLAGS_OMP = -fopenmp
LDFLAGS_OMP = -fopenmp

SRC = main.c
OUT = ecosystem

# Default N_THREADS value
N_THREADS ?= 4

all: omp

omp: $(SRC)
	$(CC) $(CFLAGS_BASE) $(CFLAGS_OMP) -DN_THREADS=$(N_THREADS) -o $(OUT) $(SRC) $(LDFLAGS_OMP)

omp_allgen: $(SRC)
	$(CC) -D_ALLGEN $(CFLAGS_BASE) $(CFLAGS_OMP) -DN_THREADS=$(N_THREADS) -o $(OUT) $(SRC) $(LDFLAGS_OMP)

omp_bench: $(SRC)
	$(CC) -D_BENCH $(CFLAGS_BASE) $(CFLAGS_OMP) -DN_THREADS=$(N_THREADS) -o $(OUT) $(SRC) $(LDFLAGS_OMP)

seq: $(SRC)
	$(CC) $(CFLAGS_BASE) -o $(OUT) $(SRC)

bench: $(SRC)
	$(CC) -D_BENCH $(CFLAGS_BASE) -o $(OUT) $(SRC)

allgen: $(SRC)
	$(CC) -D_ALLGEN $(CFLAGS_BASE) -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)

.PHONY: all omp seq allgen clean
