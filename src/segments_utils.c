#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "dtp_log.h"
#include "segments_utils.h"

typedef struct segment_t segment_t;
typedef struct segment_t {
    uint32_t start;
    uint32_t end;
    struct segment_t *next;
} segment_t;
typedef segment_t segments_t;

typedef struct segments_ctx_t {
    segments_t *segments;
    segment_t *cur_segment;
    uint32_t nof_segments;
    bool started;
} segments_ctx_t;

static void append_segment(segments_ctx_t *ctx) {
    segment_t *runner = ctx->segments;
    ctx->nof_segments++;
    if(!runner) {
        ctx->segments = ctx->cur_segment;
    } else {
        while(runner->next) {
            runner = runner->next;
        }
        runner->next = ctx->cur_segment;
    }
}

extern segments_ctx_t *init_segments_ctx(void) {
    segments_ctx_t *res = calloc(1, sizeof(segments_ctx_t));
    return res;
}

bool update_segments(segments_ctx_t *ctx, uint32_t received) {
    if (false == ctx->started) {
        /* First received packet, may or may not be the actual first one sent */
        ctx->nof_segments = 0;
        ctx->started = true;
        ctx->cur_segment = malloc(sizeof(segment_t));
        if (0 == ctx->cur_segment) {
            return false;
        }
        ctx->cur_segment->start = received;
        ctx->cur_segment->end = received;
        ctx->cur_segment->next = 0;
    } else {
        if((ctx->cur_segment->end + 1) == received) {
            /* Consecutive chunk, good */
            ctx->cur_segment->end = received;
        } else {
            /* Missed some chunks, close current segment and create a new one */
            append_segment(ctx);
            ctx->cur_segment = malloc(sizeof(segment_t));
            if (0 == ctx->cur_segment) {
                return false;
            }
            ctx->cur_segment->start = received;
            ctx->cur_segment->end = received;
            ctx->cur_segment->next = 0;
        }
    }
    return true;
}

void close_segments(segments_ctx_t *ctx) {
    if(ctx->cur_segment) {
        append_segment(ctx);
    }
}

segments_ctx_t *get_complement_segment(segments_ctx_t *ctx) {
    segments_ctx_t *complements = init_segments_ctx();
    uint32_t start = 0;
    uint32_t end = 0;
    if(0 != ctx->segments) {
        segment_t *runner  = ctx->segments;
        while(runner) {
            if(runner->start == 0) {
                start = runner->end;
                runner = runner->next;
                continue;
            } else if(start < runner->start) {
                end = runner->start;
                complements->cur_segment = malloc(sizeof(segment_t));
                complements->cur_segment->start = start;
                complements->cur_segment->end = end;
                complements->cur_segment->next = 0;
                append_segment(complements);
                start = runner->end;
            }
            runner = runner->next;
        }
    }
    if (end != 0 && end < start) {
        complements->cur_segment = malloc(sizeof(segment_t));
        complements->cur_segment->start = start;
        complements->cur_segment->end = 0xffffffff;
        complements->cur_segment->next = 0;
        append_segment(complements);
    }
    return complements;
}

void print_segments(segments_ctx_t *ctx) {
    if(0 != ctx->segments) {
        segment_t *runner  = ctx->segments;
        uint32_t i = 0;
        while(runner) {
            dbg_log("Segment #%d, start=%d, end=%d", i++, runner->start, runner->end);
            runner = runner->next;
        }
    }
}

uint32_t get_nof_segments(segments_ctx_t *ctx) {
    return ctx->nof_segments;
}

void free_segments(segments_ctx_t *ctx) {
    if(0 != ctx && 0 != ctx->segments) {
        segment_t *runner  = ctx->segments;
        segment_t *tmp;
        while(runner) {
            tmp = runner;
            runner = runner->next;
            free(tmp);
        }
    }
    free(ctx);
}

void for_each_segment(segments_ctx_t *ctx, void (*cb)(uint32_t idx, uint32_t start, uint32_t end, void *cb_ctx), void *cb_ctx) {
    if(0 != ctx->segments) {
        segment_t *runner  = ctx->segments;
        uint32_t i = 0;
        while(runner) {
            cb(i, runner->start, runner->end, cb_ctx);
            runner = runner->next;
        }
    }
}