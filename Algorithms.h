#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <string>
#include <vector>
#include <cstddef>
#include "Course.h"

// ---- Required signatures -------------------------------------------------

// Sorts by finish time, then start time, then id (all ascending). Merge sort, O(n log n).
void mergeSort(std::vector<Course>& courses);

// COUNT: greedy by earliest finish. Input must be sorted with mergeSort. Input is not modified.
std::vector<Course> selectCourses(const std::vector<Course>& sortedCourses);

// ENROLLMENTS: dynamic programming (linear backward scan for p(j)). Input sorted with mergeSort.
std::vector<Course> selectCoursesDP(const std::vector<Course>& sortedCourses);

// Test-only brute force (at most 5 courses, any order).
std::size_t optimalCountExhaustive(const std::vector<Course>& courses);

// ROOMS: greedy by start time, reuse the free room that became free earliest. Input in any order.
std::vector<std::vector<Course>> allocateClassrooms(const std::vector<Course>& courses);

// Test-only brute force over all partitions (at most 5 courses, any order).
std::size_t minClassroomsExhaustive(const std::vector<Course>& courses);

// Test-only brute force (at most 5 courses, any order).
long long optimalEnrollmentsExhaustive(const std::vector<Course>& courses);

// ---- Optional extension ---------------------------------------------------

// Same as selectCoursesDP, but p(j) is found with binary search. O(n log n) overall.
std::vector<Course> selectCoursesDPBinary(const std::vector<Course>& sortedCourses);

// ---- Helpers (exposed for tests) ------------------------------------------

// Sorts by start time, then finish time, then id (all ascending). Merge sort, O(n log n).
void mergeSortByStart(std::vector<Course>& courses);

// True if two courses cannot share a room. Touching endpoints (a.finish == b.start) do not overlap.
bool overlaps(const Course& a, const Course& b);

// p[j] for a list sorted by finish time, 0-based: index of the last i < j with
// finish(i) <= start(j), or -1 if none. Linear backward scan (O(n^2) total).
std::vector<int> computePredecessorsLinear(const std::vector<Course>& sortedCourses);

// Same values as computePredecessorsLinear, computed with binary search (O(n log n) total).
std::vector<int> computePredecessorsBinary(const std::vector<Course>& sortedCourses);

// Sum of enrollments of a list of courses.
long long totalEnrollments(const std::vector<Course>& courses);

#endif // ALGORITHMS_H
