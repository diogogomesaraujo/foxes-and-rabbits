SIZES=("5x5" "10x10" "20x20" "100x100" "200x200")
THREADS=(1 2 4 8 16)
RUNS=10

> bench.csv

for size in "${SIZES[@]}"; do
    for threads in "${THREADS[@]}"; do
        echo "Running $size with $threads thread(s)..."

        if [ $threads -eq 1 ]; then
            make bench
        else
            make omp_bench N_THREADS=$threads
        fi

        for i in $(seq 1 $RUNS); do
            ./a.out "ecosystem_examples/input${size}" "ecosystem_examples/output${size}"
        done
    done
done

echo "Benchmarking complete. Results in bench.csv"
