// modified / extended / fixed from https://gist.github.com/viking/2521704

#ifndef __LIST_H
#define __LIST_H

#include <pthread.h>

typedef struct _list_item{
  void *value;
  struct _list_item *prev;
  struct _list_item *next;
} list_item_t;

typedef struct {
  int count;
  list_item_t *head;
  list_item_t *tail;
  pthread_mutex_t mutex;
} list_t;

list_t *list_create();
void list_free(list_t *l);

list_item_t *list_add_element(list_t *l, void *ptr);
list_item_t *list_add_element_if(list_t *l, int (*func)(list_item_t *, void*), void * ptr);
list_item_t *list_get_item(list_t *l, int item);
int list_remove_element(list_t *l, void *ptr);
int list_each_element(list_t *l, int (*func)(list_item_t *));
// needs specific functor implementation based on data type, returns index of found element, otherwise -1
int list_find_element(list_t *l, void *, int (*func)(list_item_t *, void *));  

// functors
int f_compare_strings(list_item_t *item, void* value);

#endif