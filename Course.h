#ifndef COURSE_H
#define COURSE_H

#include <string>

// One course meeting on a given day. Times are minutes from the start of the day.
struct Course {
    std::string id;          // unique English name, no spaces (e.g. Operating_Systems)
    int start;               // start time in minutes
    int finish;              // finish time in minutes (start < finish)
    long long enrollments;   // positive number of enrolled students (weight for ENROLLMENTS)
};

#endif // COURSE_H
