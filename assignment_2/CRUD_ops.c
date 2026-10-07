#include <stdio.h>
#define FILE_NAME "users.txt"
#define TEMP_FILE "temp.txt"
#define NAME_SIZE 50

struct User {
    unsigned int id;
    char name[NAME_SIZE];
    unsigned short age;
};

int createFile() {
    FILE *fileptr = fopen(FILE_NAME, "a");
    if(fileptr == NULL) {
        printf("Error creating file!\n");
        return 0;
    }
    fclose(fileptr);
    return 1;
}

int idExists(unsigned int id) {
    FILE *fileptr;
    struct User user;
    fileptr = fopen(FILE_NAME, "r");
    if(fileptr == NULL) {
        return -1;
    }
    while(fscanf(fileptr, "%u,%49[^,],%hu", &user.id, user.name, &user.age) == 3) {
        if(user.id == id) {
            fclose(fileptr);
            return 1;
        }
    }
    fclose(fileptr);
    return 0;
}
int readName(char name[]) {
    int idx = 0;
    int character;
    if(fgets(name, NAME_SIZE, stdin) == NULL) {
        return 0;
    }
    while(name[idx] != '\0') {
        if(name[idx] == '\n') {
            name[idx] = '\0';
            return 1;
        }
        idx++;
    }

    while((character = getchar()) != '\n' && character != EOF) {
    }
    return 1;
}
void addUser() {
    struct User user;
    printf("\nEnter user ID: ");
    if(scanf("%u", &user.id) != 1) {
        printf("Invalid ID!\n");
        while(getchar() != '\n') {
        }
        return;
    }
    int result;
    result = idExists(user.id);
    if(result==1) {
        printf("ID already exists!\n");
        return;
    }
    if(result == -1) {
        printf("Error opening file!\n");
        return;
    }
    while(getchar()!= '\n') {
    }
    printf("Enter Name: ");
    if(!readName(user.name)) {
        printf("Error reading the name!\n");
        return;
    }
    printf("Enter Age: ");
    if(scanf("%hu", &user.age) != 1) {
        printf("Invalid age!\n");
        while(getchar() !='\n') {
        }
        return;
    }

    FILE *fileptr;
    fileptr = fopen(FILE_NAME, "a");
    if(fileptr == NULL) {
        printf("Error opening file!\n");
        return;
    }
    if(fprintf(fileptr, "%u,%s,%hu\n", user.id, user.name, user.age)<0) {
        printf("Error in writing to the file!\n");
        fclose(fileptr);
        return;
    }
    fclose(fileptr);
    printf("User added successfully!\n");
}

void displayUsers() {
    struct User user;
    FILE *fileptr;
    fileptr = fopen(FILE_NAME, "r");
    if(fileptr == NULL) {
        printf("Error opening file!\n");
        return;
    }

    printf("\n------ User Records ------\n");
    printf("ID\tName\t\tAge\n");
    while(fscanf(fileptr, "%u,%49[^,],%hu", &user.id, user.name, &user.age) == 3) {
        printf("%u\t%-15s%hu\n", user.id, user.name, user.age);
    }
    fclose(fileptr);
}

void updateUser() {
    FILE *fileptr, *temp;
    struct User user;
    unsigned int id;
    int userFound = 0;

    printf("\nEnter ID to update: ");
    if(scanf("%u", &id) != 1) {
        printf("Invalid ID!\n");
        while(getchar() != '\n') {
        }
        return;
    }
    fileptr = fopen(FILE_NAME, "r");
    if(fileptr == NULL) {
        printf("Error opening file!\n");
        return;
    }
    temp = fopen(TEMP_FILE, "w");
    if(temp==NULL) {
        printf("Error in opening the temporary file!!\n");
        fclose(fileptr);
        return;
    }

    while(fscanf(fileptr, "%u,%49[^,],%hu", &user.id, user.name, &user.age)== 3) {
        if (user.id == id) {
            userFound = 1;
            while(getchar() != '\n') {
            }
            printf("Enter new Name: ");
            if(!readName(user.name)) {
                printf("Error reading name!\n");
                fclose(fileptr);
                fclose(temp);
                return;
            }
            printf("Enter new Age: ");
            if(scanf("%hu", &user.age) != 1) {
                printf("Invalid age!\n");

                while(getchar() != '\n') {
                }
                fclose(fileptr);
                fclose(temp);
                return;
            }
        }
        if(fprintf(temp, "%u,%s,%hu\n", user.id, user.name, user.age)< 0) {
            printf("Error writing to the temporary file!\n");
            fclose(fileptr);
            fclose(temp);
            return;
        }
    }

    fclose(fileptr);
    fclose(temp);
    if(remove(FILE_NAME) != 0) {
        printf("Error removing original file!\n");
        return;
    }

    if(rename(TEMP_FILE, FILE_NAME) != 0) {
        printf("Error renaming the temporary file!\n");
        return;
    }

    if(userFound) {
        printf("User updated successfully!\n");
    }
    else
        printf("User ID not found!\n");
}

void deleteUser() {
    FILE *fileptr, *temp;
    struct User user;
    unsigned int id;
    int userFound = 0;

    printf("\nEnter ID to delete: ");
    if(scanf("%u", &id)!= 1) {
        printf("Invalid ID!\n");
        while(getchar()!='\n') {
        }
        return;
    }
    fileptr = fopen(FILE_NAME, "r");
    if(fileptr == NULL) {
        printf("Error opening file!\n");
        return;
    }
    temp = fopen(TEMP_FILE, "w");
    if(temp==NULL) {
        printf("Error in opening the temporary file!!\n");
        fclose(fileptr);
        return;
    }

    while(fscanf(fileptr, "%u,%49[^,],%hu", &user.id, user.name, &user.age) == 3) {
        if(user.id == id) {
            userFound = 1;
            continue;
        }
        if(fprintf(temp, "%u,%s,%hu\n", user.id, user.name, user.age)< 0) {
            printf("Error writing to the temporary file!\n");
            fclose(fileptr);
            fclose(temp);
            return;
        }
    }
    fclose(fileptr);
    fclose(temp);
    if(remove(FILE_NAME) != 0) {
        printf("Error removing original file!\n");
        return;
    }
    if(rename(TEMP_FILE, FILE_NAME) != 0) {
        printf("Error renaming the temporary file!\n");
        return;
    }
    if(userFound) {
      printf("User deleted successfully!\n");
    }
    else {
      printf("User ID not found!\n");
    }
}

int main() {
    int choice;
    if(!createFile()) {
        return 0;
    }
    while (1) {
        printf("\n========== USER MANAGEMENT ==========\n");
        printf("1. Create / Add User\n");
        printf("2. Read / Display Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("=====================================\n");
        printf("Enter your choice: ");
        if(scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number between 1 and 5.\n");
            while(getchar() != '\n') {
            }
            continue;
        }

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