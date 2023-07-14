#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "deque.h"

// Bi-dimensional array Deque

struct Deque {
    data_type **blocks;
    int blocks_allocated;
    int initial_block_idx, final_block_idx;
    int start_idx, end_idx;
    int block_size;
    DequeFree free_fn;
};

// criacao do deque
Deque *deque_construct(DequeFree free_fn) {
    Deque *d = calloc(1, sizeof(Deque));

    d->block_size = 5;
    d->start_idx = d->end_idx = 0;
    
    d->blocks_allocated = 15;
    d->blocks = calloc(d->blocks_allocated, sizeof(data_type *));
    d->initial_block_idx = d->final_block_idx = (int)(d->blocks_allocated / 2);
    
    d->free_fn = free_fn;

    return d;
}

void _handle_center_blocks(Deque *d) {
    data_type **new_blocks = calloc(d->blocks_allocated, sizeof(data_type *));
    int size = d->final_block_idx - d->initial_block_idx;
    int shift = (d->blocks_allocated - size) / 2;
    //printf("Shifting by %d, size %d\n", shift, size);
    memcpy(&new_blocks[shift], &d->blocks[d->initial_block_idx], size * sizeof(data_type *));
    //printf("Centered\n");
    d->initial_block_idx = shift;
    d->final_block_idx = size+shift;
    free(d->blocks);
    d->blocks = new_blocks;
}

void _handle_push_back(Deque *d) {
    if (d->initial_block_idx >= 5) {
        int shift = d->initial_block_idx / 2;
        for (int i = d->initial_block_idx; i < d->final_block_idx; i++) {
            d->blocks[i-shift] = d->blocks[i];
            d->blocks[i] = NULL;
        }
        d->initial_block_idx -= shift;
        d->final_block_idx -= shift;
    }
    else {
        d->blocks_allocated *= 2;
        _handle_center_blocks(d);
    }
}

void _handle_push_front(Deque *d) {
    if (d->final_block_idx < (d->blocks_allocated - 5)) {
        int shift = (d->blocks_allocated - d->final_block_idx) / 2;
        for (int i = d->final_block_idx; i > d->initial_block_idx; i--) {
            d->blocks[i+shift] = d->blocks[i];
            d->blocks[i] = NULL;
        }
        d->initial_block_idx += shift;
        d->final_block_idx += shift;
    }
    else {
        d->blocks_allocated *= 2;
        _handle_center_blocks(d);
    }
}

// funcoes para insercao na direita e esquerda (devem ser feitas em O(1), com eventual realloc)
void deque_push_back(Deque *d, void *val) {
    
    if (d->final_block_idx >= d->blocks_allocated) {
        _handle_push_back(d);
    }

    if (!d->blocks[d->final_block_idx]) {
        d->blocks[d->final_block_idx] = calloc(d->block_size, sizeof(data_type *));
        //printf("Aloquei\n");
    }

    d->blocks[d->final_block_idx][d->end_idx] = val;
    //printf("Adding %d %d\n", d->final_block_idx, d->end_idx);
    //printf("Coloquei %d %d\n", d->final_block_idx, d->end_idx);

    d->end_idx++;
    if (d->end_idx >= d->block_size) {
        d->end_idx = 0;
        d->final_block_idx++;
    }
}

void deque_push_front(Deque *d, void *val) {
    d->start_idx--;
    if (d->start_idx < 0) {
        d->start_idx = d->block_size - 1;
        d->initial_block_idx--;
    }
    if (d->initial_block_idx < 0) {
        _handle_push_front(d);
    }
    if (!d->blocks[d->initial_block_idx]) {
        d->blocks[d->initial_block_idx] = calloc(d->block_size, sizeof(data_type *));
        //printf("Aloquei\n");
    }

    d->blocks[d->initial_block_idx][d->start_idx] = val;
    //printf("Coloquei %d %d\n", d->initial_block_idx, d->start_idx);
}

// funcoes para remocao na direita e esquerda (devem ser feitas em O(1))
void *deque_pop_back(Deque *d) {
    if (d->start_idx == d->end_idx && d->initial_block_idx == d->final_block_idx) {
        printf("ERROR: trying to pop and empty Deque\n");
        exit(1);
    }

    data_type val = deque_get(d, deque_size(d)-1);
    //printf("Popping %d\n", deque_size(d) - 1);

    d->end_idx--;
    if (d->end_idx == 0) {
        free(d->blocks[d->final_block_idx]);
        d->blocks[d->final_block_idx] = NULL;
    }
    if (d->end_idx < 0) {
        d->end_idx = d->block_size - 1;
        d->final_block_idx--;
    }

    return val;
}
void *deque_pop_front(Deque *d) {
    if (d->start_idx == d->end_idx && d->initial_block_idx == d->final_block_idx) {
        printf("ERROR: trying to pop and empty Deque\n");
        exit(1);
    }

    data_type val = deque_get(d, 0);

    d->start_idx++;
    if (d->start_idx >= d->block_size) {
        d->start_idx = 0;
        free(d->blocks[d->initial_block_idx]);
        d->blocks[d->initial_block_idx] = NULL;
        d->initial_block_idx++;
    }

    return val;
}

// numero de elementos
int deque_size(Deque *d) {
    return (d->end_idx - d->start_idx + (d->block_size * (d->final_block_idx - d->initial_block_idx)));
}

// obtem o i-esimo elemento
void *deque_get(Deque *d, int idx) {
    if (idx >= deque_size(d)) {
        printf("Error: Index out of bounds\n");
        return NULL;
    }

    int index = (idx + d->start_idx) % d->block_size;
    int block = d->initial_block_idx + ((idx + d->start_idx) / d->block_size);

    return d->blocks[block][index];
}

// libera o espaco alocado para o deque
void deque_destroy(Deque *d) {
    int size = deque_size(d);
    for (int i = 0; i < size; i++) {
        data_type val = deque_pop_back(d);

        if (d->free_fn) {
            d->free_fn(val);
        }
    }
    for (int i = 0; i < d->blocks_allocated; i++) {
        free(d->blocks[i]);
    }
    free(d->blocks);
    free(d);
}