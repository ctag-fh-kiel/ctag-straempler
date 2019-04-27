// menusys.h menu HFSM abstraction library
// 
// Copyright 2019 Robert Manzke
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

// typedef callback function
// returns int -> next menu state
// arguments
// int -> caller ID 
// void ** -> data pointer for pass around data used in transition from one menu to another
// int -> event ID
// void * -> event data

typedef int (*_f_ptr)(int, int, void*);

typedef struct{
    int ev;
    _f_ptr cbf;
}menusys_cb_t;

typedef struct{
    int id, n_callbacks;
    menusys_cb_t **cb_array;
    _f_ptr cb_default;
}menusys_item_t;

typedef struct{
    int n_menus;
    menusys_item_t **items_array;
    menusys_item_t *active_item;
}menusys_t;

menusys_t *menusys_create();
void menusys_free(menusys_t *);
void menusys_new_item(menusys_t *, int);
void menusys_item_set_ev_cb(menusys_t *, int , int, _f_ptr);
void menusys_item_set_default_cb(menusys_t *, int , _f_ptr);
void menusys_all_set_ev_cb(menusys_t *, int, _f_ptr);
void menusys_process_ev(menusys_t *, int, void*);
void menusys_set_active_item(menusys_t*, int);