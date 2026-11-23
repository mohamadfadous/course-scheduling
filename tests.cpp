// Automated tests for the course scheduling project (doctest).
// Build: g++ -std=c++17 -Wall -Wextra tests.cpp Algorithms.cpp -o tests
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <algorithm>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "Algorithms.h"

namespace {

Course C(const std::string& id, int s, int f, long long e) { return Course{id, s, f, e}; }

bool sameCourse(const Course& a, const Course& b) {
    return a.id == b.id && a.start == b.start && a.finish == b.finish && a.enrollments == b.enrollments;
}

// Every chosen course comes from the input unchanged and appears at most once.
bool takenFromInput(const std::vector<Course>& chosen, const std::vector<Course>& input) {
    std::set<std::string> seen;
    for (const Course& c : chosen) {
        if (!seen.insert(c.id).second) return false;               // duplicate id
        bool found = false;
        for (const Course& in : input) if (sameCourse(c, in)) { found = true; break; }
        if (!found) return false;                                  // modified or invented course
    }
    return true;
}

// A single-room schedule: no overlaps and listed in the order they are held.
bool validRoom(const std::vector<Course>& room) {
    for (std::size_t i = 1; i < room.size(); ++i)
        if (room[i - 1].finish > room[i].start) return false;
    return true;
}

bool validSelection(const std::vector<Course>& chosen, const std::vector<Course>& input) {
    return takenFromInput(chosen, input) && validRoom(chosen);
}

// ROOMS validity: every input course exactly once, no empty room, no overlap inside a room.
bool validAllocation(const std::vector<std::vector<Course>>& rooms, const std::vector<Course>& input) {
    std::size_t total = 0;
    std::vector<Course> all;
    for (const auto& room : rooms) {
        if (room.empty() || !validRoom(room)) return false;
        total += room.size();
        all.insert(all.end(), room.begin(), room.end());
    }
    return total == input.size() && takenFromInput(all, input);
}

// Maximum number of courses running at the same moment (an independent lower bound for ROOMS).
std::size_t maxSimultaneous(const std::vector<Course>& courses) {
    std::vector<std::pair<int, int>> events;          // (time, +1 start / -1 finish)
    for (const Course& c : courses) { events.push_back({c.start, +1}); events.push_back({c.finish, -1}); }
    std::sort(events.begin(), events.end());          // at equal time, -1 comes before +1
    std::size_t cur = 0, best = 0;
    for (const auto& e : events) {
        if (e.second == -1) --cur; else { ++cur; best = std::max(best, cur); }
    }
    return best;
}

std::vector<Course> sortedCopy(std::vector<Course> v) { mergeSort(v); return v; }

std::vector<std::string> ids(const std::vector<Course>& v) {
    std::vector<std::string> r;
    for (const Course& c : v) r.push_back(c.id);
    return r;
}

// Random instance; times on a 5-minute grid so that touching and equal endpoints are common.
std::vector<Course> randomCourses(std::mt19937& rng, int n, int maxTime, int maxEnroll) {
    std::uniform_int_distribution<int> t(0, maxTime / 5), len(1, 12), w(1, maxEnroll);
    std::vector<Course> v;
    for (int i = 0; i < n; ++i) {
        int s = t(rng) * 5;
        v.push_back(C("C" + std::to_string(i), s, s + len(rng) * 5, w(rng)));
    }
    return v;
}

} // namespace

// ============================================================================
// Given examples (same as testdata/*.in / *.out)
// ============================================================================

TEST_CASE("sample from the assignment") {
    std::vector<Course> in = {C("Algorithms", 540, 600, 4), C("Databases", 600, 660, 4),
                              C("Operating_Systems", 540, 660, 11)};
    auto s = sortedCopy(in);
    CHECK(ids(selectCourses(s)) == std::vector<std::string>{"Algorithms", "Databases"});
    auto rooms = allocateClassrooms(in);
    REQUIRE(rooms.size() == 2);
    CHECK(ids(rooms[0]) == std::vector<std::string>{"Algorithms", "Databases"});
    CHECK(ids(rooms[1]) == std::vector<std::string>{"Operating_Systems"});
    auto dp = selectCoursesDP(s);
    CHECK(ids(dp) == std::vector<std::string>{"Operating_Systems"});
    CHECK(totalEnrollments(dp) == 11);
}

TEST_CASE("empty input") {
    std::vector<Course> in;
    auto s = sortedCopy(in);
    CHECK(selectCourses(s).empty());
    CHECK(allocateClassrooms(in).empty());
    CHECK(selectCoursesDP(s).empty());
    CHECK(selectCoursesDPBinary(s).empty());
    CHECK(optimalCountExhaustive(in) == 0);
    CHECK(minClassroomsExhaustive(in) == 0);
    CHECK(optimalEnrollmentsExhaustive(in) == 0);
}

TEST_CASE("ties: equal times and equal enrollments (id breaks the tie, DP tie -> exclude)") {
    std::vector<Course> in = {C("Databases", 540, 600, 5), C("Algorithms", 540, 600, 5),
                              C("Operating_Systems", 600, 660, 2)};
    auto s = sortedCopy(in);
    CHECK(ids(s) == std::vector<std::string>{"Algorithms", "Databases", "Operating_Systems"});
    CHECK(ids(selectCourses(s)) == std::vector<std::string>{"Algorithms", "Operating_Systems"});
    auto rooms = allocateClassrooms(in);
    REQUIRE(rooms.size() == 2);
    CHECK(ids(rooms[0]) == std::vector<std::string>{"Algorithms", "Operating_Systems"});
    CHECK(ids(rooms[1]) == std::vector<std::string>{"Databases"});
    auto dp = selectCoursesDP(s);
    // Catches: including Databases on a tie, or picking by input order instead of sorted order.
    CHECK(ids(dp) == std::vector<std::string>{"Algorithms", "Operating_Systems"});
    CHECK(totalEnrollments(dp) == 7);
}

// ============================================================================
// Merge sort
// ============================================================================

TEST_CASE("mergeSort: finish, then start, then id; content is preserved") {
    std::vector<Course> v = {C("D", 5, 10, 1), C("B", 0, 10, 2), C("A", 0, 10, 3),
                             C("E", 1, 3, 4), C("C", 2, 20, 5)};
    auto original = v;
    mergeSort(v);
    CHECK(ids(v) == std::vector<std::string>{"E", "A", "B", "D", "C"});
    CHECK(validSelection(std::vector<Course>{}, v));
    CHECK(takenFromInput(v, original));                // nothing lost, nothing modified
    CHECK(v.size() == original.size());
}

TEST_CASE("mergeSortByStart: start, then finish, then id") {
    std::vector<Course> v = {C("B", 0, 10, 1), C("A", 0, 10, 1), C("C", 0, 5, 1), C("D", -0, 7, 1), C("E", 3, 4, 1)};
    mergeSortByStart(v);
    CHECK(ids(v) == std::vector<std::string>{"C", "D", "A", "B", "E"});
}

TEST_CASE("mergeSort agrees with a reference order on random inputs (odd and even sizes)") {
    std::mt19937 rng(12345);
    for (int it = 0; it < 300; ++it) {
        auto v = randomCourses(rng, it % 40, 200, 9);
        auto ref = v;
        std::stable_sort(ref.begin(), ref.end(), [](const Course& a, const Course& b) {
            if (a.finish != b.finish) return a.finish < b.finish;
            if (a.start != b.start) return a.start < b.start;
            return a.id < b.id;
        });
        mergeSort(v);
        bool same = v.size() == ref.size();
        for (std::size_t i = 0; same && i < v.size(); ++i) same = sameCourse(v[i], ref[i]);
        CHECK(same);
    }
}

// ============================================================================
// COUNT (greedy)
// ============================================================================

TEST_CASE("COUNT: single course and touching endpoints") {
    std::vector<Course> one = {C("Only", 10, 20, 3)};
    CHECK(selectCourses(sortedCopy(one)).size() == 1);
    // Touching courses (finish == next start) must be allowed in the same room.
    std::vector<Course> chain = {C("A", 0, 10, 1), C("B", 10, 20, 1), C("C", 20, 30, 1)};
    CHECK(selectCourses(sortedCopy(chain)).size() == 3);
}

TEST_CASE("COUNT: all courses overlap -> exactly one course") {
    std::vector<Course> v = {C("A", 0, 100, 1), C("B", 10, 90, 1), C("C", 20, 80, 1), C("D", 30, 70, 1)};
    auto r = selectCourses(sortedCopy(v));
    CHECK(r.size() == 1);
    CHECK(ids(r) == std::vector<std::string>{"D"});    // earliest finish
}

TEST_CASE("COUNT: earliest-start rule would fail, earliest-finish succeeds") {
    // Catches a greedy that sorts by start: it would take Long and get 1 course.
    std::vector<Course> v = {C("Long", 0, 100, 1), C("S1", 10, 20, 1), C("S2", 30, 40, 1)};
    CHECK(selectCourses(sortedCopy(v)).size() == 2);
}

TEST_CASE("COUNT: shortest-first rule would fail, earliest-finish succeeds") {
    // Catches a greedy that picks the shortest course first (Short blocks both others).
    std::vector<Course> v = {C("Left", 0, 10, 1), C("Short", 8, 12, 1), C("Right", 11, 20, 1)};
    CHECK(selectCourses(sortedCopy(v)).size() == 2);
}

TEST_CASE("COUNT ignores enrollments; ENROLLMENTS may choose fewer courses") {
    std::vector<Course> v = {C("A", 0, 10, 1), C("B", 10, 20, 1), C("Big", 0, 20, 50)};
    auto s = sortedCopy(v);
    CHECK(selectCourses(s).size() == 2);
    auto dp = selectCoursesDP(s);
    CHECK(ids(dp) == std::vector<std::string>{"Big"});
    CHECK(totalEnrollments(dp) == 50);
}

TEST_CASE("selection functions do not modify their input") {
    std::vector<Course> v = {C("A", 0, 10, 1), C("B", 5, 15, 2), C("C", 10, 20, 3)};
    auto s = sortedCopy(v);
    auto before = s;
    selectCourses(s); selectCoursesDP(s); selectCoursesDPBinary(s);
    auto inBefore = v;
    allocateClassrooms(v);
    bool same = true;
    for (std::size_t i = 0; i < s.size(); ++i) same = same && sameCourse(s[i], before[i]) && sameCourse(v[i], inBefore[i]);
    CHECK(same);
}

// ============================================================================
// ROOMS (greedy)
// ============================================================================

TEST_CASE("ROOMS: reuse a room that became free, open a new room only when needed") {
    std::vector<Course> v = {C("A", 0, 10, 1), C("B", 5, 15, 1), C("C", 10, 20, 1), C("D", 15, 25, 1)};
    auto rooms = allocateClassrooms(v);
    REQUIRE(rooms.size() == 2);
    CHECK(ids(rooms[0]) == std::vector<std::string>{"A", "C"});
    CHECK(ids(rooms[1]) == std::vector<std::string>{"B", "D"});
}

TEST_CASE("ROOMS: among free rooms choose the one that became free earliest") {
    // When E starts at 30, room 2 (free since 20) and room 1 (free since 25) are both free.
    // Rule: earliest last finish -> room 2. Catches "first free room" implementations.
    std::vector<Course> v = {C("A", 0, 25, 1), C("B", 1, 20, 1), C("E", 30, 40, 1)};
    auto rooms = allocateClassrooms(v);
    REQUIRE(rooms.size() == 2);
    CHECK(ids(rooms[0]) == std::vector<std::string>{"A"});
    CHECK(ids(rooms[1]) == std::vector<std::string>{"B", "E"});
}

TEST_CASE("ROOMS: equal free times -> smallest room number") {
    std::vector<Course> v = {C("A", 0, 10, 1), C("B", 0, 10, 1), C("C", 10, 20, 1)};
    auto rooms = allocateClassrooms(v);
    REQUIRE(rooms.size() == 2);
    CHECK(ids(rooms[0]) == std::vector<std::string>{"A", "C"});
}

TEST_CASE("ROOMS: all overlap -> one room per course; unsorted input") {
    std::vector<Course> v = {C("C", 20, 80, 1), C("A", 0, 100, 1), C("B", 10, 90, 1)};
    auto rooms = allocateClassrooms(v);
    CHECK(rooms.size() == 3);
    CHECK(validAllocation(rooms, v));
    CHECK(ids(rooms[0]) == std::vector<std::string>{"A"});   // rooms numbered by opening order
}

// ============================================================================
// ENROLLMENTS (DP) and the p(j) critique
// ============================================================================

TEST_CASE("p(j) critique: predecessor is not always j-1") {
    // Sorted by finish: A[0,10] B[5,20] C[15,30]. For C, B (j-1) overlaps but A is compatible.
    // The wrong rule (look only at j-1) gives p(C) = -1 and total 5 instead of 10.
    std::vector<Course> v = {C("A", 0, 10, 5), C("B", 5, 20, 1), C("C", 15, 30, 5)};
    auto s = sortedCopy(v);
    auto p = computePredecessorsLinear(s);
    CHECK(p == std::vector<int>{-1, -1, 0});
    CHECK(computePredecessorsBinary(s) == p);
    auto dp = selectCoursesDP(s);
    CHECK(totalEnrollments(dp) == 10);
    CHECK(ids(dp) == std::vector<std::string>{"A", "C"});
}

TEST_CASE("p(j): touching endpoints count as compatible") {
    std::vector<Course> v = {C("A", 0, 10, 1), C("B", 10, 20, 1)};
    CHECK(computePredecessorsLinear(sortedCopy(v)) == std::vector<int>{-1, 0});
}

TEST_CASE("ENROLLMENTS: several optimal solutions -> tie rule excludes the current course") {
    // Sorted: X[0,10,6] Y[0,20,6]. Both give 6; at Y the tie -> exclude Y -> answer X.
    std::vector<Course> v = {C("Y", 0, 20, 6), C("X", 0, 10, 6)};
    auto dp = selectCoursesDP(sortedCopy(v));
    CHECK(ids(dp) == std::vector<std::string>{"X"});
}

TEST_CASE("ENROLLMENTS: weighted example where greedy-by-finish is not optimal") {
    std::vector<Course> v = {C("A", 0, 10, 2), C("B", 5, 25, 10), C("C", 10, 20, 2)};
    auto s = sortedCopy(v);
    CHECK(totalEnrollments(selectCourses(s)) == 4);
    CHECK(totalEnrollments(selectCoursesDP(s)) == 10);
}

TEST_CASE("large enrollments fit in long long") {
    std::vector<Course> v = {C("A", 0, 10, 3000000000LL), C("B", 10, 20, 3000000000LL)};
    CHECK(totalEnrollments(selectCoursesDP(sortedCopy(v))) == 6000000000LL);
}

// ============================================================================
// Exhaustive checkers sanity
// ============================================================================

TEST_CASE("exhaustive checkers on hand-made inputs") {
    std::vector<Course> v = {C("Algorithms", 540, 600, 4), C("Databases", 600, 660, 4),
                             C("Operating_Systems", 540, 660, 11)};
    CHECK(optimalCountExhaustive(v) == 2);
    CHECK(minClassroomsExhaustive(v) == 2);
    CHECK(optimalEnrollmentsExhaustive(v) == 11);
    std::vector<Course> overlapAll = {C("A", 0, 50, 1), C("B", 1, 50, 1), C("C", 2, 50, 1), C("D", 3, 50, 1), C("E", 4, 50, 1)};
    CHECK(minClassroomsExhaustive(overlapAll) == 5);
    CHECK(optimalCountExhaustive(overlapAll) == 1);
}

// ============================================================================
// Randomized comparison with the exhaustive solutions (n <= 5)
// ============================================================================

TEST_CASE("random small inputs (n <= 5) match the exhaustive optimum and are valid") {
    std::mt19937 rng(2026);                       // fixed seed -> reproducible
    int tested = 0;
    for (int it = 0; it < 3000; ++it) {
        int n = it % 6;                            // 0..5 courses
        auto in = randomCourses(rng, n, 60, 10);   // narrow range -> many overlaps and ties
        auto s = sortedCopy(in);

        auto cnt = selectCourses(s);
        CHECK(validSelection(cnt, in));
        CHECK(cnt.size() == optimalCountExhaustive(in));

        auto rooms = allocateClassrooms(in);
        CHECK(validAllocation(rooms, in));
        CHECK(rooms.size() == minClassroomsExhaustive(in));

        auto dp = selectCoursesDP(s);
        auto dpb = selectCoursesDPBinary(s);
        CHECK(validSelection(dp, in));
        CHECK(totalEnrollments(dp) == optimalEnrollmentsExhaustive(in));
        CHECK(ids(dp) == ids(dpb));
        ++tested;
    }
    CHECK(tested == 3000);
}

// ============================================================================
// Larger inputs: validity only (no exhaustive solution)
// ============================================================================

TEST_CASE("random large inputs (6..300 courses): outputs are valid and consistent") {
    std::mt19937 rng(777);
    for (int it = 0; it < 200; ++it) {
        int n = 6 + static_cast<int>(rng() % 295);
        auto in = randomCourses(rng, n, 1440, 200);
        auto s = sortedCopy(in);

        CHECK(validSelection(selectCourses(s), in));

        auto rooms = allocateClassrooms(in);
        CHECK(validAllocation(rooms, in));
        CHECK(rooms.size() == maxSimultaneous(in));   // meets the lower bound -> minimal

        auto dp = selectCoursesDP(s);
        CHECK(validSelection(dp, in));
        CHECK(computePredecessorsLinear(s) == computePredecessorsBinary(s));
        CHECK(ids(dp) == ids(selectCoursesDPBinary(s)));
        // DP is at least as good as the greedy COUNT selection on enrollments.
        CHECK(totalEnrollments(dp) >= totalEnrollments(selectCourses(s)));
    }
}
