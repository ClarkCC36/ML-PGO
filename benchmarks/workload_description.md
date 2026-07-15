# Benchmark Workloads

This document describes the five benchmark workloads used to evaluate PostgreSQL performance.

## 1. SELECT (Unique Index Lookup)

Query pattern:
```sql
SELECT * FROM <table> WHERE housename = '<value>'
```

Concurrency levels: 1, 100, 500 threads  
Duration: 5 minutes per run

## 2. UPDATE (Random Equality Update)

Query pattern:
```sql
UPDATE <table> SET buildingNo = <val>, carportprice = 77777.77, householdname = '<name>' WHERE housename = '<key>'
```

Concurrency levels: 1, 100, 500 threads  
Duration: 5 minutes per run

## 3. INSERT (Random Primary Key)

Random primary key insertion into `vanke1` (a copy of `vanke` without constraints).

Concurrency levels: 1, 100, 500 threads  
Duration: 5 minutes per run

## 4. TPC-C (OLTP Mixed Workload)

Standard TPC-C benchmark simulating a complex transaction processing workload.

Concurrency levels: 1, 100, 500 threads  
Duration: 5 minutes per run

## 5. TPC-H (Analytical Queries)

All 22 analytical queries from the TPC-H benchmark (`dbgen`), measuring individual query execution time (sequential, single run).