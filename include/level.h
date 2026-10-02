#ifndef LEVEL_H
#define LEVEL_H
#include "order.h"

typedef struct Price_Level{
    int64_t price;
    uint64_t total_qty;
    uint32_t n_orders;
    Order *head, *tail;
    struct Price_level *next, *prev;
} Price_Level;

Price_Level *level_create(int64_t price);
void level_push_back(Price_Level *lv, Order *ord);
Order *level_front(const Price_Level *lv);
void level_unlink(Price_Level *lv, Order *ord);
void level_reduce(Price_Level *lv, Order *ord, uint32_t by);
void level_destroy(Price_Level *lv);
bool level_check(const Price_Level *lv);
#endif
