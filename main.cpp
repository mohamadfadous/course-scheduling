#include <iostream>
#include <vector>

#include "Algorithms.h"

namespace {

void printCourse(const Course& c) {
    std::cout << c.id << ' ' << c.start << ' ' << c.finish << ' ' << c.enrollments << '\n';
}

void printSelection(const char* title, const std::vector<Course>& chosen) {
    std::cout << title << '\n' << chosen.size() << ' ' << totalEnrollments(chosen) << '\n';
    for (const Course& c : chosen) printCourse(c);
}

} // namespace

int main() {
    std::ios::sync_with_stdio(false);
    std::size_t n = 0;
    if (!(std::cin >> n)) n = 0;
    std::vector<Course> input(n);
    for (Course& c : input) std::cin >> c.id >> c.start >> c.finish >> c.enrollments;

    std::vector<Course> byFinish = input;          // copy; the original list stays as read
    mergeSort(byFinish);

    const std::vector<Course> countResult = selectCourses(byFinish);
    const std::vector<std::vector<Course>> rooms = allocateClassrooms(input);
    const std::vector<Course> enrollResult = selectCoursesDP(byFinish);

    printSelection("COUNT", countResult);

    std::cout << "ROOMS\n" << rooms.size() << '\n';
    for (std::size_t r = 0; r < rooms.size(); ++r)
        for (const Course& c : rooms[r]) {
            std::cout << (r + 1) << ' ';
            printCourse(c);
        }

    printSelection("ENROLLMENTS", enrollResult);
    return 0;
}
