#include "exam.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to create a new exam and link it properly in the day
struct Exam* CreateExam(int startTime, int endTime, const char* courseCode) {
    struct Exam* exam = (struct Exam*) malloc(sizeof(struct Exam));
    exam->startTime = startTime;
    exam->endTime = endTime;
    strcpy(exam->courseCode, courseCode);
    exam->next = NULL;

    return exam;
}


// exam printlemek icin helper fonction
void PrintExam(struct Exam* exam) {
    printf("%s -> %d, %d", exam->courseCode, exam->startTime, exam->endTime); 
}