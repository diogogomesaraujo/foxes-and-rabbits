make

make clean

./a.out ecosystem_examples/input200x200 ecosystem_examples/output200x200
./a.out ecosystem_examples/input100x100 ecosystem_examples/output100x100
./a.out ecosystem_examples/input20x20 ecosystem_examples/output20x20
./a.out ecosystem_examples/input10x10 ecosystem_examples/output10x10
./a.out ecosystem_examples/input5x5 ecosystem_examples/output5x5

sed -i 's/CC = clang/CC = gcc/' Makefile && sed -i 's/CFLAGS_OMP = -Xpreprocessor -fopenmp -I\/opt\/homebrew\/opt\/libomp\/include/CFLAGS_OMP = -fopenmp/' Makefile && sed -i 's/LDFLAGS_OMP = -L\/opt\/homebrew\/opt\/libomp\/lib -lomp/LDFLAGS_OMP = -fopenmp -lm/' Makefile
