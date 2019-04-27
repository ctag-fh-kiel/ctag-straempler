#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"

/* Naive linked list implementation */

list_t *
list_create()
{
  list_t *l = (list_t *) malloc(sizeof(list_t));
  l->count = 0;
  l->head = NULL;
  l->tail = NULL;
  pthread_mutex_init(&(l->mutex), NULL);
  return l;
}

void
list_free(l)
  list_t  *l;
{
  list_item_t *li, *tmp;

  pthread_mutex_lock(&(l->mutex));

  if (l != NULL && l->count != 0) {
    li = l->head;
    // delete value
    free(li->value);
    while (li != NULL) {
      tmp = li->next;
      free(li);
      li = tmp;
    }
  }

  pthread_mutex_unlock(&(l->mutex));
  pthread_mutex_destroy(&(l->mutex));
  free(l);
  l = NULL;
}

list_item_t *
list_add_element(l, ptr)
  list_t *l;
  void *ptr;
{
  list_item_t *li;

  pthread_mutex_lock(&(l->mutex));

  li = (list_item_t *) malloc(sizeof(list_item_t));
  li->value = ptr;
  li->next = NULL;
  li->prev = l->tail;

  if (l->tail == NULL) {
    l->head = l->tail = li;
  }
  else {
    l->tail->next = li;
    l->tail = li;
  }
  l->count++;

  pthread_mutex_unlock(&(l->mutex));

  return li;
}


list_item_t *
list_get_item(l, item)
  list_t *l;
  int item;
{
  list_item_t *li;
  pthread_mutex_lock(&(l->mutex));

  li = l->head;
  for(int i=0;i<item;i++){
    li = li->next;
  }
  pthread_mutex_unlock(&(l->mutex));
  return li;
}

int
list_remove_element(l, ptr)
  list_t *l;
  void *ptr;
{
  int result = 0;
  list_item_t *li = l->head;

  pthread_mutex_lock(&(l->mutex));

  while (li != NULL) {
    if (li->value == ptr) {
      if (li->prev == NULL) {
        l->head = li->next;
      }
      else {
        li->prev->next = li->next;
      }

      if (li->next == NULL) {
        l->tail = li->prev;
      }
      else {
        li->next->prev = li->prev;
      }
      l->count--;
      // remove value
      free(li->value);
      free(li);
      result = 1;
      break;
    }
    li = li->next;
  }

  pthread_mutex_unlock(&(l->mutex));

  return result;
}

int
list_each_element(l, func)
  list_t *l;
  int (*func)(list_item_t *);
{
  list_item_t *li;
  int r = 0;
  pthread_mutex_lock(&(l->mutex));

  li = l->head;
  while (li != NULL) {
    r = func(li);
    if (r != 0) {
      break;
    }
    li = li->next;
  }
  pthread_mutex_unlock(&(l->mutex));
  return r;
}

int
list_find_element(l, el, func)
  list_t *l;
  void* el;
  int (*func)(list_item_t *, void *);
{
  list_item_t *li;
  int r = 0, cnt = 0;
  pthread_mutex_lock(&(l->mutex));

  li = l->head;
  while (li != NULL) {
    r = func(li, el);
    if (r == 0) {
      break;
    }
    li = li->next;
    cnt++;
  }
  pthread_mutex_unlock(&(l->mutex));
  if(r != 0) cnt = -1;
  return cnt;
}


list_item_t *
list_add_element_if(l, func, ptr)
  list_t *l;
  int (*func)(list_item_t *, void*); // functor, item is added if returns 0
  void *ptr;
{
  list_item_t *li;
  int r = 0;
  pthread_mutex_lock(&(l->mutex));

  li = l->head;
  while (li != NULL) {
    r = func(li, ptr);
    if (r != 0) {
      break;
    }
    li = li->next;
  }
  pthread_mutex_unlock(&(l->mutex));

  if(r == 0){
    return list_add_element(l, ptr);
  }
  return NULL;
}

// functor for string comparison
int f_compare_strings(list_item_t *item, void* value){
  char *s1 = (char*)value;
  char *s2 = (char*)item->value;
  if(s1 == NULL || s2 == NULL) return -1;
  return strcmp(s1, s2);  
}