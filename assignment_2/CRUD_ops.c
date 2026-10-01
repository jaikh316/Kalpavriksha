#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "users.txt"
#define TEMP_FILE "temp.txt"

struct User {
    int id;
    char name[50];
    int age;
};

void createFile() {
    FILE *fptr = fopen(FILE_NAME, "a");
    if(fptr == NULL) {
        printf("Error creating file!\n");
        return;
    }
    fclose(fptr);
}

int idExists(int id) {
    FILE *fptr;
    struct User user;
    fptr = fopen(FILE_NAME, "r");
    if(fptr == NULL) {
        return 0;
    }
    while(fscanf(fptr, "%d,%49[^,],%d", &user.id, user.name, &user.age) == 3) {
        if(user.id == id) {
            fclose(fptr);
            return 1;
        }
    }
    fclose(fptr);
    return 0;
}

void addUser() {
    struct User user;

    printf("\nEnter user ID: ");
    scanf("%d", &user.id);
    if(idExists(user.id)) {
        printf("ID already exists!\n");
        return;
    }
    printf("Enter Name: ");
    scanf(" %[^\n]", user.name);
    printf("Enter Age: ");
    scanf("%d", &user.age);

    FILE *fptr;
    fptr = fopen(FILE_NAME, "a");
    if(fptr == NULL) {
        printf("Error opening file!\n");
        return;
    }

    fprintf(fptr, "%d,%s,%d\n", user.id, user.name, user.age);
    fclose(fptr);
    printf("User added successfully!\n");
}

void displayUsers() {
    struct User user;
    FILE *fptr;
    fptr = fopen(FILE_NAME, "r");
    if(fptr == NULL) {
        printf("No users found!\n");
        return;
    }

    printf("\n------ User Records ------\n");
    printf("ID\tName\t\tAge\n");

    while(fscanf(fptr, "%d,%49[^,],%d", &user.id, user.name, &user.age) == 3) {
        printf("%d\t%-15s%d\n", user.id, user.name, user.age);
    }
    fclose(fptr);
}

void updateUser() {
    FILE *fptr, *temp;
    struct User user;
    int id;
    int user_found = 0;

    printf("\nEnter ID to update: ");
    scanf("%d", &id);
    fptr = fopen(FILE_NAME, "r");
    temp = fopen(TEMP_FILE, "w");

    if(fptr == NULL || temp == NULL) {
        printf("Error opening file!\n");

        if(fptr != NULL) {
          fclose(fptr);
        }
        if(temp != NULL) {
          fclose(temp);
        }
        return;
    }

    while(fscanf(fptr, "%d,%49[^,],%d", &user.id, user.name, &user.age) == 3) {
        if (user.id == id) {
            user_found = 1;
            printf("Enter new Name: ");
            scanf(" %[^\n]", user.name);
            printf("Enter new Age: ");
            scanf("%d", &user.age);
        }
        fprintf(temp, "%d,%s,%d\n", user.id, user.name, user.age);
    }

    fclose(fptr);
    fclose(temp);

    remove(FILE_NAME);
    rename(TEMP_FILE, FILE_NAME);

    if(user_found) {
        printf("User updated successfully!\n");
    }
    else
        printf("User ID not found!\n");
}

void deleteUser() {
    FILE *fptr, *temp;
    struct User user;
    int id;
    int user_found = 0;

    printf("\nEnter ID to delete: ");
    scanf("%d", &id);
    fptr = fopen(FILE_NAME, "r");
    temp = fopen(TEMP_FILE, "w");
    if(fptr == NULL || temp == NULL) {
        printf("Error opening file!\n");

        if(fptr != NULL)
            fclose(fptr);

        if(temp != NULL)
            fclose(temp);

        return;
    }

    while(fscanf(fptr, "%d,%49[^,],%d", &user.id, user.name, &user.age) == 3) {
        if(user.id == id) {
            user_found = 1;
            continue;
        }
        fprintf(temp, "%d,%s,%d\n", user.id, user.name, user.age);
    }
    fclose(fptr);
    fclose(temp);
    remove(FILE_NAME);
    rename(TEMP_FILE, FILE_NAME);

    if(user_found) {
      printf("User deleted successfully!\n");
    }
    else {
      printf("User ID not found!\n");
    }
}

int main() {
    int choice;
    createFile();
    while (1) {
        printf("\n========== USER MANAGEMENT ==========\n");
        printf("1. Create / Add User\n");
        printf("2. Read / Display Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("=====================================\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addUser();
                break;

            case 2:
                displayUsers();
                break;

            case 3:
                updateUser();
                break;

            case 4:
                deleteUser();
                break;

            case 5:
                printf("Program exited.\n");
                return 0;

            default:
                printf("Invalid choice! Try again.\n");
        }
    }
    return 0;
}