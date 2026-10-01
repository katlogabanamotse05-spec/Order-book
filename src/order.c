#include "order.h"
#include <stdlib.h>
#include <assert.h>

static void set_status(Order_Status *status, Order_Status value) {
    if (status != NULL) {
        *status = value;
    }
}
Order * order_create(uint64_t id, 
                    Side side, int64_t price,
                    uint32_t qty, Order_Status *status
){
    if (id == 0){
        set_status(status,ORDER_BAD_ID);
        return NULL;
        } 
    if (price <= 0 ){
            set_status(status, ORDER_BAD_PRICE);
            return NULL;
    }
    if (qty == 0) {
        set_status(status, ORDER_BAD_QTY);
        return NULL;
        }

    Order *result = calloc(1,sizeof(Order));
    if (result == NULL){
        set_status(status, ORDER_NOMEM);
        return NULL;
    }
    result -> id = id;
    result->side = side;
    result->price = price;
    result->qty = qty;
    set_status(status, ORDER_OK);

    return result;
}
void order_destroy(Order *order){
    assert(order != NULL);
    free(order);
}
