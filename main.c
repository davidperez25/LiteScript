#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_LINES 1000
#define MAX_FUNCTIONS 1000


typedef struct {
    char name[64];
    int value;
} var;

var variables[2048];

typedef enum {
    CMD_INTEGER,
    CMD_CHAR,
    CMD_THROW,
    CMD_SET,
    CMD_ADD,
    CMD_SUBTRACT,
    CMD_PRINT,
    CMD_SLEEP,
    CMD_MULTIPLY,
    CMD_NEWLINE,
    CMD_GOTO,
    CMD_GETLINE,
    CMD_ALLOCATE,
    CMD_FUNCTION,
    CMD_EXIT,
    CMD_WHILE,
    CMD_IF,
    CMD_FOR,
    CMD_UNKNOWN
} cmdtyp;

int tempvars = 0;

char* funcnames[MAX_FUNCTIONS] = {""};
char* funcends[MAX_FUNCTIONS] = {0};
char* funcstarts[MAX_FUNCTIONS] = {0};
int numfunc = 0;

cmdtyp gettyp(const char* token);

void toalpha(char *str) {
    int i, j;
    for (i = 0, j = 0; str[i] != '\0'; i++) {
        if (isalpha((unsigned char) str[i])) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

cmdtyp gettyp(const char* token) {
    //toalpha(token);
    if (strcmp(token, "int") == 0) return CMD_INTEGER;
    if (strcmp(token, "char") == 0) return CMD_CHAR;
    if (strcmp(token, "throw") == 0) return CMD_THROW;
    if (strcmp(token , "put") == 0) return CMD_SET;
    if (strcmp(token , "add") == 0) return CMD_ADD;
    if (strcmp(token , "sub") == 0) return CMD_SUBTRACT;
    if (strcmp(token , "print") == 0) return CMD_PRINT;
    if (strcmp(token , "sleep") == 0) return CMD_SLEEP;
    if (strcmp(token, "mult") == 0) return CMD_MULTIPLY;
    if (strcmp(token, "newl") == 0) return CMD_NEWLINE;
    if (strcmp(token, "goto") == 0) return CMD_GOTO;
    if (strcmp(token, "getline") == 0) return CMD_GETLINE;
    if (strcmp(token, "allocate") == 0) return CMD_ALLOCATE;
    if (strcmp(token, "exit") == 0) return CMD_EXIT;
    if (strcmp(token, "function") == 0) return CMD_FUNCTION;
    if (strcmp(token, "for") == 0) return CMD_FOR;
    if (strcmp(token, "if") == 0) return CMD_IF;
    if (strcmp(token, "while") == 0) return CMD_WHILE;
    return CMD_UNKNOWN;
}

void sv(var* vars, int* count, char* name, int value) {
    for (int i = 0; i < *count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            vars[i].value = value;
            return;
        }
    }
    strcpy(vars[*count].name, name);
    vars[*count].value = (int)value;
    (*count)++;
}

int gvv(var* vars, int count, char* name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].value;
        }
    }
    fprintf(stderr, "Error: Variable %s not found.\n", name);
    exit(1);
}

void pv(var *vars, int count, char* name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            int adjusted_value = vars[i].value - 100000;
            if (adjusted_value >= 32 && adjusted_value <= 126) {
                printf("%c", (char)adjusted_value);
            } else {
                printf("%d", vars[i].value);
            }
            return;
        }
    }
    fprintf(stderr, "Error: Variable %s not found.\n", name);
}


char* ctv(var* vars, int* count, char* var1, char* var2) {
    char tempvarname[20];
    sprintf(tempvarname, "Tempvar%d", *count);

    sv(vars, count, tempvarname, atoi(var2));

    char* str = (char*)malloc((strlen(tempvarname) + 1) * sizeof(char));
    strcpy(str, tempvarname);

    return str;
}

bool isNumeric(char* count) {
    bool ISNUMERIC = true;
    for (int i = 0; i < strlen(count); i++) {
        if (!isdigit(count[i])) {
            ISNUMERIC = false;
            break;
        }
    }
}

bool extractchar(const char *str, char *result) {
    const char *start = strchr(str, '\'');

    if (start) {
        start++;
        const char *end = strchr(start, '\'');
        if (end && end != start) {
            strncpy(result, start, end - start);
            result[end - start] = '\0';
            return true;
        }
    }
    return false;
}

void interpret(char** program, int numCommands, var *vars, int *count) {
    for (int ln = 0; ln < numCommands; ln++) {
        char *cmd = strdup(program[ln]);

        cmd[strcspn(cmd, "\n")] = '\0';

        if (strlen(cmd) == 0) {
            free(cmd);
            continue;
        }

        char *token = strtok(cmd, " ,");

        cmdtyp typ = gettyp(token);

        switch (typ) {
            case CMD_INTEGER: {
                char* vname = strtok(NULL, " ,=");
                if (vname) {
                    char* value = strtok(NULL, " ,=");

                    sv(vars, count, vname, atoi(value));
                }
                break;
            }
            case CMD_CHAR: {
                char* vname = strtok(NULL, " ,=");
                if (vname) {
                    char* fsq = strtok(NULL, " '");

                    if (fsq && fsq[0] != '\0') {
                        char value = fsq[3];
                        int ascii_value = (int)value + 100000;
                        sv(vars, count, vname, ascii_value);
                    }
                }
                break;
            }
            case CMD_THROW: {
                char *_cmd = strdup(program[ln]);

                if (_cmd == NULL) {
                    perror("Memory allocation failed.");
                    return 1;
                }



                char *src = _cmd;
                char *dst = _cmd;

                while (isspace((unsigned char)*src))
                    src++;

                while (*src) {
                    *dst = *src;
                    dst++;
                    src++;
                }
                *dst = '\0';

                for (int i = 0; i < 6; i++)
                    _cmd++;

                printf("%s", _cmd);

                free(_cmd);

                break;
            }
            case CMD_SET: {
                char* vname = strtok(NULL, " ");
                char* value_str = strtok(NULL, " ");
                if (vname && value_str) {
                    int value = atoi(value_str);
                    sv(vars, count, vname, value);
                }
                break;
            }
            case CMD_ADD: {
                char *var1 = strtok(NULL, " ,");
                char *var2 = strtok(NULL, " ,");
                if (var1 && var2) {
                    int result;

                    if (isdigit(var1[0]) || isdigit(var2[0]) || var1[0] == '-' || var2[0] == '-') {
                        char* tempvarname = ctv(vars, count, var1, var2);

                        result = gvv(vars, *count, var1) + gvv(vars, *count, tempvarname);

                        sv(vars, count, var1, result);
                        free(tempvarname);

                    } else {
                        result = gvv(vars, *count, var1) + gvv(vars, *count, var2);
                        sv(vars, count, var1, result);
                    }
                }
                break;
            }
            case CMD_PRINT: {
                char *vname = strtok(NULL, " ,");
                if (vname) {
                    if (isdigit(vname[0]) || vname[0] == '-') {
                        printf("%d", atoi(vname));
                    } else {
                        pv(vars, *count, vname);
                    }

                }
                break;
            }
            case CMD_MULTIPLY: {
                char *var1 = strtok(NULL, " ,");
                char *var2 = strtok(NULL, " ,");
                if (var1 && var2) {
                    int result;

                    if (isdigit(var1[0]) || isdigit(var2[0]) || var1[0] == '-' || var2[0] == '-') {
                        char* tempvarname = ctv(vars, count, var1, var2);

                        int result = gvv(vars, *count, var1) * gvv(vars, *count, tempvarname);

                        sv(vars, count, var1, result);
                        free(tempvarname);

                    } else {
                        result = gvv(vars, *count, var1) * gvv(vars, *count, var2);
                        sv(vars, count, var1, result);
                    }
                }
                break;
            }
            case CMD_NEWLINE: {
                char *count = strtok(NULL, " ");
                if (count) {
                    for (int i = 0; i < atoi(count); i++) {
                        printf("\n");
                    }
                }
                else {
                    printf("\n");
                }

                break;
            }

            case CMD_FUNCTION: {
                char *fname = strtok(NULL, " ");
                char *end = strtok(NULL, " to ");

                funcnames[numfunc] = fname;
                funcstarts[numfunc] = ln;
                funcends[numfunc] = atoi(end);
                ln = atoi(end);


                numfunc++;
                break;
            }

            case CMD_SLEEP: {
                char *count = strtok(NULL, " ");

                if (count && isNumeric(count)) {
                    unsigned int seconds = atoi(count);
                    sleep(seconds);
                } else {
                    fprintf(stderr, "Error: Wait time is not an integer or not provided!\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }
            case CMD_GETLINE: {
                char* vname = strtok(NULL, " ,");

                if (vname) {
                    sv(vars, count, vname, ln + 1);
                }
                break;
            }
            case CMD_IF: {
                char* condition = strtok(NULL, " ");
                char* end = strtok(NULL, " to ");
                if (gvv(vars, count, condition) >= 1) {

                } else {
                    ln = atoi(end);
                }

                break;
            }
            case CMD_GOTO: {
                char* vname = strtok(NULL, " ");

                if (vname) {
                    if (isNumeric(vname)) {
                        ln = atoi(vname) - 1;
                    } else {
                        ln = gvv(vars, count, vname) - 1;
                    }
                }


                break;
            }
            case CMD_EXIT: {
                exit(EXIT_SUCCESS);
                break;
            }

            case CMD_UNKNOWN:
            default:
                //fprintf(stderr, "Error: Unknown command %s\n", token);

                for (int lnn = 0; lnn < numCommands; lnn++) {
                    char *cmdd = strdup(program[lnn]);

                    if (token == funcnames[lnn])
                        ln = funcstarts[lnn] + 1;


                }
                break;
        }
        free(cmd);
    }
}

char ** rf(const char *filename, int *num_lines) {
    FILE *file_pointer;
    char **lines = NULL;
    char buffer[1024];
    int line_count = 0;

    file_pointer = fopen(filename, "r");

    if (file_pointer == NULL) {
        perror("Error opening file");
        return NULL;
    }

    lines = (char**) malloc(MAX_LINES * sizeof(char*));
    if (lines == NULL) {
        perror("Memory allocation error");
        fclose(file_pointer);
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), file_pointer) != NULL) {
        char *comment_pos = strstr(buffer, "//");
        if (comment_pos != NULL) {
            *comment_pos = '\0';
        }

        lines[line_count] = (char*) malloc(strlen(buffer) + 1);
        if (lines[line_count] == NULL) {
            perror("Memory allocation error");
            fclose(file_pointer);

            for (int i = 0; i < line_count; i++) {
                free(lines[i]);
            }
            free(lines);

            return NULL;
        }

        strcpy(lines[line_count], buffer);
        line_count++;

        if (line_count >= MAX_LINES) {
            break;
        }
    }
    fclose(file_pointer);

    *num_lines = line_count;
    return lines;
}

int main() {
    int vcount = 0;

    char **lines;
    int num_lines;

    lines = rf("main.lite", &num_lines);
    if (lines == NULL) {
        fprintf(stderr, "Error reading file\n");
        return 1;
    }

    interpret(lines, num_lines, variables, &vcount);

    for (int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }

    free(lines);
    return 0;
}
