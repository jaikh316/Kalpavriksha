#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100
#define INITIAL_CAPACITY 2

struct Employee {
    int id;
    char name[50];
    char designation[50];
    int managerId;
    struct Employee *manager;
    struct Employee **reports;
    int reportCount;
    int reportCapacity;
};
struct Employee employees[MAX];
int employeeCount = 0;

int findEmployee(int id) {
    for(int i=0; i < employeeCount; i++) {
        if(employees[i].id == id) {
            return i;
        }
    }
    return -1;
}

int findCEO(void) {
    for(int i = 0; i < employeeCount; i++) {
        if(employees[i].managerId == -1) {
            return i;
        }
    }
    return -1;
}
int wouldCreateCycle(int employeeId, int managerId) {
    int currentId = managerId;

    while(currentId != -1) {
        if(currentId == employeeId) {
            return 1;
        }
        int index = findEmployee(currentId);
        if(index == -1) {
            return 1;
        }
        currentId = employees[index].managerId;
    }
    return 0;
}
void rebuildManagerPointers(void) {
    for(int i = 0; i < employeeCount; i++) {
        if(employees[i].managerId == -1) {
            employees[i].manager = NULL;
        } else {
            int index = findEmployee(employees[i].managerId);

            employees[i].manager = (index != -1) ? &employees[index] : NULL;
        }
    }
}

int reserveReports(struct Employee *employee, int requiredCapacity) {
    if(requiredCapacity <= employee->reportCapacity) {
        return 1;
    }

    int newCapacity = employee->reportCapacity == 0
                          ? INITIAL_CAPACITY
                          : employee->reportCapacity;

    while(newCapacity < requiredCapacity) {
        newCapacity *= 2;
    }
    struct Employee **newReports = realloc(
        employee->reports,
        (size_t)newCapacity * sizeof(*employee->reports)
    );

    if(newReports == NULL) {
        printf("Memory allocation failed.\n");
        return 0;
    }
    employee->reports = newReports;
    employee->reportCapacity = newCapacity;
    return 1;
}

int addReport(struct Employee *manager, struct Employee *report) {
    if (!reserveReports(manager, manager->reportCount + 1)) {
        return 0;
    }
    manager->reports[manager->reportCount++] = report;
    return 1;
}
int countDirectReports(struct Employee *employee) {
    return employee->reportCount;
}

int countUnder(struct Employee *employee) {
    if (employee == NULL) {
        return 0;
    }
    int total = 0;
    for(int i = 0; i < employee->reportCount; i++) {
        total += 1 + countUnder(employee->reports[i]);
    }
    return total;
}

void displayHierarchy(struct Employee *employee, int level) {
    if(employee == NULL) {
        return;
    }
    for(int i = 0; i < level; i++) {
        printf("    ");
    }
    printf("%d - %s - %s\n", employee->id, employee->name, employee->designation);

    for(int i = 0; i < employee->reportCount; i++) {
        displayHierarchy(employee->reports[i], level + 1);
    }
}

void addEmployee(void) {
    if(employeeCount >= MAX) {
        printf("Employee limit reached.\n");
        return;
    }
    int id, managerId;
    printf("Enter Employee ID: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        return;
    }
    if (id<=0 || findEmployee(id) != -1) {
        printf("Invalid or duplicate Employee ID.\n");
        return;
    }
    printf("Enter Reporting Manager ID (-1 for CEO): ");
    if (scanf("%d", &managerId) != 1) {
        printf("Invalid manager ID.\n");
        return;
    }

    struct Employee *manager = NULL;
    if(managerId == -1) {
        if (findCEO() != -1) {
            printf("A CEO already exists. Only one CEO is allowed.\n");
            return;
        }
    } 
    else {
        int managerIndex = findEmployee(managerId);
        if(managerIndex == -1) {
            printf("Reporting manager does not exist.\n");
            return;
        }

        if(wouldCreateCycle(id, managerId)) {
            printf("Invalid reporting relationship: cycle detected.\n");
            return;
        }

        manager = &employees[managerIndex];
    }

    int index = employeeCount;

    employees[index].id = id;
    employees[index].managerId = managerId;
    employees[index].manager = manager;
    employees[index].reports = NULL;
    employees[index].reportCount = 0;
    employees[index].reportCapacity = 0;

    printf("Enter Name: ");
    if (scanf(" %49[^\n]", employees[index].name) != 1) {
        printf("Invalid name.\n");
        return;
    }

    printf("Enter Designation: ");
    if (scanf(" %49[^\n]", employees[index].designation) != 1) {
        printf("Invalid designation.\n");
        return;
    }

    if(manager != NULL && !reserveReports(manager, manager->reportCount + 1)) {
        return;
    }
    employeeCount++;
    rebuildManagerPointers();
    if(managerId != -1) {
        manager = employees[findEmployee(managerId)].manager;
        manager = &employees[findEmployee(managerId)];

        employees[index].manager = manager;
        manager->reports[manager->reportCount++] = &employees[index];
    }
    printf("Employee added successfully.\n");
}

void removeEmployee(void) {
    int id;
    printf("Enter Employee ID to remove: ");
    if(scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        return;
    }
    int index = findEmployee(id);
    if(index == -1) {
        printf("Employee not found.\n");
        return;
    }
    if(employees[index].managerId == -1) {
        printf("CEO cannot be removed.\n");
        return;
    }

    int superiorId = employees[index].managerId;
    int siblingIndex = -1;
    int minimumReports = MAX + 1;

    for(int i = 0; i < employeeCount; i++) {
        if(i != index && employees[i].managerId == superiorId) {
            int reports = countDirectReports(&employees[i]);

            if(reports < minimumReports) {
                minimumReports = reports;
                siblingIndex = i;
            }
        }
    }

    int destinationId;
    if (siblingIndex != -1) {
        destinationId = employees[siblingIndex].id;
    } else {
        destinationId = superiorId;
    }

    int destinationIndex = findEmployee(destinationId);

    if (employees[index].reportCount > 0 && destinationIndex == -1) {
        printf("No valid destination for direct reports.\n");
        return;
    }

    if (employees[index].reportCount > 0) {
        struct Employee *destination = &employees[destinationIndex];

        if(!reserveReports(destination,destination->reportCount + employees[index].reportCount)) {
            return;
        }
    }

    if (employees[index].reportCount > 0) {
        struct Employee *destination = &employees[destinationIndex];

        for (int i = 0; i < employees[index].reportCount; i++) {
            struct Employee *report = employees[index].reports[i];
            destination->reports[destination->reportCount++] = report;
            report->managerId = destinationId;
        }
    }
    int superiorIndex = findEmployee(superiorId);

    if (superiorIndex != -1) {
        struct Employee *superior = &employees[superiorIndex];

        for (int i = 0; i < superior->reportCount; i++) {
            if (superior->reports[i] == &employees[index]) {
                memmove(
                    &superior->reports[i],
                    &superior->reports[i + 1],
                    (size_t)(superior->reportCount - i - 1) *
                        sizeof(*superior->reports)
                );
                superior->reportCount--;
                break;
            }
        }
    }

    free(employees[index].reports);
    for (int i = index; i < employeeCount - 1; i++) {
        employees[i] = employees[i + 1];
    }

    employeeCount--;
    rebuildManagerPointers();

    for (int i = 0; i < employeeCount; i++) {
        employees[i].reportCount = 0;
    }
    for (int i = 0; i < employeeCount; i++) {
        if (employees[i].managerId != -1) {
            int managerIndex = findEmployee(employees[i].managerId);

            if (managerIndex != -1) {
                addReport(&employees[managerIndex], &employees[i]);
            }
        }
    }
    printf("Employee removed successfully.\n");
    printf("Direct reports transferred to employee ID %d.\n",
           destinationId);
}

void displayEmployeeHierarchy(void) {
    int id;
    printf("Enter Employee ID: ");
    if(scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        return;
    }

    int index = findEmployee(id);

    if(index == -1) {
        printf("Employee not found.\n");
        return;
    }

    printf("\n===== EMPLOYEE HIERARCHY =====\n");
    displayHierarchy(&employees[index], 0);

    printf("Total Employees Under %d: %d\n", id, countUnder(&employees[index]));
}

void displayCompleteHierarchy(void) {
    printf("\n===== ORGANIZATION HIERARCHY =====\n");
    int ceoIndex = findCEO();
    if (ceoIndex == -1) {
        printf("No CEO found.\n");
        return;
    }
    displayHierarchy(&employees[ceoIndex], 0);
    printf("Total Employees: %d\n", employeeCount);
}

void freeAllReports(void) {
    for (int i = 0; i < employeeCount; i++) {
        free(employees[i].reports);
        employees[i].reports = NULL;
        employees[i].reportCount = 0;
        employees[i].reportCapacity = 0;
    }
}

int main(void) {
    int choice;
    do {
        printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        if(scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting.\n");
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
                break;

            default:
                printf("Invalid choice. Try again.\n");
        }

    } while (choice != 5);

    freeAllReports();
    return 0;
}
