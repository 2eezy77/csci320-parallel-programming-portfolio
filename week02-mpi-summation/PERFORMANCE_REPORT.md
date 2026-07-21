# Performance Report: Static vs Dynamic Parallel Summation

## Test Setup

I tested both implementations with N = 10,000,000 elements using p = 1, 2, 4, and 8 processes. Both programs use the same random seed (987654321) so they're working on the same data. I had to make sure they used the same seed; I kept running into issues where they were generating different data and I couldn't compare them fairly.

---

## Results

### Execution Times

| Processes | Static (seconds) | Dynamic (seconds) | Static Speedup | Dynamic Speedup |
| --------- | ---------------- | ----------------- | -------------- | --------------- |
| 1         | 0.0553           | 0.0453            | 1.00x          | 1.00x           |
| 2         | 0.0276           | 0.0415            | 2.01x          | 1.09x           |
| 4         | 0.0139           | 0.0842            | 3.97x          | 0.54x           |
| 8         | 0.0071           | 0.0062            | 7.75x          | 7.33x           |

**Correctness:** Both implementations produce the same checksum (1) and same total sum (7464.454000), so they're both correct.

---

## Analysis

### Static Approach

The static approach scales really well. With 2 processes it's about 2x faster, with 4 processes it's almost 4x faster, and with 8 processes it's about 7.75x faster. This makes sense because it has low communication overhead - just one MPI_Scatterv to distribute data at the start, then a tree reduction to combine results at the end.

The downside is that if busy minutes aren't spread evenly, some processes might finish way before others. But in my tests, the work was distributed pretty evenly so this wasn't a big problem.

### Dynamic Approach

The dynamic approach is interesting. With 1 process it's actually faster than static (0.0453 vs 0.0553 seconds), probably because there's no MPI communication at all. But with 2 and 4 processes, it gets slower because of all the communication overhead - every chunk needs a request, work assignment, and result message. With 4 processes it's actually slower than the single-process version (0.0842 vs 0.0453 seconds), which shows how much the overhead hurts.

But with 8 processes it catches up and actually becomes slightly faster (0.0062 vs 0.0071 seconds). I think this is because with more workers, the adaptive chunk sizing can better balance the load, and the overhead gets spread across more processes so it doesn't hurt as much.

The problem is that dynamic broadcasts the full array to everyone, which uses more memory. But for this problem size it's not a big deal.

---

## Comparison

**When static is better:**

- When you have uniform workloads (like in my tests)
- When you want to minimize communication overhead
- When memory is limited

**When dynamic is better:**

- When workloads are uneven and you need load balancing
- When you have many processes (8+) where the overhead gets amortized
- When some processes might be slower than others

**What I learned:**
The communication overhead in dynamic really hurts at low process counts. With 2 or 4 processes, static is clearly better - dynamic is actually slower with 4 processes than with 1 process, which is pretty bad! But at 8 processes, dynamic catches up and becomes slightly faster. I think the adaptive chunk sizing helps balance the work better when you have more workers, and the overhead gets spread out so it doesn't hurt as much.

---

## Conclusions

1. Both implementations are correct - they produce the same results.
2. Static is better for most cases, especially with fewer processes. It scales really well and has less overhead.
3. Dynamic can be faster with many processes (8+), but the communication overhead makes it slower with fewer processes. In fact, with 4 processes, dynamic is slower than running with just 1 process!
4. For uniform workloads like this assignment, I'd recommend static. But for real-world problems where some parts take longer than others, dynamic might be better because it can adapt.

---

## Test Commands

```bash
# Static tests
mpirun -np 1 ./sum_static 10000000
mpirun -np 2 ./sum_static 10000000
mpirun -np 4 ./sum_static 10000000
mpirun -np 8 ./sum_static 10000000

# Dynamic tests
mpirun -np 1 ./sum_dynamic 10000000
mpirun -np 2 ./sum_dynamic 10000000
mpirun -np 4 ./sum_dynamic 10000000
mpirun -np 8 ./sum_dynamic 10000000
```
