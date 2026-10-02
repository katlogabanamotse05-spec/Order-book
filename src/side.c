#include <assert.h>
#include <stdlib.h>
#include "side.h"

void side_init(Book_Side *s, Side side){
    s->best = NULL;
    s->n_levels = 0;
    s->side = side;
}

Price_Level *side_best(const Book_Side *s){
    return s->best;
}

/* Returns the level at `price`, creating it in the right place if needed.
   Returns NULL ONLY if allocation fails (nothing is modified in that case). */
Price_Level *side_find_or_create(Book_Side *s, int64_t price){
    Price_Level **link = &s->best;   /* the pointer I may overwrite */
    Price_Level *prev = NULL;        /* the level I last stepped past */

    /* Step past every level that is strictly better than `price`. */
    while (*link != NULL && price_better(s->side, (*link)->price, price)){
        prev = *link;
        link = &(*link)->next;
    }

    /* Now *link is NULL, or the first level that is the same price or worse. */
    if (*link != NULL && (*link)->price == price){
        return *link;                /* level already exists */
    }

    Price_Level *new_lv = level_create(price);
    if (new_lv == NULL){
        return NULL;
    }

    /* Splice new_lv in between prev and *link. */
    new_lv->prev = prev;
    new_lv->next = *link;
    if (*link != NULL){
        (*link)->prev = new_lv;
    }
    *link = new_lv;                  /* updates s->best OR prev->next */
    s->n_levels++;
    return new_lv;
}

/* Removes an EMPTY level and frees it.
   Whoever removes the last order from a level must call this (invariant I2). */
void side_remove_level(Book_Side *s, Price_Level *lv){
    assert(s != NULL);
    assert(lv != NULL);
    assert(lv->n_orders == 0);

    /* Who is in front of me? */
    if (lv->prev != NULL) {
        lv->prev->next = lv->next;
    } else {
        s->best = lv->next;          /* I was the best level */
    }

    /* Who is behind me? (a side has no tail pointer, so no else) */
    if (lv->next != NULL) {
        lv->next->prev = lv->prev;
    }

    s->n_levels--;
    free(lv);
}
