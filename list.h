#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_
#include <stdbool.h>
#define MAX_WORD_LEN 50
typedef struct node_t {
    char track_name[MAX_WORD_LEN];
    char artist_names[MAX_WORD_LEN];
    int artist_count;
    int released_year;
    int released_month;
    int released_day;
    int in_spotify_playlists;
    long long streams;
    int in_apple_playlists;
    struct node_t *next;
} node_t;
typedef enum {
    ASCENDING,
    DESCENDING
} Order;
typedef bool (*CompareFunc)(node_t*, node_t*, Order);
typedef struct {
    CompareFunc compare;
    Order order;
} SortContext;
typedef struct {
    char *filter_field;
    char *filter_value;
} FilterContext;
node_t *new_node(const char *track_name, char *artist_names, int artist_count, int released_year, int released_month, int released_day, int in_spotify_playlists, long long streams, int in_apple_playlists);
node_t *add_front(node_t *, node_t *);
node_t *add_end(node_t *, node_t *);
node_t *add_inorder(node_t *, node_t *, SortContext*);
node_t *peek_front(node_t *);
node_t *remove_front(node_t *);
void apply(node_t *, void (*fn)(node_t *, void *), void *arg);
bool compare_streams(node_t *a, node_t *b, Order order);
bool compare_in_spotify_playlists(node_t *a, node_t *b, Order order);
bool compare_in_apple_playlists(node_t *a, node_t *b, Order order);
node_t *copy_node(const node_t *original);
bool satisfies_filter(node_t *node, FilterContext *context);
node_t *filter_list(node_t *head, FilterContext *context);
#endif
