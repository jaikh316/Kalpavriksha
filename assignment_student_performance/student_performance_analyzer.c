#include <stdio.h>
#define NAME_SIZE 50
#define MAXIMUM_STUDENTS 100
#define SUBJECT_COUNT 3
struct Student {
    unsigned int rollNo;
    char name[NAME_SIZE];
    unsigned int marks[SUBJECT_COUNT];
};
int totalMarks(unsigned int marks[]) {
  int total = 0;
  for(int i=0; i<SUBJECT_COUNT; i++) {
      total += marks[i];
  }
  return total;
}
double averageMarks(int total, int numSubjects) {
    if(numSubjects <= 0) {
        return 0.0;
    }
    return (double)total / numSubjects;
}
char studentGrade(double average) {
    if(average >= 85) {
        return 'A';
    }
    else if(average >= 70) {
        return 'B';
    }
    else if(average >= 50) {
        return 'C';
    }
    else if(average >= 35) {
        return 'D';
    }
    else {
        return 'F';
    }
}
void performanceRating(char grade) {
  int stars = 0;
    switch(grade) {
        case 'A': stars = 5;
                  break;
        case 'B': stars = 4;
                  break;
        case 'C': stars = 3;
                  break;
        case 'D': stars = 2;
                  break;              
        default : stars = 0;
                  break;
    }
    printf("Performance: ");
    for(int i=0; i < stars; i++) {
        printf("*");
    }
    printf("\n");
}
void printRollNums(struct Student students[], int index, int noOfStudents) {
  if(index == noOfStudents) return;
  printf("%u ", students[index].rollNo);
  printRollNums(students, index + 1, noOfStudents);
}
int main() {
    int noOfStudents;
    if(scanf("%d", &noOfStudents) != 1) {
        printf("Invalid input for number of students!\n");
        return 1;
    }
    if(noOfStudents <= 0 || noOfStudents > MAXIMUM_STUDENTS) {
        printf("Invalid number of students! Enter number between 1 - 100\n");
        return 1;
    }
    struct Student students[MAXIMUM_STUDENTS];
    for(int i=0; i < noOfStudents; i++) {
      if(scanf("%u", &students[i].rollNo) != 1) {
          printf("Invalid roll number!\n");
          return 1;
      }
      if(scanf("%49s", students[i].name) != 1) {
          printf("Invalid name!\n");
          return 1;
      }
      for(int j=0; j<SUBJECT_COUNT; j++) {
        int mark;
        if(scanf("%d", &mark) != 1) {
            printf("Invalid marks!\n");
            return 1;
        }
        if(mark < 0 || mark > 100) {
            printf("Invalid marks! Marks should be between 0 - 100\n");
            return 1;
        }
        students[i].marks[j] = mark;
      }
    }
    for(int i=0; i < noOfStudents; i++) {
        int total = totalMarks(students[i].marks);
        double average = averageMarks(total, SUBJECT_COUNT);
        char grade = studentGrade(average);
    
        printf("Roll: %u\n", students[i].rollNo);
        printf("Name: %s\n", students[i].name);
        printf("Total: %d\n", total);
        printf("Average: %.2f\n", average);
        printf("Grade: %c\n", grade);
        if(average < 35) {
          continue;
        }
        performanceRating(grade);
    }
    printf("List of Roll Numbers (via recursion): ");
    printRollNums(students, 0, noOfStudents);
    printf("\n");
    return 0;
}