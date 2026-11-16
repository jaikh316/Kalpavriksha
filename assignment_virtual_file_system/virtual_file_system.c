#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024
#define MAX_NAME 50
#define LINE_BUF 7000
#define COMMAND_BUF_SIZE 64
#define INITIAL_BLOCK_CAPACITY 4

typedef struct free_block_t {
    int block_index;
    struct free_block_t *next_node;
    struct free_block_t *prev_node;
} free_block_t;

typedef struct file_node_t {
    char name[MAX_NAME + 1]; /* Extra byte for the '\0' terminator. */
    bool is_directory;
    struct file_node_t *parent_directory;
    struct file_node_t *first_child;
    struct file_node_t *next_node;
    struct file_node_t *prev_node;

    int *block_pointers;
    int block_count;
    int block_capacity;
    int size_bytes;
} file_node_t;

typedef struct file_system_t {
    unsigned char virtual_disk[NUM_BLOCKS][BLOCK_SIZE];
    free_block_t *free_head;
    free_block_t *free_tail;
    int free_block_count;

    file_node_t *root;
    file_node_t *cwd;
} file_system_t;

static file_system_t file_system;

static void copy_name_safely(char *destination, const char *source) {
    if (!destination) {
        return;
    }
    if (!source) {
        destination[0] = '\0';
        return;
    }
    strncpy(destination, source, MAX_NAME);
    destination[MAX_NAME] = '\0';
}

static void append_free_block(int block_index) {
    free_block_t *node = (free_block_t *)malloc(sizeof(free_block_t));
    if (!node) {
        fprintf(stderr, "malloc failed in append_free_block\n");
        exit(EXIT_FAILURE);
    }
    node->block_index = block_index;
    node->next_node = NULL;
    node->prev_node = file_system.free_tail;
    if (!file_system.free_head) {
        file_system.free_head = node;
    } else {
        file_system.free_tail->next_node = node;
    }
    file_system.free_tail = node;
    file_system.free_block_count++;
}

int allocate_block_index(void) {
    if (!file_system.free_head) {
        return -1;
    }
    free_block_t *free_block = file_system.free_head;
    int block_index = free_block->block_index;
    file_system.free_head = free_block->next_node;
    if (file_system.free_head) {
        file_system.free_head->prev_node = NULL;
    } else {
        file_system.free_tail = NULL;
    }
    free(free_block);
    file_system.free_block_count--;
    memset(file_system.virtual_disk[block_index], 0, BLOCK_SIZE);
    return block_index;
}

void release_block_index(int block_index) {
    if (block_index < 0 || block_index >= NUM_BLOCKS) {
        return;
    }
    append_free_block(block_index);
}

int count_free_blocks(void) {
    return file_system.free_block_count;
}

static void ensure_block_capacity(file_node_t *file_node, int required_capacity) {
    if (!file_node) {
        return;
    }
    if (file_node->block_capacity >= required_capacity) {
        return;
    }
    int new_capacity = (file_node->block_capacity == 0)
                           ? INITIAL_BLOCK_CAPACITY
                           : file_node->block_capacity;
    while (new_capacity < required_capacity) {
        new_capacity *= 2;
    }
    int *resized_block_pointers = (int *)realloc(file_node->block_pointers, sizeof(int) * (size_t)new_capacity);
    if (!resized_block_pointers) {
        fprintf(stderr, "realloc failed\n");
        exit(EXIT_FAILURE);
    }
    file_node->block_pointers = resized_block_pointers;

    for (int index = file_node->block_capacity; index < new_capacity; ++index) {
        file_node->block_pointers[index] = -1;
    }
    file_node->block_capacity = new_capacity;
}

file_node_t *create_node(const char *name, bool is_directory) {
    file_node_t *file_node = (file_node_t *)malloc(sizeof(file_node_t));
    if (!file_node) {
        fprintf(stderr, "malloc failed in create_node\n");
        return NULL;
    }
    copy_name_safely(file_node->name, name ? name : "");
    file_node->is_directory = is_directory;
    file_node->parent_directory = NULL;
    file_node->first_child = NULL;
    file_node->next_node = file_node->prev_node = file_node;
    file_node->block_pointers = NULL;
    file_node->block_count = 0;
    file_node->block_capacity = 0;
    file_node->size_bytes = 0;
    return file_node;
}

static file_node_t *find_child(file_node_t *directory_node, const char *name) {
    if (!directory_node || !directory_node->first_child || !name) {
        return NULL;
    }
    file_node_t *first_child = directory_node->first_child;
    file_node_t *current_node = first_child;
    do {
        if (strcmp(current_node->name, name) == 0) {
            return current_node;
        }
        current_node = current_node->next_node;
    } while (current_node != first_child);
    return NULL;
}

static void insert_child(file_node_t *directory_node, file_node_t *node) {
    if (!directory_node || !node) {
        return;
    }
    node->parent_directory = directory_node;
    node->first_child = NULL;
    if (!directory_node->first_child) {
        directory_node->first_child = node;
        node->next_node = node->prev_node = node;
    } else {
        file_node_t *first_child = directory_node->first_child;
        file_node_t *last_child = first_child->prev_node;
        last_child->next_node = node;
        node->prev_node = last_child;
        node->next_node = first_child;
        first_child->prev_node = node;
    }
}

void remove_child_from_parent(file_node_t *node) {
    if (!node || !node->parent_directory) {
        return;
    }
    file_node_t *parent_directory = node->parent_directory;
    if (!parent_directory->first_child) {
        return;
    }

    file_node_t *first_child = parent_directory->first_child;

    if (first_child == node && node->next_node == node) {
        parent_directory->first_child = NULL;
        node->next_node = node->prev_node = node;
        node->parent_directory = NULL;
        return;
    }

    file_node_t *current_node = first_child;
    do {
        if (current_node == node) {
            current_node->prev_node->next_node = current_node->next_node;
            current_node->next_node->prev_node = current_node->prev_node;
            if (parent_directory->first_child == node) {
                parent_directory->first_child = node->next_node;
            }
            node->next_node = node->prev_node = node;
            node->parent_directory = NULL;
            return;
        }
        current_node = current_node->next_node;
    } while (current_node != first_child);
}

void free_file_blocks(file_node_t *file_node) {
    if (!file_node || file_node->is_directory) {
        return;
    }
    for (int index = 0; index < file_node->block_count; ++index) {
        int block_index = file_node->block_pointers[index];
        if (block_index >= 0 && block_index < NUM_BLOCKS) {
            memset(file_system.virtual_disk[block_index], 0, BLOCK_SIZE);
            release_block_index(block_index);
        }
        file_node->block_pointers[index] = -1;
    }
    file_node->block_count = 0;
    file_node->size_bytes = 0;
}

void destroy_node(file_node_t *node) {
    if (!node) {
        return;
    }
    if (!node->is_directory) {
        free_file_blocks(node);
        free(node->block_pointers);
    }
    free(node);
}

static void free_directory_tree(file_node_t *node) {
    if (!node) {
        return;
    }
    file_node_t *first_child = node->first_child;
    if (first_child) {
        file_node_t *current_node = first_child;
        do {
            file_node_t *next_node = current_node->next_node;
            free_directory_tree(current_node);
            current_node = next_node;
        } while (current_node != first_child);
    }
    if (!node->is_directory) {
        for (int index = 0; index < node->block_count; ++index) {
            int block_index = node->block_pointers[index];
            if (block_index >= 0 && block_index < NUM_BLOCKS) {
                memset(file_system.virtual_disk[block_index], 0, BLOCK_SIZE);
                release_block_index(block_index);
            }
        }
        free(node->block_pointers);
    }
    free(node);
}

void initialize_file_system(void) {
    file_system.free_head = file_system.free_tail = NULL;
    file_system.free_block_count = 0;
    for (int block_index = 0; block_index < NUM_BLOCKS; ++block_index) {
        append_free_block(block_index);
    }

    file_system.root = create_node("/", true);
    if (!file_system.root) {
        fprintf(stderr, "Failed to allocate root\n");
        exit(EXIT_FAILURE);
    }
    file_system.root->parent_directory = NULL;
    file_system.root->first_child = NULL;
    file_system.root->next_node = file_system.root->prev_node = file_system.root;
    file_system.cwd = file_system.root;

    for (int block_index = 0; block_index < NUM_BLOCKS; ++block_index) {
        memset(file_system.virtual_disk[block_index], 0, BLOCK_SIZE);
    }
}

static void cleanup_file_system(void) {
    /* First release the directory tree. File blocks are returned to the
       free-block list during this traversal. */
    if (file_system.root) {
        file_node_t *first_child = file_system.root->first_child;
        if (first_child) {
            file_node_t *current_node = first_child;
            do {
                file_node_t *next_node = current_node->next_node;
                free_directory_tree(current_node);
                current_node = next_node;
            } while (current_node != first_child);
        }

        free(file_system.root->block_pointers);
        free(file_system.root);
        file_system.root = NULL;
        file_system.cwd = NULL;
    }

    /* Now release both the originally free blocks and blocks returned
       while destroying files. */
    free_block_t *current_free_block = file_system.free_head;
    while (current_free_block) {
        free_block_t *next_free_block = current_free_block->next_node;
        free(current_free_block);
        current_free_block = next_free_block;
    }

    file_system.free_head = NULL;
    file_system.free_tail = NULL;
    file_system.free_block_count = 0;
}

void command_mkdir(const char *name) {
    if (!name || name[0] == '\0') {
        printf("mkdir: missing name\n");
        return;
    }
    if (strlen(name) > MAX_NAME) {
        printf("mkdir: name too long\n");
        return;
    }
    if (find_child(file_system.cwd, name)) {
        printf("Name already exists in current directory.\n");
        return;
    }
    file_node_t *directory_node = create_node(name, true);
    if (!directory_node) {
        printf("mkdir: allocation failed\n");
        return;
    }
    insert_child(file_system.cwd, directory_node);
    printf("Directory '%s' created successfully.\n", name);
}

void command_create(const char *name) {
    if (!name || name[0] == '\0') {
        printf("create: missing name\n");
        return;
    }
    if (strlen(name) > MAX_NAME) {
        printf("create: name too long\n");
        return;
    }
    if (find_child(file_system.cwd, name)) {
        printf("Name already exists in current directory.\n");
        return;
    }
    file_node_t *file_node = create_node(name, false);
    if (!file_node) {
        printf("create: allocation failed\n");
        return;
    }
    insert_child(file_system.cwd, file_node);
    printf("File '%s' created successfully.\n", name);
}

void command_list_directory(void) {
    if (!file_system.cwd->first_child) {
        printf("(empty)\n");
        return;
    }
    file_node_t *first_child = file_system.cwd->first_child;
    file_node_t *current_node = first_child;
    do {
        printf("%s%s\n", current_node->name,
               current_node->is_directory ? "/" : "");
        current_node = current_node->next_node;
    } while (current_node != first_child);
}

static void print_path_recursive(file_node_t *file_node) {
    if (!file_node || file_node == file_system.root) {
        return;
    }
    print_path_recursive(file_node->parent_directory);
    printf("/%s", file_node->name);
}

void command_change_directory(const char *name) {
    if (!name || name[0] == '\0') {
        printf("cd: missing directory name\n");
        return;
    }

    if (strcmp(name, "/") == 0) {
        file_system.cwd = file_system.root;
    } else if (strcmp(name, ".") == 0) {
        /* Stay in the current directory. */
    } else if (strcmp(name, "..") == 0) {
        if (file_system.cwd->parent_directory) {
            file_system.cwd = file_system.cwd->parent_directory;
        }
    } else {
        file_node_t *target = find_child(file_system.cwd, name);
        if (!target || !target->is_directory) {
            printf("Directory not found.\n");
            return;
        }
        file_system.cwd = target;
    }

    printf("Moved to ");
    if (file_system.cwd == file_system.root) {
        printf("/");
    } else {
        print_path_recursive(file_system.cwd);
    }
    printf("\n");
}

void command_print_working_directory(void) {
    if (file_system.cwd == file_system.root) {
        printf("/\n");
        return;
    }
    print_path_recursive(file_system.cwd);
    printf("\n");
}
void command_write_file(const char *filename, const char *content) {
    if (!filename || filename[0] == '\0') {
        printf("write: missing filename\n");
        return;
    }

    file_node_t *file_node = find_child(file_system.cwd, filename);
    if (!file_node || file_node->is_directory) {
        printf("File not found.\n");
        return;
    }

    size_t content_length = content ? strlen(content) : 0;
    int required_block_count = (int)(
        (content_length + BLOCK_SIZE - 1) / BLOCK_SIZE
    );

    if (required_block_count > NUM_BLOCKS) {
        printf("File too large for single file limit.\n");
        return;
    }

    int available_block_count_after_overwrite = count_free_blocks() + file_node->block_count;
    if (required_block_count > available_block_count_after_overwrite) {
        printf("Disk full. Not enough free blocks.\n");
        return;
    }

    ensure_block_capacity(file_node, required_block_count);

    free_file_blocks(file_node);

    size_t bytes_written = 0;
    for (int block_number = 0; block_number < required_block_count;
         ++block_number) {
        int block_index = allocate_block_index();
        if (block_index < 0) {
            for (int index = 0; index < file_node->block_count; ++index) {
                int allocated_block_index = file_node->block_pointers[index];
                if (allocated_block_index >= 0 && allocated_block_index < NUM_BLOCKS) {
                    memset(file_system.virtual_disk[allocated_block_index], 0, BLOCK_SIZE);
                    release_block_index(allocated_block_index);
                    file_node->block_pointers[index] = -1;
                }
            }
            file_node->block_count = 0;
            file_node->size_bytes = 0;
            printf("Unable to allocate disk block.\n");
            return;
        }

        file_node->block_pointers[file_node->block_count++] = block_index;

        size_t remaining_bytes = content_length - bytes_written;
        size_t bytes_to_copy =
            remaining_bytes < BLOCK_SIZE ? remaining_bytes : BLOCK_SIZE;
        if (bytes_to_copy > 0) {
            memcpy(file_system.virtual_disk[block_index],
                   content + bytes_written, bytes_to_copy);
        }
        if (bytes_to_copy < BLOCK_SIZE) {
            memset(file_system.virtual_disk[block_index] + bytes_to_copy, 0,
                   BLOCK_SIZE - bytes_to_copy);
        }
        bytes_written += bytes_to_copy;
    }

    file_node->size_bytes = (int)content_length;
    printf("Data written successfully (size=%zu bytes).\n", content_length);
}

void command_read_file(const char *filename) {
    if (!filename || filename[0] == '\0') {
        printf("read: missing filename\n");
        return;
    }
    file_node_t *file_node = find_child(file_system.cwd, filename);
    if (!file_node || file_node->is_directory) {
        printf("File not found.\n");
        return;
    }
    if (file_node->block_count == 0 || file_node->size_bytes == 0) {
        printf("(empty)\n");
        return;
    }

    int remaining_bytes = file_node->size_bytes;
    for (int index = 0; index < file_node->block_count; ++index) {
        int block_index = file_node->block_pointers[index];
        int bytes_to_print =
            remaining_bytes < BLOCK_SIZE ? remaining_bytes : BLOCK_SIZE;
        if (bytes_to_print > 0) {
            fwrite(file_system.virtual_disk[block_index], 1,
                   (size_t)bytes_to_print, stdout);
        }
        remaining_bytes -= bytes_to_print;
    }
    printf("\n");
}

void command_delete_file(const char *filename) {
    if (!filename || filename[0] == '\0') {
        printf("delete: missing filename\n");
        return;
    }
    file_node_t *file_node = find_child(file_system.cwd, filename);
    if (!file_node) {
        printf("File not found.\n");
        return;
    }
    if (file_node->is_directory) {
        printf("Target is a directory. Use rmdir to remove directories.\n");
        return;
    }

    remove_child_from_parent(file_node);
    destroy_node(file_node);
    printf("File deleted successfully.\n");
}

void command_remove_directory(const char *dirname) {
    if (!dirname || dirname[0] == '\0') {
        printf("rmdir: missing name\n");
        return;
    }
    file_node_t *directory_node = find_child(file_system.cwd, dirname);
    if (!directory_node) {
        printf("Directory not found.\n");
        return;
    }
    if (!directory_node->is_directory) {
        printf("Not a directory.\n");
        return;
    }
    if (directory_node->first_child) {
        printf("Directory not empty. Remove files first.\n");
        return;
    }

    remove_child_from_parent(directory_node);
    destroy_node(directory_node);
    printf("Directory removed successfully.\n");
}

void command_show_disk_usage(void) {
    int free_block_count = count_free_blocks();
    int used_block_count = NUM_BLOCKS - free_block_count;
    double disk_usage_percent =
        ((double)used_block_count / (double)NUM_BLOCKS) * 100.0;
    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", used_block_count);
    printf("Free Blocks: %d\n", free_block_count);
    printf("Disk Usage: %.2f%%\n", disk_usage_percent);
}

void trim_whitespace(char *text) {
    if (!text) {
        return;
    }
    size_t text_length = strlen(text);
    size_t index = 0;
    while (index < text_length && isspace((unsigned char)text[index])) {
        index++;
    }
    if (index > 0) {
        memmove(text, text + index, text_length - index + 1);
    }
    text_length = strlen(text);
    while (text_length > 0 && isspace((unsigned char)text[text_length - 1])) {
        text[text_length - 1] = '\0';
        text_length--;
    }
}

void handle_command_line(char *input) {
    if (!input) {
        return;
    }
    char line[LINE_BUF];

    strncpy(line, input, LINE_BUF - 1);
    line[LINE_BUF - 1] = '\0';
    trim_whitespace(line);
    if (line[0] == '\0') {
        return;
    }

    char command_name[COMMAND_BUF_SIZE] = "";
    int command_position = 0;
    while (line[command_position] &&
           !isspace((unsigned char)line[command_position]) &&
           command_position < (int)(sizeof(command_name) - 1)) {
        command_name[command_position] = line[command_position];
        command_position++;
    }
    command_name[command_position] = '\0';

    char *arguments = line + command_position;
    while (*arguments && isspace((unsigned char)*arguments)) {
        arguments++;
    }

    if (strcmp(command_name, "mkdir") == 0) {
        command_mkdir(arguments);
    } else if (strcmp(command_name, "create") == 0) {
        command_create(arguments);
    } else if (strcmp(command_name, "ls") == 0) {
        command_list_directory();
    } else if (strcmp(command_name, "cd") == 0) {
        command_change_directory(arguments);
    } else if (strcmp(command_name, "pwd") == 0) {
        command_print_working_directory();
    } else if (strcmp(command_name, "write") == 0) {

        char file_name[256] = "";
        char content[LINE_BUF];
        content[0] = '\0';

        int index = 0;
        while (*arguments && !isspace((unsigned char)*arguments) &&
               *arguments != '"' && index < (int)sizeof(file_name) - 1) {
            file_name[index++] = *arguments++;
        }
        file_name[index] = '\0';

        while (*arguments && isspace((unsigned char)*arguments)) {
            arguments++;
        }

        if (file_name[0] == '\0') {
            printf("write: missing filename\n");
            return;
        }

        if (*arguments == '\'' || *arguments == '"') {
            char quote_character = *arguments;
            arguments++;
            char *closing_quote = strchr(arguments, quote_character);
            if (!closing_quote) {
                printf("write: missing closing quote\n");
                return;
            }

            size_t copy_length = (size_t)(closing_quote - arguments);
            if (copy_length >= sizeof(content)) {
                copy_length = sizeof(content) - 1;
            }
            memcpy(content, arguments, copy_length);
            content[copy_length] = '\0';

            arguments = closing_quote + 1;
            while (*arguments && isspace((unsigned char)*arguments)) {
            arguments++;
        }
            if (*arguments != '\0') {
                printf("write: unexpected text after closing quote\n");
                return;
            }
        } else {
            strncpy(content, arguments, sizeof(content) - 1);
            content[sizeof(content) - 1] = '\0';
        }

        command_write_file(file_name, content);
    } else if (strcmp(command_name, "read") == 0) {
        command_read_file(arguments);
    } else if (strcmp(command_name, "delete") == 0) {
        command_delete_file(arguments);
    } else if (strcmp(command_name, "rmdir") == 0) {
        command_remove_directory(arguments);
    } else if (strcmp(command_name, "df") == 0) {
        command_show_disk_usage();
    } else if (strcmp(command_name, "exit") == 0) {
        printf("Memory released. Exiting program...\n");
        cleanup_file_system();
        exit(0);
    } else {
        printf("Unknown command: %s\n", command_name);
    }
}

int main(void) {
    initialize_file_system();
    printf("Compact VFS - ready. Type 'exit' to quit.\n");

    char line[LINE_BUF];

    while (1) {
        if (file_system.cwd == file_system.root) {
            printf("/ > ");
        } else {
            print_path_recursive(file_system.cwd);
            printf(" > ");
        }

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        line[strcspn(line, "\n")] = '\0';
        handle_command_line(line);
    }
    cleanup_file_system();
    return 0;
}
