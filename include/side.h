#ifndef SIDE_H
#define SIDE_H
#include "order.h"
#include "level.h"
typedef struct Book_Side{
    Side side;
    Price_Level *best;
    size_t n_levels;
} Book_Side;

void side_init(Book_Side *s, Side side);
static inline bool price_better(Side s, int64_t a, int64_t b){
    if (s == SIDE_BUY){
        return a > b;
    } else {
        return a < b;
    }
}
Price_Level *side_best(const Book_Side *s);
Price_Level *side_find_or_create(Book_Side *s, int64_t price);
void side_remove_level(Book_Side *s, Price_Level *lv);

#endif
