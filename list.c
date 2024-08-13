#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emalloc.h"
#include "list.h"
node_t *new_node(const char *track_name, char *artist_names, int artist_count, int released_year, int released_month, int released_day, int in_spotify_playlists, long long streams, int in_apple_playlists) {
    node_t *temp = (node_t *)emalloc(sizeof(node_t));
    strncpy(temp->track_name, track_name, MAX_WORD_LEN - 1);
    temp->track_name[MAX_WORD_LEN - 1] = '\0';
    strncpy(temp->artist_names, artist_names, MAX_WORD_LEN - 1);
    temp->artist_names[MAX_WORD_LEN - 1] = '\0';
    temp->released_year = released_year;
    temp->released_month = released_month;
    temp->released_day = released_day;
    temp->in_spotify_playlists = in_spotify_playlists;
    temp->streams = streams;
    temp->in_apple_playlists = in_apple_playlists;
    temp->next = NULL;
    return temp;
}
node_t *add_front(node_t *list, node_t *new) {
    new->next = list;
    return new;
}
node_t *add_end(node_t *list, node_t *new) {
    node_t *curr;
    if (list == NULL) {
        new->next = NULL;
        return new;
    }
    for (curr = list; curr->next != NULL; curr = curr->next) ;
    curr->next = new;
    new->next = NULL;
    return list;
}
node_t *peek_front(node_t *list) {
    return list;
}
node_t *remove_front(node_t *list) {
    if (list == NULL) {
        return NULL;
    }
    return list->next;
}
void apply(node_t *list, void (*fn)(node_t *list, void *), void *arg) {
    for (; list != NULL; list = list->next) {
        (*fn)(list, arg);
    }
}
bool compare_streams(node_t *a, node_t *b, Order order) {
    if (order == ASCENDING) {
        return a->streams < b->streams;
    } else {
        return a->streams > b->streams;
    }
}
bool compare_in_spotify_playlists(node_t *a, node_t *b, Order order) {
    if (order == ASCENDING) {
        return a->in_spotify_playlists < b->in_spotify_playlists;
    } else {
        return a->in_spotify_playlists > b->in_spotify_playlists;
    }
}
bool compare_in_apple_playlists(node_t *a, node_t *b, Order order) {
    if (order == ASCENDING) {
        return a->in_apple_playlists < b->in_apple_playlists;
    } else {
        return a->in_apple_playlists > b->in_apple_playlists;
    }
}
node_t *add_inorder(node_t *head, node_t *new_node, SortContext *context) {
    if (head == NULL || context->compare(new_node, head, context->order)) {
        new_node->next = head;
        return new_node;
    }
    node_t *current = head;
    while (current->next != NULL && !context->compare(new_node, current->next, context->order)) {
        current = current->next;
    }
    new_node->next = current->next;
    current->next = new_node;
    return head;
}
node_t *copy_node(const node_t *original) {
    node_t *copy = emalloc(sizeof(node_t));
    strncpy(copy->track_name, original->track_name, MAX_WORD_LEN);
    strncpy(copy->artist_names, original->artist_names, MAX_WORD_LEN);
    copy->artist_count = original->artist_count;
    copy->released_year = original->released_year;
    copy->released_month = original->released_month;
    copy->released_day = original->released_day;
    copy->in_spotify_playlists = original->in_spotify_playlists;
    copy->streams = original->streams;
    copy->in_apple_playlists = original->in_apple_playlists;
    copy->next = NULL;
    return copy;
}
bool satisfies_filter(node_t *node, FilterContext *context) {
    if (strcmp(context->filter_field, "ARTIST") == 0) {
        if (strstr(node->artist_names, context->filter_value) != NULL) {
            return true;
        }
    } else if (strcmp(context->filter_field, "YEAR") == 0) {
        int year = atoi(context->filter_value);
        if (node->released_year == year) {
            return true;
        }
    }
    return false;
}
node_t *filter_list(node_t *head, FilterContext *context) {
    node_t *filtered_head = NULL;
    node_t *filtered_tail = NULL;
    node_t *current = head;
    while (current != NULL) {
        if (satisfies_filter(current, context)) {
            node_t *copy = copy_node(current);
            if (filtered_head == NULL) {
                filtered_head = copy;
            } else {
                filtered_tail->next = copy;
            }
            filtered_tail = copy;
        }
        node_t *temp = current;
        current = current->next;
        free(temp);
    }
    return filtered_head;
}
