CC = clang
CFLAGS_BASE = -O3
CFLAGS_OMP = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include
LDFLAGS_OMP = -L/opt/homebrew/opt/libomp/lib -lomp
SRC = main.c
OUT = a.out

all: omp

omp: $(SRC)
	$(CC) $(CFLAGS_BASE) $(CFLAGS_OMP) -o $(OUT) $(SRC) $(LDFLAGS_OMP)

seq: $(SRC)
	$(CC) $(CFLAGS_BASE) -o $(OUT) $(SRC)

allgen: $(SRC)
	$(CC) -D_ALLGEN $(CFLAGS_BASE) -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)

.PHONY: all omp seq clean
