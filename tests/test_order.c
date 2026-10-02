#include <assert.h>
#include <stdio.h>
#include "order.h"

/* Every failure test starts st at a deliberately wrong value, so a
   function that forgets to write the status is caught. */

static void test_valid_buy(void) {
    Order_Status st = ORDER_NOMEM;
    Order *o = order_create(1, SIDE_BUY, 10100, 50, &st);
    assert(o != NULL);
    assert(st == ORDER_OK);
    assert(o->id == 1 && o->side == SIDE_BUY && o->price == 10100 && o->qty == 50);
    assert(o->seq == 0);
    assert(o->prev == NULL && o->next == NULL && o->level == NULL);
    order_destroy(o);
}

static void test_valid_sell(void) {
    Order_Status st = ORDER_NOMEM;
    Order *o = order_create(2, SIDE_SELL, 10150, 25, &st);
    assert(o != NULL);
    assert(st == ORDER_OK);
    assert(o->id == 2 && o->side == SIDE_SELL && o->price == 10150 && o->qty == 25);
    assert(o->seq == 0);
    assert(o->prev == NULL && o->next == NULL && o->level == NULL);
    order_destroy(o);
}

static void test_zero_qty(void) {
    Order_Status st = ORDER_OK;
    Order *o = order_create(1, SIDE_BUY, 100, 0, &st);
    assert(o == NULL);
    assert(st == ORDER_BAD_QTY);
}

static void test_zero_price(void) {
    Order_Status st = ORDER_OK;
    Order *o = order_create(1, SIDE_BUY, 0, 5, &st);
    assert(o == NULL);
    assert(st == ORDER_BAD_PRICE);
}

static void test_negative_price(void) {
    Order_Status st = ORDER_OK;
    Order *o = order_create(1, SIDE_BUY, -1, 5, &st);
    assert(o == NULL);
    assert(st == ORDER_BAD_PRICE);
}

static void test_zero_id(void) {
    Order_Status st = ORDER_OK;
    Order *o = order_create(0, SIDE_BUY, 100, 5, &st);
    assert(o == NULL);
    assert(st == ORDER_BAD_ID);
}

static void test_null_status_valid(void) {
    Order *o = order_create(1, SIDE_BUY, 100, 5, NULL);
    assert(o != NULL);
    order_destroy(o);
}

static void test_null_status_invalid(void) {
    assert(order_create(1, SIDE_BUY, 100, 0, NULL) == NULL);
}

static void test_smallest_valid(void) {
    Order_Status st = ORDER_NOMEM;
    Order *o = order_create(1, SIDE_SELL, 1, 1, &st);
    assert(o != NULL);
    assert(st == ORDER_OK);
    assert(o->price == 1 && o->qty == 1);
    order_destroy(o);
}

static void test_largest_qty(void) {
    Order_Status st = ORDER_NOMEM;
    Order *o = order_create(UINT64_MAX, SIDE_BUY, INT64_MAX, UINT32_MAX, &st);
    assert(o != NULL);
    assert(st == ORDER_OK);
    assert(o->id == UINT64_MAX && o->price == INT64_MAX && o->qty == UINT32_MAX);
    order_destroy(o);
}

int main(void) {
    test_valid_buy();
    test_valid_sell();
    test_zero_qty();
    test_zero_price();
    test_negative_price();
    test_zero_id();
    test_null_status_valid();
    test_null_status_invalid();
    test_smallest_valid();
    test_largest_qty();
    puts("test_order: all passed");
    return 0;
}
