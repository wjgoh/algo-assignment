# Build & Run Guide

How to compile and run every program in this CCP6214 assignment, and how to
generate all the datasets. All commands assume macOS/Linux with `g++` and are
run from the **repository root** unless noted.

> **Status:** Only `dataset_generator` exists today (`Dataset Generator/dataset_generator.cpp`).
> The other six programs are not yet written — their rows below document the
> required compile/run/output conventions from the spec so they match when added.

## Conventions (graded — keep exact)

- **Standard C++ only**, compiled per-file. No external build system.
- **No library sort/search** and no STL container that sorts/searches internally
  (`std::sort`, `std::set`, `std::map`, `std::binary_search`, etc.).
- Compile flags: `-O2 -std=c++17`.
- Output files are written to the **current working directory**, so `cd` into
  wherever you want the `.csv`/`.txt` files first.
- The 10-digit integer is the sort/search **key**; every output row keeps both
  fields: `integer,string` in CSV outputs, `integer/string` in step files.
- Timing must **exclude I/O** (don't measure file reads or printing).

---

## 1. `dataset_generator` (exists)

Compile:

```bash
g++ -O2 -std=c++17 "Dataset Generator/dataset_generator.cpp" -o dataset_generator
```

Run (writes `dataset_<n>.csv` to the current directory):

```bash
./dataset_generator <n>
```

Example:

```bash
./dataset_generator 1000          # -> dataset_1000.csv
```

### Generate the 10 required input sizes

Run each (large ones take a while and a lot of disk — see notes):

```bash
./dataset_generator 1000
./dataset_generator 10000
./dataset_generator 100000
./dataset_generator 500000
./dataset_generator 1000000
./dataset_generator 5000000
./dataset_generator 10000000
./dataset_generator 50000000
./dataset_generator 100000000
./dataset_generator 200000000
```

Or as a loop:

```bash
for n in 1000 10000 100000 500000 1000000 5000000 10000000 50000000 100000000 200000000; do
  ./dataset_generator "$n"
done
```

**Notes on the large sizes**

- Approx. disk per file: ~17 bytes/row → 1M ≈ 17 MB, 10M ≈ 170 MB,
  100M ≈ 1.7 GB, 200M ≈ 3.4 GB. Make sure you have free disk space.
- The generator holds the keys in RAM (`8 bytes × n`): 100M ≈ 0.8 GB,
  200M ≈ 1.6 GB. A low-memory machine may swap or fail to allocate at 200M.
- These files are far too big for the 99 MB zip — per the spec, link them via a
  OneDrive folder instead of submitting them.

---

## 2. Sorting & search programs (to be written)

Compile each the same way once the `.cpp` exists, e.g.:

```bash
g++ -O2 -std=c++17 radix_sort.cpp -o radix_sort
```

| Program | Run | Produces |
|---|---|---|
| `radix_sort` | `./radix_sort dataset_1000000.csv` | `radix_sorted_dataset_1000000.csv` + prints running time (LSD, rightmost digit first) |
| `heap_sort` | `./heap_sort dataset_1000000.csv` | `heap_sort_dataset_1000000.csv` + prints running time (maxheap) |
| `hash_table_search` | `./hash_table_search dataset_1000000.csv` | `hash_table_search_dataset_1000000.txt` with best/average/worst times (does `n` searches) |

Each accepts one or more CSV filenames. Capture a command-prompt screenshot of
the printed running time for every input size (graded).

## 3. Step programs (to be written)

These produce a `.txt` trace of each sorting/search step. Match the PDF sample
formats exactly.

| Program | Run | Produces |
|---|---|---|
| `radix_sort_step` | `./radix_sort_step dataset_1000.csv <start> <end>` | `dataset_1000_radix_sorted_step_<start>_<end>.txt` — `[...] original` then passes `d=10 … d=1` |
| `heap_sort_step` | `./heap_sort_step dataset_1000.csv <start> <end>` | `dataset_1000_heap_sorted_step_<start>_<end>.txt` — `[...] initial` then `i = 6 … i = 1` |
| `hash_table_search_step` | `./hash_table_search_step dataset_1000.csv` | `dataset_1000_hash_table_search_step_<target>.txt` — found: `2008864030 = 2008864030/rdiea`; not found: `-1 != 123456789` |

`<start>`/`<end>` are 0-based-or-1-based **row numbers** in the CSV (decide once
and stay consistent). Targets for the step search are specified inside the code
file per the spec.

---

## Quick demo sequence (matches the PDF "Demo" section)

```bash
# 1. generate
./dataset_generator 10000000

# 2-4. step traces on the small sample
./radix_sort_step dataset_1000.csv 1 7
./heap_sort_step  dataset_1000.csv 1 7
./hash_table_search_step dataset_1000.csv

# 5-7. full sort/search on 1,000,000
./radix_sort dataset_1000000.csv
./heap_sort  dataset_1000000.csv
./hash_table_search dataset_1000000.csv
```
