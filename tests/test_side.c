#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "side.h"
#include "level.h"
#include "order.h"

/* Walks the whole side and checks:
   - levels are strictly ordered best to worst (I1), prev/next consistent (I8)
   - no empty level (I2), every level passes level_check (I3, I4, I5, I8)
   - n_levels matches what the walk counts
   - the number of orders found equals expected_orders */
static void check_side(const Book_Side *s, size_t expected_orders) {
    const Price_Level *prev = NULL;
    size_t levels = 0, orders = 0;
    for (const Price_Level *lv = s->best; lv != NULL; lv = lv->next) {
        assert(lv->prev == prev);
        assert(lv->n_orders > 0);
        assert(level_check(lv));
        if (prev != NULL) {
            assert(price_better(s->side, prev->price, lv->price));
        }
        levels++;
        orders += lv->n_orders;
        prev = lv;
    }
    assert(levels == s->n_levels);
    assert(orders == expected_orders);
}

static Order *make_order(uint64_t id, Side side, int64_t price, uint64_t seq) {
    Order_Status st = ORDER_NOMEM;
    Order *o = order_create(id, side, price, 1, &st);
    assert(o != NULL);
    assert(st == ORDER_OK);
    o->seq = seq;
    return o;
}

/* Puts an order on the side the way the book will: find or create the level,
   then push at the back. */
static Price_Level *add_order(Book_Side *s, Order *o) {
    Price_Level *lv = side_find_or_create(s, o->price);
    assert(lv != NULL);
    level_push_back(lv, o);
    return lv;
}

/* Removes every order in the given order, removing each level as it empties,
   checking the side after every step. Ends with an empty side. */
static void drain(Book_Side *s, Order **orders, size_t n) {
    for (size_t i = 0; i < n; i++) {
        Order *o = orders[i];
        Price_Level *lv = o->level;
        level_unlink(lv, o);
        if (lv->n_orders == 0) {
            side_remove_level(s, lv);
        }
        order_destroy(o);
        check_side(s, n - i - 1);
    }
    assert(s->best == NULL);
    assert(s->n_levels == 0);
}

/* price_better: six cases (below, equal, above) for each side */
static void test_price_better(void) {
    assert(price_better(SIDE_BUY, 101, 100));    /* bids: higher is better */
    assert(!price_better(SIDE_BUY, 100, 100));   /* equal is NOT better */
    assert(!price_better(SIDE_BUY, 99, 100));
    assert(price_better(SIDE_SELL, 99, 100));    /* asks: lower is better */
    assert(!price_better(SIDE_SELL, 100, 100));
    assert(!price_better(SIDE_SELL, 101, 100));
}

static void test_init(void) {
    Book_Side bids, asks;
    side_init(&bids, SIDE_BUY);
    side_init(&asks, SIDE_SELL);
    assert(bids.side == SIDE_BUY && bids.best == NULL && bids.n_levels == 0);
    assert(asks.side == SIDE_SELL && asks.best == NULL && asks.n_levels == 0);
    assert(side_best(&bids) == NULL);
    assert(side_best(&asks) == NULL);
    check_side(&bids, 0);
    check_side(&asks, 0);
}

/* Insertion cases: empty side, new best, new worst, middle, price already present.
   best_first holds the 5 distinct prices in the order they must appear. */
static void test_insert_cases(Side side, const int64_t *best_first) {
    Book_Side s;
    side_init(&s, side);

    /* bids: 10100, 10050 (worst), 10200 (new best), 10075 (middle), 10000 (worst),
       10100 again (already present). Asks use the same input; the order flips. */
    const int64_t input[6] = {10100, 10050, 10200, 10075, 10000, 10100};
    Order *orders[6];
    Price_Level *first_100 = NULL;

    for (size_t i = 0; i < 6; i++) {
        orders[i] = make_order(i + 1, side, input[i], i + 1);
        Price_Level *lv = add_order(&s, orders[i]);
        if (input[i] == 10100 && first_100 == NULL) first_100 = lv;
        if (i == 5) {
            assert(lv == first_100);        /* same level returned, not a new one */
        }
        check_side(&s, i + 1);
    }
    assert(s.n_levels == 5);

    /* expected order from best */
    const Price_Level *lv = s.best;
    for (size_t i = 0; i < 5; i++) {
        assert(lv != NULL && lv->price == best_first[i]);
        lv = lv->next;
    }
    assert(lv == NULL);
    assert(side_best(&s)->price == best_first[0]);
    assert(first_100->n_orders == 2);        /* two orders at 10100 */

    drain(&s, orders, 6);
}

/* 1,000 shuffled prices (with duplicates) in, strict order, then remove in random order */
#define N 1000
#define PRICE_BASE 10000
#define PRICE_SPREAD 400

static void shuffle(Order **a, size_t n) {
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = (size_t)rand() % (i + 1);
        Order *tmp = a[i]; a[i] = a[j]; a[j] = tmp;
    }
}

static void test_shuffled(Side side) {
    Book_Side s;
    side_init(&s, side);
    static Order *orders[N];
    static bool seen[PRICE_SPREAD];
    size_t distinct = 0;
    for (size_t i = 0; i < PRICE_SPREAD; i++) seen[i] = false;

    for (size_t i = 0; i < N; i++) {
        int64_t price = PRICE_BASE + (rand() % PRICE_SPREAD);
        if (!seen[price - PRICE_BASE]) {
            seen[price - PRICE_BASE] = true;
            distinct++;
        }
        orders[i] = make_order(i + 1, side, price, i + 1);
        add_order(&s, orders[i]);
    }
    check_side(&s, N);
    assert(s.n_levels == distinct);

    shuffle(orders, N);
    drain(&s, orders, N);
}

int main(void) {
    srand(12345);   /* fixed seed: the run is reproducible */

    test_price_better();
    test_init();

    const int64_t bid_order[5] = {10200, 10100, 10075, 10050, 10000};  /* high to low */
    const int64_t ask_order[5] = {10000, 10050, 10075, 10100, 10200};  /* low to high */
    test_insert_cases(SIDE_BUY, bid_order);
    test_insert_cases(SIDE_SELL, ask_order);

    test_shuffled(SIDE_BUY);
    test_shuffled(SIDE_SELL);

    puts("test_side: all passed");
    return 0;
}
