#include "level.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

Price_Level *level_create(int64_t price){
    Price_Level *result = calloc(1,sizeof(Price_Level)); /* calloc zeros all the memeber fields*/
    if( result == NULL){
        return NULL;
    }
    result->price = price;
    return result;
}
Order *level_front(const Price_Level *lv) {
    return lv->head;
}
void level_push_back(Price_Level *lv, Order *ord){

    assert(ord->price == lv->price && ord->level == NULL);
    ord->level = lv;
    ord->prev = lv->tail;
    ord->next = NULL;

    if (lv->head == NULL){
        lv->head = ord;
    } else {
        lv->tail->next = ord;
    }
    lv->tail = ord;

    lv->total_qty += ord->qty;
    lv->n_orders++;
}void level_unlink(Price_Level *lv, Order *ord) {
    assert(ord->level == lv);

    /* Who is in front of me? */
    if (ord->prev != NULL) {
        ord->prev->next = ord->next;
    } else {
        lv->head = ord->next;      /* I was the head */
    }

    /* Who is behind me? */
    if (ord->next != NULL) {
        ord->next->prev = ord->prev;
    } else {
        lv->tail = ord->prev;      /* I was the tail */
    }

    lv->total_qty -= ord->qty;
    lv->n_orders--;

    /* Reset last: the checks above needed prev and next. */
    ord->prev = NULL;
    ord->next = NULL;
    ord->level = NULL;
}
void level_reduce(Price_Level *lv, Order *ord, uint32_t by){
    assert(ord->level == lv);
    assert(by < ord->qty);
    assert(by > 0);

    lv->total_qty-= by;
    ord->qty-=by;
}
void level_destroy(Price_Level *lv){
    if (lv == NULL){
        return;
    }
    Order *curr = lv->head;
    while (curr != NULL){
        Order *next = curr->next;
        order_destroy(curr);
        curr = next;
    }
    free(lv);
}

bool level_check(const Price_Level *lv){
    /* Empty level: everything must agree it is empty. */
    if (lv->head == NULL){
        if (lv->tail != NULL || lv->total_qty != 0 || lv->n_orders != 0){
            fprintf(stderr, "level_check: I3/I8 failed (empty level has stale fields)\n");
            return false;
        }
        return true;
    }

    if (lv->tail == NULL){
        fprintf(stderr, "level_check: I8 failed (head set but tail is NULL)\n");
        return false;
    }
    if (lv->head->prev != NULL || lv->tail->next != NULL){
        fprintf(stderr, "level_check: I8 failed (head->prev or tail->next not NULL)\n");
        return false;
    }

    uint64_t count_qty = 0;
    uint32_t count_order = 0;
    const Order *current = lv->head;
    const Order *prev = NULL;

    while (current != NULL){
        if (prev != NULL && prev->seq >= current->seq){
            fprintf(stderr, "level_check: I5 failed (seq not increasing at order %llu)\n",
                    (unsigned long long)current->id);
            return false;
        }
        if (current->next != NULL && current->next->prev != current){
            fprintf(stderr, "level_check: I8 failed (order %llu next->prev mismatch)\n",
                    (unsigned long long)current->id);
            return false;
        }
        if (current->qty == 0 || current->price != lv->price){
            fprintf(stderr, "level_check: I4 failed (order %llu qty or price wrong)\n",
                    (unsigned long long)current->id);
            return false;
        }
        if (current->level != lv){
            fprintf(stderr, "level_check: I8 failed (order %llu has wrong level)\n",
                    (unsigned long long)current->id);
            return false;
        }
        count_qty += current->qty;
        count_order++;
        prev = current;
        current = current->next;
    }

    if (prev != lv->tail){
        fprintf(stderr, "level_check: I8 failed (last node is not tail)\n");
        return false;
    }
    if (count_order != lv->n_orders || count_qty != lv->total_qty){
        fprintf(stderr, "level_check: I3 failed (count or total_qty mismatch)\n");
        return false;
    }
    return true;
}
