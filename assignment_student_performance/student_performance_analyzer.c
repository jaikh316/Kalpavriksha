#include <stdio.h>
#define NAME_SIZE 50
#define MAXIMUM_STUDENTS 100
#define SUBJECT_COUNT 3
#define GRADE_A_MINIMUM 85
#define GRADE_B_MINIMUM 70
#define GRADE_C_MINIMUM 50
#define GRADE_D_MINIMUM 35
struct Student {
    unsigned int rollNo;
    char name[NAME_SIZE];
    unsigned int marks[SUBJECT_COUNT];
};
int inputStudent(struct Student *student) {
    if(scanf("%u", &student->rollNo) != 1) {
        printf("Invalid roll number!\n");
        return 0;
    }
    if(scanf("%49s", student->name) != 1) {
        printf("Invalid name!\n");
        return 0;
    }
    for(int j=0; j<SUBJECT_COUNT; j++) {
        int mark;
        if(scanf("%d", &mark) != 1) {
            printf("Invalid marks!\n");
            return 0;
        }
        if(mark < 0 || mark > 100) {
            printf("Invalid marks! Marks should be between 0 - 100\n");
            return 0;
        }
        student->marks[j] = mark;
    }
    return 1;
}
void displayStudentDetails(struct Student *student, int total, double average, char grade) {
        printf("Roll: %u\n", student->rollNo);
        printf("Name: %s\n", student->name);
        printf("Total: %d\n", total);
        printf("Average: %.2f\n", average);
        printf("Grade: %c\n", grade);
}
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
    if(average >= GRADE_A_MINIMUM) {
        return 'A';
    }
    else if(average >= GRADE_B_MINIMUM) {
        return 'B';
    }
    else if(average >= GRADE_C_MINIMUM) {
        return 'C';
    }
    else if(average >= GRADE_D_MINIMUM) {
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
void printRollNumbers(struct Student students[], int index, int noOfStudents) {
  if(index == noOfStudents) return;
  printf("%u ", students[index].rollNo);
  printRollNumbers(students, index + 1, noOfStudents);
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
      if(!inputStudent(&students[i])) {
            return 1;
        }
    }
    for(int i=0; i < noOfStudents; i++) {
        int total = totalMarks(students[i].marks);
        double average = averageMarks(total, SUBJECT_COUNT);
        char grade = studentGrade(average);
    
        displayStudentDetails(&students[i], total, average, grade);
        if(average < 35) {
          continue;
        }
        performanceRating(grade);
    }
    printf("List of Roll Numbers (via recursion): ");
    printRollNumbers(students, 0, noOfStudents);
    printf("\n");
    return 0;
}