#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DESCRIPTION_LENGTH 128
#define MAX_LINE_LENGTH 256
#define INITIAL_TASK_LENGTH 8
#define FILE_NAME "todo.txt"

typedef enum
{
    SUCCESS,
    FAILURE,
} Result;

typedef enum
{
    IN_PROGRESS,
    ABANDONED,
    DONE
} Status;

typedef struct
{
    int id;
    char description[MAX_DESCRIPTION_LENGTH];
    Status status;
} Task;

typedef struct
{
    Task *items;
    size_t count;
    size_t capacity;
    int next_id;
} Tasks;

Tasks init_tasks(void);
Result add_task(Tasks *tasks, const char *description);
Result mark_task_as_done(Tasks *tasks, int id);
Result remove_task(Tasks *tasks, int id);
void list_tasks(const Tasks *tasks);
Result append_task(Tasks *tasks, Task task);
Result save_tasks(Tasks *tasks);

void strip_newline(char *str);
const char *display_status(Status status);

int main(void)
{
    Tasks tasks = init_tasks();

    char command[20];
    do
    {
        printf("Type a command: \n");
        printf("q: quit \n");
        printf("a: add new task \n");
        printf("d: mark task as done \n");
        printf("r: remove task \n");
        printf("l: list all tasks \n");

        char *command_result = fgets(command, sizeof(command), stdin);

        if (command_result == NULL)
        {
            fprintf(stderr, "Failed to read command\n");
            continue;
        }

        strip_newline(command);

        if (command[0] == 'a')
        {
            printf("Type the task's description: \n");

            char description[MAX_DESCRIPTION_LENGTH];

            char *desc_result = fgets(description, sizeof(description), stdin);

            if (desc_result == NULL)
            {
                fprintf(stderr, "Failed to read task's description\n");
                continue;
            }

            strip_newline(description);

            if (add_task(&tasks, description) == FAILURE)
            {
                fprintf(stderr, "Failed to add task\n");
            }
        }
        else if (command[0] == 'l')
        {
            list_tasks(&tasks);
        }
        else if (command[0] == 'd')
        {
            list_tasks(&tasks);

            printf("Type the task's id: \n");

            char id_str[10];
            char *id_result = fgets(id_str, sizeof(id_str), stdin);

            if (id_result == NULL)
            {
                fprintf(stderr, "Failed to read task's id\n");
                continue;
            }

            strip_newline(id_str);

            char *endptr;
            long numeric_id = strtol(id_str, &endptr, 10);

            if (endptr == id_str || *endptr != '\0' || numeric_id <= 0)
            {
                fprintf(stderr, "Invalid task id\n");
                continue;
            }

            if (mark_task_as_done(&tasks, numeric_id) == FAILURE)
            {
                fprintf(stderr, "Failed to mark task as done\n");
            }
        }
        else if (command[0] == 'r')
        {
            list_tasks(&tasks);

            printf("Type the task's id: \n");

            char id_str[10];
            char *id_result = fgets(id_str, sizeof(id_str), stdin);

            if (id_result == NULL)
            {
                fprintf(stderr, "Failed to read task's id\n");
                continue;
            }

            strip_newline(id_str);

            char *endptr;
            long numeric_id = strtol(id_str, &endptr, 10);

            if (endptr == id_str || *endptr != '\0' || numeric_id <= 0)
            {
                fprintf(stderr, "Invalid task id\n");
                continue;
            }

            if (remove_task(&tasks, numeric_id) == FAILURE)
            {
                fprintf(stderr, "Failed to remove task\n");
            }
        }

    } while (command[0] != 'q');

    free(tasks.items);

    return EXIT_SUCCESS;
}

Tasks init_tasks(void)
{
    Task *items = calloc(INITIAL_TASK_LENGTH, sizeof(Task));

    if (items == NULL)
    {
        fprintf(stderr, "Failed to alloc memory to tasks\n");
        exit(EXIT_FAILURE);
    }

    Tasks tasks = {.count = 0,
                   .capacity = INITIAL_TASK_LENGTH,
                   .next_id = 1,
                   .items = items};

    // check if we have stored information
    FILE *fp = fopen(FILE_NAME, "r");

    if (fp == NULL)
    {
        return tasks;
    }

    char buffer[MAX_LINE_LENGTH];
    int tmp_status;
    int biggest_id = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        Task task;
        int fields = sscanf(buffer, "%d;%127[^;];%d",
                            &task.id,
                            task.description,
                            &tmp_status);

        if (fields == 3)
        {
            task.status = (Status)tmp_status;
            if (biggest_id <= task.id)
            {
                biggest_id = task.id;
            }
            if (append_task(&tasks, task) == FAILURE)
            {
                fclose(fp);
                return tasks;
            }
        }
    }

    tasks.next_id = biggest_id + 1;

    fclose(fp);

    return tasks;
}

Result add_task(Tasks *tasks, const char *description)
{
    Task task = {.id = tasks->next_id,
                 .status = IN_PROGRESS};

    snprintf(task.description, sizeof(task.description), "%s", description);

    if (append_task(tasks, task) == FAILURE)
    {
        return FAILURE;
    }

    tasks->next_id++;

    if (save_tasks(tasks) == FAILURE)
    {
        fprintf(stderr, "Failed to save tasks\n");
        return FAILURE;
    }

    return SUCCESS;
}

Result mark_task_as_done(Tasks *tasks, int id)
{
    Task *items = tasks->items;
    Task *task = NULL;

    for (size_t i = 0; i < tasks->count; i++)
    {
        if (items[i].id == id)
        {
            task = &items[i];
            break;
        }
    }

    if (task == NULL)
    {
        fprintf(stderr, "No task found with id: %d\n", id);
        return FAILURE;
    }

    task->status = DONE;

    if (save_tasks(tasks) == FAILURE)
    {
        fprintf(stderr, "Failed to save tasks\n");
        return FAILURE;
    }

    return SUCCESS;
}

Result remove_task(Tasks *tasks, int id)
{
    Task *items = tasks->items;
    for (size_t i = 0; i < tasks->count; i++)
    {
        if (items[i].id == id)
        {
            for (size_t j = i; j + 1 < tasks->count; j++)
            {
                items[j] = items[j + 1];
            }

            tasks->count--;

            if (save_tasks(tasks) == FAILURE)
            {
                fprintf(stderr, "Failed to save tasks\n");
                return FAILURE;
            }

            return SUCCESS;
        }
    }

    fprintf(stderr, "No task found with id: %d\n", id);
    return FAILURE;
}

void list_tasks(const Tasks *tasks)
{
    for (size_t i = 0; i < tasks->count; i++)
    {
        Task current_task = tasks->items[i];
        printf("%d|%s|%s\n",
               current_task.id, current_task.description, display_status(current_task.status));
    }
}

Result append_task(Tasks *tasks, Task task)
{
    if (tasks->count >= tasks->capacity)
    {
        size_t new_capacity = tasks->capacity * 2;

        Task *new_items = realloc(
            tasks->items,
            new_capacity * sizeof(*tasks->items));

        if (new_items == NULL)
        {
            fprintf(stderr, "Failed to resize tasks\n");
            return FAILURE;
        }

        tasks->capacity = new_capacity;
        tasks->items = new_items;
    }

    tasks->items[tasks->count] = task;
    tasks->count++;

    return SUCCESS;
}

Result save_tasks(Tasks *tasks)
{
    FILE *fp = fopen(FILE_NAME, "w");

    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open the file: %s\n", FILE_NAME);
        return FAILURE;
    }

    for (size_t i = 0; i < tasks->count; i++)
    {
        Task task = tasks->items[i];
        fprintf(fp, "%d;%s;%d\n", task.id, task.description, task.status);
    }

    fclose(fp);

    return SUCCESS;
}

void strip_newline(char *str)
{
    str[strcspn(str, "\n")] = '\0';
}

const char *display_status(Status status)
{
    switch (status)
    {
    case IN_PROGRESS:
        return "in progress";
    case ABANDONED:
        return "abandoned";
    case DONE:
        return "done";
    default:
        return "unknown";
    }
}
