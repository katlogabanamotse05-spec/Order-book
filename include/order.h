#ifndef ORDER_H
#define ORDER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
typedef enum { SIDE_BUY,
               SIDE_SELL,
} Side;

struct Price_Level;

typedef struct Order {
    uint64_t id;
    Side side;
    int64_t price;
    uint32_t qty;
    uint64_t seq;
    struct Order *prev;
    struct Order *next;
    struct Price_Level *level;
} Order; 

typedef enum { ORDER_OK,
               ORDER_BAD_ID,
               ORDER_BAD_PRICE,
               ORDER_BAD_QTY,
               ORDER_NOMEM
} Order_Status;
Order * order_create(uint64_t id, 
                    Side side, int64_t price,
                    uint32_t qty, Order_Status *status);

void order_destroy(Order *order);

#endif 
