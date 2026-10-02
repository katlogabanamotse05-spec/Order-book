#include <assert.h>
#include <stdio.h>
#include "level.h"
#include "order.h"

#define PRICE 10100

/* Makes an order at PRICE with the given qty. The book will assign seq later;
   here we set it by hand so level_check's I5 (seq strictly increasing) holds. */
static Order *make_order(uint64_t id, uint32_t qty, uint64_t seq) {
    Order_Status st = ORDER_NOMEM;
    Order *o = order_create(id, SIDE_SELL, PRICE, qty, &st);
    assert(o != NULL);
    assert(st == ORDER_OK);
    o->seq = seq;
    return o;
}

/* 1. Empty level */
static void test_empty_level(void) {
    Price_Level *lv = level_create(PRICE);
    assert(lv != NULL);
    assert(lv->price == PRICE);
    assert(lv->head == NULL && lv->tail == NULL);
    assert(lv->next == NULL && lv->prev == NULL);
    assert(lv->total_qty == 0);
    assert(lv->n_orders == 0);
    assert(level_front(lv) == NULL);
    assert(level_check(lv));
    level_destroy(lv);
}

/* 2. Push 5, 7, 9: totals, front, and both walk directions */
static void test_push_back(void) {
    Price_Level *lv = level_create(PRICE);
    Order *a = make_order(1, 5, 1);
    level_push_back(lv, a);
    assert(level_check(lv));
    assert(lv->head == a && lv->tail == a);
    assert(a->prev == NULL && a->next == NULL);
    assert(a->level == lv);

    Order *b = make_order(2, 7, 2);
    level_push_back(lv, b);
    assert(level_check(lv));
    Order *c = make_order(3, 9, 3);
    level_push_back(lv, c);
    assert(level_check(lv));

    assert(lv->total_qty == 21);
    assert(lv->n_orders == 3);
    assert(level_front(lv) == a);

    /* forwards: a, b, c */
    assert(lv->head == a && a->next == b && b->next == c && c->next == NULL);
    /* backwards: c, b, a */
    assert(lv->tail == c && c->prev == b && b->prev == a && a->prev == NULL);

    level_destroy(lv);   /* frees a, b, c */
}

/* 3. Unlink the middle */
static void test_unlink_middle(void) {
    Price_Level *lv = level_create(PRICE);
    Order *a = make_order(1, 5, 1);
    Order *b = make_order(2, 7, 2);
    Order *c = make_order(3, 9, 3);
    level_push_back(lv, a);
    level_push_back(lv, b);
    level_push_back(lv, c);

    level_unlink(lv, b);
    assert(level_check(lv));
    assert(lv->total_qty == 14);
    assert(lv->n_orders == 2);
    assert(a->next == c);
    assert(c->prev == a);
    assert(lv->head == a && lv->tail == c);
    /* the unlinked order looks like a fresh one */
    assert(b->prev == NULL && b->next == NULL && b->level == NULL);

    order_destroy(b);    /* unlink does not free: b is ours */
    level_destroy(lv);
}

/* 4. Unlink head, then tail, then the last remaining order */
static void test_unlink_head_tail_last(void) {
    Price_Level *lv = level_create(PRICE);
    Order *a = make_order(1, 5, 1);
    Order *b = make_order(2, 7, 2);
    Order *c = make_order(3, 9, 3);
    level_push_back(lv, a);
    level_push_back(lv, b);
    level_push_back(lv, c);

    level_unlink(lv, a);                 /* head */
    assert(level_check(lv));
    assert(lv->head == b && b->prev == NULL);
    assert(lv->total_qty == 16 && lv->n_orders == 2);
    order_destroy(a);

    level_unlink(lv, c);                 /* tail */
    assert(level_check(lv));
    assert(lv->tail == b && b->next == NULL);
    assert(lv->total_qty == 7 && lv->n_orders == 1);
    order_destroy(c);

    level_unlink(lv, b);                 /* the only order */
    assert(level_check(lv));
    assert(lv->head == NULL && lv->tail == NULL);
    assert(lv->total_qty == 0 && lv->n_orders == 0);
    order_destroy(b);

    level_destroy(lv);
}

/* 5. Partial fill keeps position */
static void test_reduce(void) {
    Price_Level *lv = level_create(PRICE);
    Order *a = make_order(1, 5, 1);
    Order *b = make_order(2, 7, 2);
    Order *c = make_order(3, 9, 3);
    level_push_back(lv, a);
    level_push_back(lv, b);
    level_push_back(lv, c);

    level_reduce(lv, level_front(lv), 2);
    assert(level_check(lv));
    assert(lv->total_qty == 19);
    assert(a->qty == 3);
    assert(lv->n_orders == 3);
    /* position unchanged */
    assert(level_front(lv) == a && a->next == b && b->next == c);

    level_destroy(lv);
}

/* 6. Pop from the front: orders leave in arrival order */
static void test_pop_in_arrival_order(void) {
    Price_Level *lv = level_create(PRICE);
    Order *o[3];
    for (int i = 0; i < 3; i++) {
        o[i] = make_order((uint64_t)(i + 1), (uint32_t)(5 + 2 * i), (uint64_t)(i + 1));
        level_push_back(lv, o[i]);
    }
    for (int i = 0; i < 3; i++) {
        Order *front = level_front(lv);
        assert(front == o[i]);
        level_unlink(lv, front);
        assert(level_check(lv));
        order_destroy(front);
    }
    assert(level_front(lv) == NULL);
    assert(lv->total_qty == 0);
    level_destroy(lv);
}

/* 7. level_check must be able to fail: corrupt things by hand.
      (If your level_check asserts instead of returning false, this test
      will abort; make it return false, as the spec allows either.) */
static void test_check_detects_corruption(void) {
    Price_Level *lv = level_create(PRICE);
    Order *a = make_order(1, 5, 1);
    Order *b = make_order(2, 7, 2);
    level_push_back(lv, a);
    level_push_back(lv, b);
    assert(level_check(lv));

    /* wrong total (I3) */
    lv->total_qty += 1;
    assert(!level_check(lv));
    lv->total_qty -= 1;
    assert(level_check(lv));

    /* broken back-link (I8) */
    b->prev = NULL;
    assert(!level_check(lv));
    b->prev = a;
    assert(level_check(lv));

    /* stale tail (the mutation-7 bug) */
    lv->tail = a;
    assert(!level_check(lv));
    lv->tail = b;
    assert(level_check(lv));

    /* seq not increasing (I5) */
    b->seq = 1;
    assert(!level_check(lv));
    b->seq = 2;
    assert(level_check(lv));

    level_destroy(lv);
}

/* 8. Destroy empty, destroy one-order */
static void test_destroy_edges(void) {
    Price_Level *empty = level_create(PRICE);
    level_destroy(empty);

    Price_Level *one = level_create(PRICE);
    level_push_back(one, make_order(1, 5, 1));
    level_destroy(one);
}

int main(void) {
    test_empty_level();
    test_push_back();
    test_unlink_middle();
    test_unlink_head_tail_last();
    test_reduce();
    test_pop_in_arrival_order();
    test_check_detects_corruption();
    test_destroy_edges();
    puts("test_level: all passed");
    return 0;
}
