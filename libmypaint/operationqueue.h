#ifndef OPERATIONQUEUE_H
#define OPERATIONQUEUE_H

#include <stdint.h>
#include "tilemap.h"

typedef struct {
    double x;
    double y;
    double radius;
    uint16_t color_r;
    uint16_t color_g;
    uint16_t color_b;
    double color_a;
    double opaque;
    double hardness;
    double aspect_ratio;
    double angle;
    double normal;
    double lock_alpha;
    double colorize;
    double posterize;
    double posterize_num;
    double paint;
} OperationDataDrawDab;

typedef struct OperationQueue OperationQueue;

OperationQueue *operation_queue_new(void);
void operation_queue_free(OperationQueue *self);

int operation_queue_get_dirty_tiles(OperationQueue *self, TileIndex** tiles_out);
void operation_queue_clear_dirty_tiles(OperationQueue *self);

void operation_queue_add(OperationQueue *self, TileIndex index, OperationDataDrawDab *op);
OperationDataDrawDab *operation_queue_pop(OperationQueue *self, TileIndex index);

OperationDataDrawDab *operation_queue_peek_first(OperationQueue *self, TileIndex index);
OperationDataDrawDab *operation_queue_peek_last(OperationQueue *self, TileIndex index);

#endif // OPERATIONQUEUE_H
