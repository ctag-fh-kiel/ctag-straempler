// menusys.c menu HFSM abstraction library
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

#include <stdlib.h> 
#include <string.h>
#include "menusys.h"


menusys_t *menusys_create(){
    menusys_t *ms = (menusys_t*)malloc(sizeof(menusys_t));
    ms->n_menus = 0;
    ms->items_array = NULL;
    ms->active_item = NULL;
    return ms;
}

void menusys_free(menusys_t *ms){
    for(int i=0;i<ms->n_menus;i++){
        menusys_item_t *it = ms->items_array[i];
        for(int j=0;j<it->n_callbacks;j++) free(it->cb_array[j]);
        free(ms->items_array[i]);
    }
    free(ms);
}

void menusys_new_item(menusys_t *ms, int id){
    menusys_item_t *it = (menusys_item_t*)malloc(sizeof(menusys_item_t));
    it->id = id;
    it->cb_array = NULL;
    it->n_callbacks = 0;
    it->cb_default = NULL;
    ms->n_menus++;
    ms->items_array = (menusys_item_t**)realloc(ms->items_array, ms->n_menus * sizeof(menusys_item_t*));
    ms->items_array[ms->n_menus - 1] = it;
}

void menusys_item_set_ev_cb(menusys_t *ms, int id, int ev, _f_ptr cbf){
    for(int i=0;i<ms->n_menus;i++){
        if(ms->items_array[i]->id == id){
            menusys_item_t *it = ms->items_array[i];
            menusys_cb_t *cb = (menusys_cb_t*)malloc(sizeof(menusys_cb_t));
            cb->ev = ev;
            cb->cbf = cbf;
            it->n_callbacks++;
            it->cb_array = (menusys_cb_t**)realloc(it->cb_array, it->n_callbacks * sizeof(menusys_cb_t*));
            it->cb_array[it->n_callbacks - 1] = cb;
            break;
        }
    }
}

void menusys_process_ev(menusys_t *ms, int ev, void* ev_data){
    menusys_item_t *it = ms->active_item;
    if(it == NULL) return;
    int ev_handled = 0;
    int it_id_prev;
    for(int i=0;i<it->n_callbacks;i++){
        if(it->cb_array[i]->ev == ev){
            _f_ptr cbf = it->cb_array[i]->cbf;
            it_id_prev = it->id;
            int new_item_id = cbf(it->id, ev, ev_data);
            ev_handled = 1;
            if(new_item_id != 0){
                for(int j=0;j<ms->n_menus;j++){
                    if(ms->items_array[j]->id == new_item_id){
                        ms->active_item = ms->items_array[j];
                        it = ms->active_item;
                        // call once default handler of new menu first transition, event -1
                        if(it->cb_default != NULL){
                            cbf = it->cb_default;
                            cbf(it_id_prev, -1, NULL);
                        }
                        break;
                    }
                }
            }
            break;
        }
    }
    if(ev_handled == 0 && it->cb_default != NULL){
        _f_ptr cbf = it->cb_default;
        it_id_prev = it->id;
        int new_item_id = cbf(it->id, ev, ev_data);
        if(new_item_id != 0){
            for(int j=0;j<ms->n_menus;j++){
                if(ms->items_array[j]->id == new_item_id){
                    ms->active_item = ms->items_array[j];
                    it = ms->active_item;
                    // call once default handler of new menu first transition, event -1
                    if(it->cb_default != NULL){
                        cbf = it->cb_default;
                        cbf(it_id_prev, -1, NULL);
                    }
                    break;
                }
            }
        }
    }
}

void menusys_set_active_item(menusys_t *ms, int id){
    for(int i=0;i<ms->n_menus;i++){
        if(ms->items_array[i]->id == id){
            ms->active_item = ms->items_array[i];
            menusys_item_t *it = ms->active_item;
            if(it->cb_default != NULL){
                _f_ptr cbf = it->cb_default;
                // call once default handler of new menu first transition, event -1
                cbf(it->id, -1, NULL);
            }
            break;
        }
    }
}

void menusys_all_set_ev_cb(menusys_t *ms, int ev, _f_ptr cbf){
    for(int i=0;i<ms->n_menus;i++){
        menusys_item_t *it = ms->items_array[i];
        menusys_cb_t *cb = (menusys_cb_t*)malloc(sizeof(menusys_cb_t));
        cb->ev = ev;
        cb->cbf = cbf;
        it->n_callbacks++;
        it->cb_array = (menusys_cb_t**)realloc(it->cb_array, it->n_callbacks * sizeof(menusys_cb_t*));
        it->cb_array[it->n_callbacks - 1] = cb;
    }
}

void menusys_item_set_default_cb(menusys_t *ms, int id, _f_ptr cbf){
    for(int i=0;i<ms->n_menus;i++){
        if(ms->items_array[i]->id == id){
            menusys_item_t *it = ms->items_array[i];
            it->cb_default = cbf;
        }
    }
}