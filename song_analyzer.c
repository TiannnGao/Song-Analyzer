#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emalloc.h"
#include "list.h"
#define MAX_LINE_LEN 1024
#define MAX_ARTIST_COUNT 10
#define MAX_WORD_LEN 50
typedef struct {
    char *data_filename;
    char *filter_field;
    char *filter_value;
    char *order_by_field;
    Order order_direction;
    int limit;
} CommandLineOptions;
void parse_command_line(int argc, char *argv[], CommandLineOptions *options);
node_t *parse_line_to_node(const char *line);
void read_data_file(const char *filename, node_t **list);
void print_list(const node_t *list, int limit, const char* order_by_field);
void free_list(node_t *list);
int main(int argc, char *argv[]) {
    CommandLineOptions options;
    node_t *list = NULL, *filtered_list = NULL;
    SortContext sort_context;
    parse_command_line(argc, argv, &options);
    read_data_file(options.data_filename, &list);
    if (options.filter_field != NULL && options.filter_value != NULL) {
        FilterContext filter_context = {options.filter_field, options.filter_value};
        filtered_list = filter_list(list, &filter_context);
    } else {
        filtered_list = list;
    }
    if (options.order_by_field != NULL) {
        if (strcmp(options.order_by_field, "STREAMS") == 0) {
            sort_context.compare = compare_streams;
        } else if (strcmp(options.order_by_field, "NO_SPOTIFY_PLAYLISTS") == 0) {
            sort_context.compare = compare_in_spotify_playlists;
        } else if (strcmp(options.order_by_field, "NO_APPLE_PLAYLISTS") == 0) {
            sort_context.compare = compare_in_apple_playlists;
        }
        sort_context.order = options.order_direction;
        node_t *sorted_list = NULL;
        while (filtered_list != NULL) {
            node_t *current = filtered_list;
            filtered_list = filtered_list->next;
            sorted_list = add_inorder(sorted_list, current, &sort_context);
        }
        filtered_list = sorted_list;
    }
    print_list(filtered_list, options.limit, options.order_by_field);
    if (filtered_list != list) {
        free_list(filtered_list);
    }
    free_list(list);
    return EXIT_SUCCESS;
}
void parse_command_line(int argc, char *argv[], CommandLineOptions *options) {
    options->data_filename = NULL;
    options->filter_field = NULL;
    options->filter_value = NULL;
    options->order_by_field = NULL;
    options->order_direction = ASCENDING; // 使用枚举类型，默认为升序
    options->limit = -1;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--data=", 7) == 0) {
            options->data_filename = argv[i] + 7;
        } else if (strncmp(argv[i], "--filter=", 9) == 0) {
            options->filter_field = argv[i] + 9;
        } else if (strncmp(argv[i], "--value=", 8) == 0) {
            options->filter_value = argv[i] + 8;
        } else if (strncmp(argv[i], "--order_by=", 11) == 0) {
            options->order_by_field = argv[i] + 11;
        } else if (strncmp(argv[i], "--order=", 8) == 0) {
            if (strcmp(argv[i] + 8, "ASC") == 0) {
                options->order_direction = ASCENDING;
            } else if (strcmp(argv[i] + 8, "DES") == 0) {
                options->order_direction = DESCENDING;
            } else {
                fprintf(stderr, "Invalid order direction: %s\n", argv[i] + 8);
                exit(EXIT_FAILURE);
            }
        } else if (strncmp(argv[i], "--limit=", 8) == 0) {
            options->limit = atoi(argv[i] + 8);
            if (options->limit < 0) {
                fprintf(stderr, "Invalid limit: must be a positive number\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    if (!options->data_filename || !options->filter_field || !options->filter_value || !options->order_by_field) {
        fprintf(stderr, "Missing required arguments.\n");
        exit(EXIT_FAILURE);
    }
}
node_t *parse_line_to_node(const char *line) {
    char *token;
    char track_name[MAX_WORD_LEN] = {0};
    char artist_names[MAX_WORD_LEN] = {0};
    int artist_count = 0, released_year = 0, released_month = 0, released_day = 0;
    int in_spotify_playlists = 0, in_apple_playlists = 0;
    long long streams = 0;
    char line_copy[MAX_LINE_LEN];
    strncpy(line_copy, line, MAX_LINE_LEN - 1);
    line_copy[MAX_LINE_LEN - 1] = '\0';
    token = strtok(line_copy, ",");
    if (token != NULL) {
        strncpy(track_name, token, MAX_WORD_LEN - 1);
    }
    token = strtok(NULL, ",");
    if (token != NULL) {
        strncpy(artist_names, token, MAX_WORD_LEN - 1);
    }
    token = strtok(NULL, ","); artist_count = token ? atoi(token) : 0;
    token = strtok(NULL, ","); released_year = token ? atoi(token) : 0;
    token = strtok(NULL, ","); released_month = token ? atoi(token) : 0;
    token = strtok(NULL, ","); released_day = token ? atoi(token) : 0;
    token = strtok(NULL, ","); in_spotify_playlists = token ? atoi(token) : 0;
    token = strtok(NULL, ","); streams = token ? atoll(token) : 0;
    token = strtok(NULL, ","); in_apple_playlists = token ? atoi(token) : 0;
    return new_node(track_name, artist_names, artist_count, released_year, released_month, released_day, in_spotify_playlists, streams, in_apple_playlists);
}
void read_data_file(const char *filename, node_t **list) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Unable to open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    char line[MAX_LINE_LEN];
    fgets(line, sizeof(line), file);
    node_t *tail = NULL;
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = 0;
        node_t *new_node = parse_line_to_node(line);
        if (*list == NULL) {
            *list = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }
    fclose(file);
}
void print_list(const node_t *list, int limit, const char* order_by_field) {
    FILE *file = fopen("output.csv", "w");
    if (!file) {
        fprintf(stderr, "Could not open file for writing.\n");
        return;
    }
    const char* fourth_column_name = "";
    if (strcmp(order_by_field, "STREAMS") == 0) {
        fourth_column_name = "streams";
    } else if (strcmp(order_by_field, "NO_SPOTIFY_PLAYLISTS") == 0) {
        fourth_column_name = "in_spotify_playlists";
    } else if (strcmp(order_by_field, "NO_APPLE_PLAYLISTS") == 0) {
        fourth_column_name = "in_apple_playlists";
    }
    fprintf(file, "released,track_name,artist(s)_name,%s\n", fourth_column_name);
    int count = 0;
    while (list != NULL && (limit < 0 || count < limit)) {
        fprintf(file, "%d-%d-%d,%s,%s",
                list->released_year, list->released_month, list->released_day,
                list->track_name, list->artist_names);
        if (strcmp(order_by_field, "STREAMS") == 0) {
            fprintf(file, ",%lld\n", list->streams);
        } else if (strcmp(order_by_field, "NO_SPOTIFY_PLAYLISTS") == 0) {
            fprintf(file, ",%d\n", list->in_spotify_playlists);
        } else if (strcmp(order_by_field, "NO_APPLE_PLAYLISTS") == 0) {
            fprintf(file, ",%d\n", list->in_apple_playlists);
        } else {
            fprintf(file, ",\n");
        }
        list = list->next;
        count++;
    }
    fclose(file);
}
void free_list(node_t *list) {
    while (list) {
        node_t *temp = list->next;
        free(list);
        list = temp;
    }
}

