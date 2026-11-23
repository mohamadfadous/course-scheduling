# Course Scheduling in Classrooms – שיבוץ קורסים בכיתות

**Team:** Muhammad Fadous (individual work)

C++17 program that solves three optimization problems on a list of courses held on one day:

| Part | Problem | Method | Time |
|---|---|---|---|
| `COUNT` | Max number of non-overlapping courses in one room | Greedy, earliest finish | O(n log n) |
| `ROOMS` | Schedule **all** courses in the minimum number of rooms | Greedy by start time, reuse the room that became free earliest (linear scan) | O(n²) |
| `ENROLLMENTS` | Max total enrollments in one room | Dynamic programming, p(j) by backward scan | O(n²) |
| optional | Same as `ENROLLMENTS`, p(j) by binary search | `selectCoursesDPBinary` | O(n log n) |

Both sort orders use a hand-written **merge sort** (no library sort).

## Files

| File | Role |
|---|---|
| `Course.h` | `struct Course` (id, start, finish, enrollments), include guard |
| `Algorithms.h` | All required signatures + the optional extension + small helpers used by tests |
| `Algorithms.cpp` | Merge sort (2 orders), greedy COUNT, greedy ROOMS, DP (shared core for linear / binary p(j)), 3 exhaustive checkers. No `main` |
| `main.cpp` | Reads input, runs the algorithms, prints `COUNT`, `ROOMS`, `ENROLLMENTS` |
| `tests.cpp` | doctest tests: given examples, edge cases, p(j) critique, random comparison with exhaustive solutions |
| `doctest.h` | doctest single-header library (v2.5.3) |
| `testdata/` | `sample`, `empty`, `ties` from the assignment + extra cases (`.in` / `.out`) |

## Build and run

Tested with **g++ 15.2.0** (MinGW-W64, Windows 11) and **g++ 13.3.0** (Ubuntu 24.04), `-std=c++17`. Result: 24 test cases, 22,761 assertions, all passed; no compiler warnings.

```bash
# main program (main.cpp + Algorithms.cpp)
g++ -std=c++17 -Wall -Wextra -O2 main.cpp Algorithms.cpp -o scheduler

# tests (tests.cpp + Algorithms.cpp; doctest provides main)
g++ -std=c++17 -Wall -Wextra -O2 tests.cpp Algorithms.cpp -o tests
./tests
```

Run with input/output redirection and compare to the expected output:

```bash
./scheduler < testdata/sample.in > out.txt
diff out.txt testdata/sample.out        # no output = identical

for t in sample empty ties touching overlap_all weighted; do
  ./scheduler < testdata/$t.in | diff -q - testdata/$t.out && echo "$t OK"
done
```

On Windows (cmd, MinGW):

```bat
g++ -std=c++17 -Wall -Wextra -O2 main.cpp Algorithms.cpp -o scheduler.exe
g++ -std=c++17 -Wall -Wextra -O2 tests.cpp Algorithms.cpp -o tests.exe
tests.exe
scheduler.exe < testdata\sample.in > out.txt
fc /W out.txt testdata\sample.out
```

**External dependency:** only `doctest.h` (header-only), included in the repository.
Source: https://github.com/doctest/doctest (release v2.5.3, file `doctest/doctest.h`).

## Input / output format

```
3
Algorithms 540 600 4
Databases 600 660 4
Operating_Systems 540 660 11
```
Times are minutes from midnight (09:00 = 540). Output:
```
COUNT
2 8
Algorithms 540 600 4
Databases 600 660 4
ROOMS
2
1 Algorithms 540 600 4
1 Databases 600 660 4
2 Operating_Systems 540 660 11
ENROLLMENTS
1 11
Operating_Systems 540 660 11
```

## Tests (summary)

- Given examples: sample, empty input, ties.
- Edge cases: single course, touching endpoints, all overlapping, equal times with different ids, unsorted input, COUNT vs ENROLLMENTS gap, several optimal solutions (DP tie rule), reusing a freed room vs opening a new one, choosing the room that became free earliest, `long long` totals.
- **p(j) critique:** a case where the predecessor is not `j-1` (wrong rule gives 5, correct answer 10).
- **Random vs exhaustive:** 3000 inputs with 0–5 courses (seed `2026`), times on a 5-minute grid in [0, 120], enrollments 1–10. All three results equal the exhaustive optimum and are valid.
- **Large inputs:** 200 inputs with 6–300 courses (seed `777`), validity only; ROOMS also equals the maximum number of simultaneous courses (lower bound).
- Mutation check during development: planted bugs (p(j) = j-1, DP tie includes, first free room, touching = overlap, strict greedy, sort ignoring id) are each caught by at least one test.

## AI usage

Claude (Anthropic, model Claude Opus 5.5) was used to help write the code, the tests and the explanations. All outputs were reviewed, compiled and checked against the assignment requirements and the tests above.
