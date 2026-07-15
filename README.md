# ML-PGO: Multi-Level Profile-Guided Optimization

This repository contains scripts, tools, and configuration files for reproducing the multi-level Profile-Guided Optimization (PGO) experiments described in:

> **"Beyond Single-Level PGO: Understanding and Harnessing Cross-Level Optimization Effects in Industrial Systems"**

---

## Overview

Profile-Guided Optimization (PGO) is widely used to improve software performance in both applications and operating system kernels. However, prior work typically applies PGO to a single layer in isolation, overlooking cross-layer interactions in real-world systems where kernel and application are often optimized independently using different workloads.

This paper presents a large-scale industrial study of **multi-level PGO** across three representative products:

| Product | Kernel Versions | Compiler Versions | Hardware |
|---|---|---|---|
| **Database Server** (PostgreSQL) | 3.10, 4.18, 4.19 | GCC 6.2.0, 8.3.1 | x86_64 Intel Xeon |
| **Web Server** (nginx + DPDK) | 4.19 | GCC 6.2.0 | x86_64 Intel Xeon |
| **Embedded System** | 3.10 | GCC 6.2.0 | ARM64 |

For each product, four optimization configurations are compared:

1. **Baseline** — standard compiler optimizations (`-O2`), no PGO
2. **Kernel-only PGO** — kernel PGO-optimized, application at baseline
3. **Application-only PGO** — application PGO-optimized, kernel at baseline
4. **Independent combination** — both layers independently PGO-optimized

The paper shows that multi-level PGO can yield gains up to 17.04%, but also introduces severe regressions of up to 81.90% under certain conditions, driven by cross-layer profile mismatch and microarchitectural contention.

---

## Repository Structure

```
Release/
├── README.md                         ← This file
├── scripts/
│   ├── gather.sh                     ← Collect gcda profiles from /sys/kernel/debug/gcov
│   ├── process.sh                    ← Extract, validate checksums, rename gcda files
│   └── rename.sh                     ← Rename gcda to avoid stale data pollution
├── tools/
│   └── calcsum.cc                    ← GCOV profile checksum validation and repair tool
└── benchmarks/
    ├── create_table.sql              ← Database schema for PostgreSQL benchmarks
    ├── postgresql_benchmark.conf     ← PostgreSQL server configuration
    └── workload_description.md       ← Description of all evaluation workloads
```

---

## Prerequisites

### Kernel & Compiler

Three kernel versions were modified to support kernel-level PGO, along with matching GCC versions:

| Kernel | GCC | Notes |
|---|---|---|
| 3.10 | 6.2.0 | Includes embedded system product |
| 4.18 | 8.3.1 | GCOV format version 5 (GCC 5+) |
| 4.19 | 6.2.0 | |

Kernel modification follows the approach described in Yuan et al. [1,2,3] and Bearman [4].

**[1]** Yuan, P., Guo, Y., & Chen, X. (2014). "Experiences in profile-guided operating system kernel optimization." *Proc. 5th Asia-Pacific Workshop on Systems (APSys)*. DOI: 10.1145/2637166.2637227

**[2]** Yuan, P., Guo, Y., & Chen, X. (2015). "Rethinking compiler optimizations for the Linux kernel: An explorative study." *Proc. 6th Asia-Pacific Workshop on Systems (APSys)*. DOI: 10.1145/2797022.2797033

**[3]** Yuan, P., Guo, Y., Zhang, L., Chen, X., & Mei, H. (2018). "Building application-specific operating systems: A profile-guided approach." *Science China Information Sciences*. DOI: 10.1007/s11432-017-9300-y

**[4]** Ian Bearman. (2020). Exploring Profile Guided Optimization of the Linux Kernel. *Linux Plumbers Conference 2020*. https://lpc.events/event/7/contributions/771/

### Applications

- **PostgreSQL** — compiled from source with configurable CFLAGS/LDFLAGS
- **nginx** — with DPDK for kernel-bypass networking

---

## Step-by-Step Workflow

### 1. Build Instrumented Kernel

Enable GCOV profiling in kernel configuration:

```bash
CONFIG_GCOV_KERNEL=y
CONFIG_GCOV_PROFILE_ALL=y
CONFIG_CONSTRUCTORS=y       # Required for automatic gcov data structure registration
CONFIG_GCOV_FORMAT_5=y      # Required for GCC 8.3 (GCC 5+ format)
```

Add `-fprofile-generate` to kernel compilation flags.

### 2. Collect Kernel Profile Data

For configurations that require kernel profiling (kernel-only PGO and independent combination):

```bash
# Before running workload: reset profile counters
echo 1 > /sys/kernel/debug/gcov/reset

# Run the target workload...

# After workload: collect and package profile data
./scripts/gather.sh profile.tar.gz
```

The `gather.sh` script copies all `.gcda` files from `/sys/kernel/debug/gcov` into a compressed tarball with directory structure preserved, referencing the kernel PGO methodology of [1,2,3].

### 3. Process Collected Profiles

```bash
# Build the checksum validation tool (one-time)
cd tools && g++ -O2 calcsum.cc -o calcsum

# Process the profile tarball
./scripts/process.sh profile.tar.gz
```

This pipeline:
1. Extracts the tarball
2. Runs `calcsum` to validate and repair GCOV integrity checksums on all `.gcda` files
3. Renames each `.gcda` to `.tmp_<name>` to prevent stale data from polluting subsequent runs

### 4. Build Optimized Kernel

```bash
CONFIG_GCOV_KERNEL=n
CONFIG_GCOV_PROFILE_ALL=n

CFLAGS += -fprofile-use=<PATH_TO_PROFILE_DIR> \
          -fprofile-correction \
          -Wno-error=coverage-mismatch \
          -Wno-error
```

This step follows the AutoFDO-inspired approach [4] adapted for kernel-level PGO, using GCC''s `-fprofile-use` to feed collected execution counts back into the compiler.

### 5. Build Instrumented Application

```bash
CFLAGS="-fprofile-generate"
LDFLAGS="-fprofile-generate"
```

Both `CFLAGS` and `LDFLAGS` must include the flag for correct instrumented builds.

### 6. Collect Application Profile Data

```bash
mkdir -p <PROFILE_OUTPUT_DIR>
export GCOV_PREFIX=<PROFILE_OUTPUT_DIR>

# Start the instrumented application (e.g., PostgreSQL)
pg_ctl -D /path/to/data -l logfile start

# Run benchmark workloads...

# Stop the application to flush gcda files to disk
pg_ctl -D /path/to/data stop
```

Unlike the kernel, application profile collection is inherently bounded by process lifecycle — gcda files are written when the process terminates.

### 7. Build Optimized Application

```bash
CFLAGS="-fprofile-use"
LDFLAGS="-fprofile-use"
```

### 8. Run Benchmarks

Refer to `benchmarks/workload_description.md` for the list of all workloads and their execution parameters. Database setup uses `benchmarks/create_table.sql` and `benchmarks/postgresql_benchmark.conf`.

Each product is evaluated under all four optimization configurations, with performance measured as throughput (transactions/second for databases, requests/second for web servers).

---

## `calcsum` — GCOV Profile Integrity Tool

**Source:** `tools/calcsum.cc`

When kernel gcda files are collected from `/sys/kernel/debug/gcov`, they may contain incomplete or corrupted counter data (e.g., missing `GCOV_TAG_PROGRAM_SUMMARY`). This tool parses the binary GCOV data format and:

- Walks all `GCOV_TAG_COUNTER_BASE` entries to accumulate execution counters
- Computes a histogram of counter values
- Emits corrected `GCOV_TAG_PROGRAM_SUMMARY` blocks with proper checksums

**Compilation:**

```bash
g++ -O2 tools/calcsum.cc -o calcsum
```

**Usage:**

```bash
# Generate file list from collected gcda files
find sys -name '*.gcda' > list.txt

# Validate and repair all gcda files
./calcsum list.txt
```

The tool modifies gcda files in-place, inserting correct summary data. This step is required before using the profiles for `-fprofile-use`.

---

## Database Configuration

### `benchmarks/create_table.sql`

Defines the benchmark tables:

- **`vanke`** — primary table with 35 columns, simulating a realistic business schema. Includes a unique index on `housename`.
- **`vanke1`** — clone of `vanke` without primary key and unique constraints, used for high-throughput INSERT testing.

### `benchmarks/postgresql_benchmark.conf`

PostgreSQL configuration covering three profiles:
1. **Full configuration** — with extensions (orafce, pg_pathman, pg_stat_statements, etc.), remote connections, and detailed logging
2. **Without extensions** — same as above but without `shared_preload_libraries`
3. **Minimal tuning** — reduced parameter set for throughput-focused benchmarking

Key tuning parameters include connection limits, shared buffers, WAL settings, autovacuum, and planner cost constants.

### `benchmarks/workload_description.md`

Describes all five evaluation workloads:

| Workload | Type | Concurrency | Duration |
|---|---|---|---|
| SELECT (unique index) | Read | 1, 100, 500 threads | 5 min |
| UPDATE (random equality) | Write | 1, 100, 500 threads | 5 min |
| INSERT (random PK) | Write | 1, 100, 500 threads | 5 min |
| TPC-C | Mixed OLTP | 1, 100, 500 threads | 5 min |
| TPC-H (22 queries) | Analytical | Sequential | Per-query |
