#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define NAME_SIZE 50
#define INITIAL_CAPACITY 4
#define DESIGNATION_SIZE 50

typedef struct {
    int id;
    char name[NAME_SIZE];
    char designation[DESIGNATION_SIZE];
    int managerId;
} Employee;

static Employee *employees = NULL;
static int employeeCount = 0;
static int employeeCapacity = 0;


int readInteger(const char *prompt, int *value) {
    char input[128];
    while (1) {
        char *endPointer;
        long parsedValue;
        printf("%s", prompt);
        if(fgets(input, sizeof(input), stdin) == NULL) {
            return 0;
        }
        errno = 0;
        parsedValue = strtol(input, &endPointer, 10);

        if(endPointer == input || errno == ERANGE || parsedValue < INT_MIN || parsedValue > INT_MAX) {
            printf("Invalid input. Please enter a valid integer.\n");
            continue;
        }
        while(*endPointer == ' ' || *endPointer == '\t' || *endPointer == '\n' || *endPointer == '\r') {
            endPointer++;
        }
        if(*endPointer != '\0') {
            printf("Invalid input. Please enter only an integer.\n");
            continue;
        }
        *value = (int)parsedValue;
        return 1;
    }
}

int readText(const char *prompt, char *destination, size_t destinationSize) {
    char input[256];
    while (1) {
        size_t length;
        printf("%s", prompt);
        if(fgets(input, sizeof(input), stdin)==NULL) {
            return 0;
        }
        length = strcspn(input, "\n");
        if(input[length]== '\n') {
            input[length] = '\0';
        } else {
            int character;
            while ((character = getchar()) != '\n' && character != EOF) {
            }
            printf("Input is too long. Please try again.\n");
            continue;
        }
        if(length==0) {
            printf("Input cannot be empty. Please try again.\n");
            continue;
        }
        if(length >= destinationSize) {
            printf("Input is too long (maximum %zu characters). Please try again.\n",
                   destinationSize - 1);
            continue;
        }

        memcpy(destination, input, length + 1);
        return 1;
    }
}

int findEmployeeIndex(int employeeId) {
    for (int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        if(employees[employeeIndex].id == employeeId) {
            return employeeIndex;
        }
    }
    return -1;
}

int findCEOIndex(void) {
    for (int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        if(employees[employeeIndex].managerId== -1) {
            return employeeIndex;
        }
    }
    return -1;
}

int reserveEmployees(int requiredCapacity) {
    int newCapacity;
    Employee *resizedEmployees;
    if(requiredCapacity <= employeeCapacity) {
        return 1;
    }

    newCapacity = employeeCapacity == 0 ? INITIAL_CAPACITY : employeeCapacity;
    while(newCapacity < requiredCapacity) {
        if(newCapacity >INT_MAX / 2) {
            printf("Cannot allocate space for more employees.\n");
            return 0;
        }
        newCapacity *= 2;
    }
    resizedEmployees = realloc(employees, (size_t)newCapacity * sizeof(*employees));
    if (resizedEmployees== NULL) {
        printf("Memory allocation failed. Employee was not added.\n");
        return 0;
    }
    employees = resizedEmployees;
    employeeCapacity = newCapacity;
    return 1;
}

int wouldCreateCycle(int employeeId, int proposedManagerId) {
    int currentManagerId = proposedManagerId;
    while (currentManagerId != -1) {
        int managerIndex;
        if(currentManagerId == employeeId) {
            return 1;
        }
        managerIndex = findEmployeeIndex(currentManagerId);
        if (managerIndex == -1) {
            return 1;
        }

        currentManagerId = employees[managerIndex].managerId;
    }
    return 0;
}

int countDirectReports(int managerId) {
    int directReportCount = 0;
    for(int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        if(employees[employeeIndex].managerId == managerId) {
            directReportCount++;
        }
    }
    return directReportCount;
}

int countEmployeesUnder(int managerId) {
    int totalCount = 0;
    for (int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        if(employees[employeeIndex].managerId == managerId) {
            totalCount += 1 + countEmployeesUnder(employees[employeeIndex].id);
        }
    }
    return totalCount;
}

void displayDirectReports(int managerId, int level);

void displayHierarchy(int managerId, int level) {
    for(int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        if(employees[employeeIndex].id == managerId) {
            int indentation;
            for (indentation = 0; indentation < level; indentation++) {
                printf("    ");
            }

            printf("%d - %s - %s\n", employees[employeeIndex].id, employees[employeeIndex].name, employees[employeeIndex].designation);
            displayDirectReports(employees[employeeIndex].id, level + 1);
            return;
        }
    }
}

void displayDirectReports(int managerId, int level) {
    for (int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        if (employees[employeeIndex].managerId == managerId) {
            int indentation;
            for (indentation = 0; indentation < level; indentation++) {
                printf("    ");
            }
            printf("%d - %s - %s\n", employees[employeeIndex].id, employees[employeeIndex].name, employees[employeeIndex].designation);
            displayDirectReports(employees[employeeIndex].id, level + 1);
        }
    }
}

int getValidatedEmployeeDetails(int *employeeId, int *managerId, char *name, char *designation) {
    int managerIndex;
    if(!readInteger("Enter Employee ID: ", employeeId)) {
        return 0;
    }
    if(*employeeId<= 0 || findEmployeeIndex(*employeeId) != -1) {
        printf("Invalid or duplicate Employee ID.\n");
        return -1;
    }
    if(!readInteger("Enter Reporting Manager ID (-1 for CEO): ", managerId)) {
        return 0;
    }
    if(*managerId== -1) {
        if(findCEOIndex() != -1) {
            printf("A CEO already exists. Only one CEO is allowed.\n");
            return -1;
        }
    } 
    else {
        managerIndex = findEmployeeIndex(*managerId);
        if(managerIndex== -1) {
            printf("Reporting manager does not exist.\n");
            return -1;
        }
        if (wouldCreateCycle(*employeeId, *managerId)) {
            printf("Invalid reporting relationship: cycle detected.\n");
            return -1;
        }
    }
    if(!readText("Enter Name: ", name, NAME_SIZE) ||
        !readText("Enter Designation: ", designation, DESIGNATION_SIZE)) {
        return 0;
    }
    return 1;
}

void addEmployee(void) {
    int employeeId;
    int managerId;
    int detailsStatus;
    Employee newEmployee;
    detailsStatus = getValidatedEmployeeDetails(&employeeId, &managerId, newEmployee.name, newEmployee.designation);
    if(detailsStatus != 1) {
        return;
    }
    if(!reserveEmployees(employeeCount + 1)) {
        return;
    }
    newEmployee.id = employeeId;
    newEmployee.managerId = managerId;
    employees[employeeCount] = newEmployee;
    employeeCount++;
    printf("Employee added successfully.\n");
}

int findRemovalDestination(int removedEmployeeIndex) {
    int superiorId = employees[removedEmployeeIndex].managerId;
    int siblingIndex = -1;
    int minimumDirectReports = INT_MAX;
    for (int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        if (employeeIndex != removedEmployeeIndex &&
            employees[employeeIndex].managerId == superiorId) {
            int directReportCount = countDirectReports(employees[employeeIndex].id);

            if (directReportCount < minimumDirectReports) {
                minimumDirectReports = directReportCount;
                siblingIndex = employeeIndex;
            }
        }
    }

    if (siblingIndex != -1) {
        return employees[siblingIndex].id;
    }
    return superiorId;
}

void removeEmployee(void) {
    int employeeId;
    int removedEmployeeIndex;
    int destinationId;
    int removedReportCount = 0;
    if(!readInteger("Enter Employee ID to remove: ", &employeeId)) {
        return;
    }
    removedEmployeeIndex = findEmployeeIndex(employeeId);
    if (removedEmployeeIndex == -1) {
        printf("Employee not found.\n");
        return;
    }

    if(employees[removedEmployeeIndex].managerId == -1) {
        printf("CEO cannot be removed.\n");
        return;
    }

    destinationId = findRemovalDestination(removedEmployeeIndex);
    removedReportCount = countDirectReports(employeeId);

    for(int employeeIndex = 0; employeeIndex < employeeCount; employeeIndex++) {
        if (employees[employeeIndex].managerId == employeeId) {
            employees[employeeIndex].managerId = destinationId;
        }
    }

    for(int employeeIndex = removedEmployeeIndex; employeeIndex < employeeCount - 1; employeeIndex++) {
        employees[employeeIndex] = employees[employeeIndex + 1];
    }

    employeeCount--;
    printf("Employee %d removed successfully.\n", employeeId);
    if (removedReportCount > 0) {
        printf("Direct reports transferred to employee ID %d.\n", destinationId);
    }
}

void displayEmployeeHierarchy(void) {
    int employeeId;
    int employeeIndex;
    if (!readInteger("Enter Employee ID: ", &employeeId)) {
        return;
    }
    employeeIndex = findEmployeeIndex(employeeId);
    if(employeeIndex == -1) {
        printf("Employee not found.\n");
        return;
    }

    printf("\n===== EMPLOYEE HIERARCHY =====\n");
    printf("%d - %s - %s\n", employees[employeeIndex].id, employees[employeeIndex].name, employees[employeeIndex].designation);
    displayDirectReports(employeeId, 1);
    printf("Total employees under %d: %d\n", employeeId, countEmployeesUnder(employeeId));
}

void displayCompleteHierarchy(void) {
    int ceoIndex = findCEOIndex();
    printf("\n===== ORGANIZATION HIERARCHY =====\n");
    if (ceoIndex == -1) {
        printf("No CEO found.\n");
        return;
    }

    displayHierarchy(employees[ceoIndex].id, 0);
    printf("Total employees: %d\n", employeeCount);
}

void displayMenu(void) {
    printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
    printf("1. Add Employee\n");
    printf("2. Remove Employee\n");
    printf("3. Display Employee Hierarchy\n");
    printf("4. Display Complete Hierarchy\n");
    printf("5. Exit\n");
}

int main(void) {
    int choice;
    while (1) {
        displayMenu();
        if(!readInteger("Enter your choice: ", &choice)) {
            printf("\nInput ended. Exiting program.\n");
            break;
        }

        switch (choice) {
            case 1:
                addEmployee();
                break;
            case 2:
                removeEmployee();
                break;
            case 3:
                displayEmployeeHierarchy();
                break;
            case 4:
                displayCompleteHierarchy();
                break;
            case 5:
                printf("Exiting program.\n");
                free(employees);
                return 0;
            default:
                printf("Invalid choice. Please choose from 1 to 5.\n");
        }
    }
    free(employees);
    return 0;
}
