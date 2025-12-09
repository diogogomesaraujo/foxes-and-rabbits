import pandas as pd

bench_data = pd.read_csv(
    "bench.csv", header=None, names=["m_size", "n_threads", "exec_time"]
)

mean_dict = {}

for m_size, n_thread, exec_time in bench_data.values:
    vals = mean_dict.get((m_size, n_thread))
    if vals != None:
        (total, count) = vals
        mean_dict[(m_size, n_thread)] = (total + exec_time, count + 1)
    else:
        mean_dict[(m_size, n_thread)] = (exec_time, 1)

for key in mean_dict:
    total, count = mean_dict[key]
    mean_dict[key] = total / count

m_sizes = sorted(set(m for m, _ in mean_dict.keys()))
n_threads = sorted(set(n for _, n in mean_dict.keys()))

speedup_data = []
for m_size in m_sizes:
    t1 = mean_dict.get((m_size, 1), None)
    if t1:
        row = {"matrix_size/number_of_threads": m_size}
        for n_thread in n_threads:
            tn = mean_dict.get((m_size, n_thread), None)
            if tn:
                row[f"{n_thread}"] = t1 / tn
        speedup_data.append(row)

speedup_df = pd.DataFrame(speedup_data)
speedup_df.to_csv("speedup.csv", index=False, float_format="%.10f")

efficiency_data = []
for m_size in m_sizes:
    t1 = mean_dict.get((m_size, 1), None)
    if t1:
        row = {"matrix_size/number_of_threads": m_size}
        for n_thread in n_threads:
            tn = mean_dict.get((m_size, n_thread), None)
            if tn:
                row[f"{n_thread}"] = (t1 / tn) / n_thread
        efficiency_data.append(row)

efficiency_df = pd.DataFrame(efficiency_data)
efficiency_df.to_csv("efficiency.csv", index=False, float_format="%.10f")

mean_data = []
for m_size in m_sizes:
    row = {"matrix_size/number_of_threads": m_size}
    for n_thread in n_threads:
        tn = mean_dict.get((m_size, n_thread), None)
        if tn:
            row[f"{n_thread}"] = tn
    mean_data.append(row)

mean_df = pd.DataFrame(mean_data)
mean_df.to_csv("execution_times.csv", index=False, float_format="%.10f")
