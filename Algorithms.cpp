#include "Algorithms.h"

#include <algorithm>

namespace {

// ---- Comparators ----------------------------------------------------------

// Order for COUNT and ENROLLMENTS: finish, then start, then id.
bool lessByFinish(const Course& a, const Course& b) {
    if (a.finish != b.finish) return a.finish < b.finish;
    if (a.start != b.start) return a.start < b.start;
    return a.id < b.id;
}

// Order for ROOMS: start, then finish, then id.
bool lessByStart(const Course& a, const Course& b) {
    if (a.start != b.start) return a.start < b.start;
    if (a.finish != b.finish) return a.finish < b.finish;
    return a.id < b.id;
}

using Less = bool (*)(const Course&, const Course&);

// ---- Merge sort -----------------------------------------------------------

// Merges the sorted halves [lo, mid) and [mid, hi) of v using buffer tmp.
void merge(std::vector<Course>& v, std::vector<Course>& tmp,
           std::size_t lo, std::size_t mid, std::size_t hi, Less less) {
    std::size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        // Take from the right half only if it is strictly smaller (keeps the merge stable).
        if (less(v[j], v[i])) tmp[k++] = v[j++];
        else                  tmp[k++] = v[i++];
    }
    while (i < mid) tmp[k++] = v[i++];
    while (j < hi)  tmp[k++] = v[j++];
    for (std::size_t t = lo; t < hi; ++t) v[t] = tmp[t];
}

// Sorts v[lo, hi). Base case: a range of 0 or 1 elements is already sorted.
void mergeSortRange(std::vector<Course>& v, std::vector<Course>& tmp,
                    std::size_t lo, std::size_t hi, Less less) {
    if (hi - lo < 2) return;
    std::size_t mid = lo + (hi - lo) / 2;
    mergeSortRange(v, tmp, lo, mid, less);
    mergeSortRange(v, tmp, mid, hi, less);
    merge(v, tmp, lo, mid, hi, less);
}

void mergeSortWith(std::vector<Course>& v, Less less) {
    if (v.size() < 2) return;
    std::vector<Course> tmp(v.size());
    mergeSortRange(v, tmp, 0, v.size(), less);
}

// ---- Dynamic programming core (shared by both DP versions) ----------------

// p is 0-based (-1 = no compatible previous course).
// M[j] (1-based) = best total enrollments using only the first j courses.
// M[0] = 0;  M[j] = max(M[j-1], w_j + M[p(j)+1]).
// Reconstruction: include course j only if including it is STRICTLY better (ties -> exclude).
std::vector<Course> solveWeighted(const std::vector<Course>& sorted, const std::vector<int>& p) {
    const std::size_t n = sorted.size();
    std::vector<long long> M(n + 1, 0);
    for (std::size_t j = 1; j <= n; ++j) {
        long long skip = M[j - 1];
        long long take = sorted[j - 1].enrollments + M[static_cast<std::size_t>(p[j - 1] + 1)];
        M[j] = std::max(skip, take);
    }
    std::vector<Course> chosen;
    std::size_t j = n;
    while (j > 0) {
        long long take = sorted[j - 1].enrollments + M[static_cast<std::size_t>(p[j - 1] + 1)];
        if (take > M[j - 1]) {
            chosen.push_back(sorted[j - 1]);
            j = static_cast<std::size_t>(p[j - 1] + 1);
        } else {
            j = j - 1;
        }
    }
    std::reverse(chosen.begin(), chosen.end());   // back to the order they are held in the room
    return chosen;
}

bool isCompatibleSet(const std::vector<Course>& c, unsigned mask) {
    for (std::size_t a = 0; a < c.size(); ++a) {
        if (!(mask & (1u << a))) continue;
        for (std::size_t b = a + 1; b < c.size(); ++b)
            if ((mask & (1u << b)) && overlaps(c[a], c[b])) return false;
    }
    return true;
}

// Tries every partition of courses into rooms (restricted growth strings).
void searchPartitions(const std::vector<Course>& c, std::vector<int>& room, std::size_t idx,
                      int usedRooms, std::size_t& best) {
    if (static_cast<std::size_t>(usedRooms) >= best) return;   // cannot improve
    if (idx == c.size()) { best = static_cast<std::size_t>(usedRooms); return; }
    // Put course idx in an existing room r (0..usedRooms-1) or open room usedRooms.
    for (int r = 0; r <= usedRooms; ++r) {
        bool ok = true;
        for (std::size_t k = 0; k < idx && ok; ++k)
            if (room[k] == r && overlaps(c[k], c[idx])) ok = false;
        if (!ok) continue;
        room[idx] = r;
        searchPartitions(c, room, idx + 1, r == usedRooms ? usedRooms + 1 : usedRooms, best);
    }
}

} // namespace

// ---- Public helpers -------------------------------------------------------

bool overlaps(const Course& a, const Course& b) {
    return a.start < b.finish && b.start < a.finish;
}

long long totalEnrollments(const std::vector<Course>& courses) {
    long long sum = 0;
    for (const Course& c : courses) sum += c.enrollments;
    return sum;
}

void mergeSort(std::vector<Course>& courses) { mergeSortWith(courses, lessByFinish); }

void mergeSortByStart(std::vector<Course>& courses) { mergeSortWith(courses, lessByStart); }

// ---- COUNT: greedy --------------------------------------------------------

std::vector<Course> selectCourses(const std::vector<Course>& sortedCourses) {
    std::vector<Course> chosen;
    for (const Course& c : sortedCourses) {
        // Take the first course that starts at or after the last chosen course ends.
        if (chosen.empty() || c.start >= chosen.back().finish) chosen.push_back(c);
    }
    return chosen;
}

// ---- ENROLLMENTS: dynamic programming ------------------------------------

std::vector<int> computePredecessorsLinear(const std::vector<Course>& sortedCourses) {
    const int n = static_cast<int>(sortedCourses.size());
    std::vector<int> p(static_cast<std::size_t>(n), -1);
    for (int j = 0; j < n; ++j) {
        // Scan backwards; the first compatible course found is the last one (largest i).
        for (int i = j - 1; i >= 0; --i) {
            if (sortedCourses[static_cast<std::size_t>(i)].finish <= sortedCourses[static_cast<std::size_t>(j)].start) {
                p[static_cast<std::size_t>(j)] = i;
                break;
            }
        }
    }
    return p;
}

std::vector<int> computePredecessorsBinary(const std::vector<Course>& sortedCourses) {
    const int n = static_cast<int>(sortedCourses.size());
    std::vector<int> p(static_cast<std::size_t>(n), -1);
    for (int j = 0; j < n; ++j) {
        // Finish times are non-decreasing, so "finish <= start(j)" is true on a prefix.
        // Find the last index in [0, j-1] where it holds.
        int lo = 0, hi = j - 1, ans = -1;
        const int s = sortedCourses[static_cast<std::size_t>(j)].start;
        while (lo <= hi) {
            int mid = lo + (hi - lo) / 2;
            if (sortedCourses[static_cast<std::size_t>(mid)].finish <= s) { ans = mid; lo = mid + 1; }
            else hi = mid - 1;
        }
        p[static_cast<std::size_t>(j)] = ans;
    }
    return p;
}

std::vector<Course> selectCoursesDP(const std::vector<Course>& sortedCourses) {
    return solveWeighted(sortedCourses, computePredecessorsLinear(sortedCourses));
}

std::vector<Course> selectCoursesDPBinary(const std::vector<Course>& sortedCourses) {
    return solveWeighted(sortedCourses, computePredecessorsBinary(sortedCourses));
}

// ---- ROOMS: greedy --------------------------------------------------------

std::vector<std::vector<Course>> allocateClassrooms(const std::vector<Course>& courses) {
    std::vector<Course> sorted = courses;          // the input is not modified
    mergeSortByStart(sorted);
    std::vector<std::vector<Course>> rooms;
    std::vector<int> lastFinish;                   // finish time of the last course in each room
    for (const Course& c : sorted) {
        int best = -1;
        for (std::size_t r = 0; r < rooms.size(); ++r) {
            // Free room = its last course ended by c.start. Prefer the earliest such finish;
            // strict '<' keeps the smallest room number on ties.
            if (lastFinish[r] <= c.start &&
                (best == -1 || lastFinish[r] < lastFinish[static_cast<std::size_t>(best)]))
                best = static_cast<int>(r);
        }
        if (best == -1) {                          // no free room: open a new one
            rooms.push_back({c});
            lastFinish.push_back(c.finish);
        } else {
            rooms[static_cast<std::size_t>(best)].push_back(c);
            lastFinish[static_cast<std::size_t>(best)] = c.finish;
        }
    }
    return rooms;
}

// ---- Exhaustive checkers (tests only, n <= 5) -----------------------------

std::size_t optimalCountExhaustive(const std::vector<Course>& courses) {
    std::size_t best = 0;
    const unsigned total = 1u << courses.size();
    for (unsigned mask = 0; mask < total; ++mask) {
        if (!isCompatibleSet(courses, mask)) continue;
        std::size_t cnt = 0;
        for (std::size_t a = 0; a < courses.size(); ++a) if (mask & (1u << a)) ++cnt;
        best = std::max(best, cnt);
    }
    return best;
}

long long optimalEnrollmentsExhaustive(const std::vector<Course>& courses) {
    long long best = 0;
    const unsigned total = 1u << courses.size();
    for (unsigned mask = 0; mask < total; ++mask) {
        if (!isCompatibleSet(courses, mask)) continue;
        long long sum = 0;
        for (std::size_t a = 0; a < courses.size(); ++a) if (mask & (1u << a)) sum += courses[a].enrollments;
        best = std::max(best, sum);
    }
    return best;
}

std::size_t minClassroomsExhaustive(const std::vector<Course>& courses) {
    if (courses.empty()) return 0;
    std::size_t best = courses.size();             // one room per course always works
    std::vector<int> room(courses.size(), -1);
    searchPartitions(courses, room, 0, 0, best);
    return best;
}
