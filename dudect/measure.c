#include "measure.h"
#include "dudect.h"
#include "queue.h"
#include "random.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_TRIES 10

/* Maintain a queue independent from the qtest since
 * we do not want the test to affect the original functionality
 */
static uint8_t s[CHUNK_SIZE];

static uint8_t compute_insert_head(void *priv, size_t size, struct list_head *l)
{
    q_insert_head(l, (char *) s);
    return 0;
}

static uint8_t compute_insert_tail(void *priv, size_t size, struct list_head *l)
{
    q_insert_tail(l, (char *) s);
    return 0;
}

static uint8_t compute_remove_head(void *priv, size_t size, struct list_head *l)
{
    element_t *elem = q_remove_head(l, (char *) s, size);
    q_release_element(elem);
    return 0;
}

static uint8_t compute_remove_tail(void *priv, size_t size, struct list_head *l)
{
    element_t *elem = q_remove_tail(l, (char *) s, size);
    q_release_element(elem);
    return 0;
}

static void release_queue(struct list_head *l)
{
    element_t *node = NULL, *safe = NULL;
    list_for_each_entry_safe (node, safe, l, list) {
        q_release_element(node);
    }
    INIT_LIST_HEAD(l);
}

static void random_string(uint8_t *buf, size_t len)
{
    uint8_t rand;
    for (size_t i = 0; i < len - 1; i++) {
        randombytes(&rand, 1);
        buf[i] = 'a' + (rand % 26);
    }
    buf[len - 1] = '\0';
}

static void fixed_queue(struct list_head *l)
{
    if (!list_empty(l))
        release_queue(l);
    q_insert_head(l, (char *) s);
}
static void random_queue(struct list_head *l)
{
    if (!list_empty(l))
        release_queue(l);
    uint8_t buf[1];
    randombytes(buf, 1);
    uint8_t len = buf[0];
    for (uint16_t i = 0; i < len + 1; i++) {
        q_insert_head(l, (char *) s);
    }
}

static void prepare_all(void *priv,
                        dudect_config_t *cfg,
                        struct list_head *input_data,
                        uint8_t *classes)
{
    for (size_t i = 0; i < N_MEASURES; i++) {
        struct list_head *ptr = &input_data[i];
        classes[i] = randombit();
        if (classes[i] == 0)
            fixed_queue(ptr);
        else
            random_queue(ptr);
    }
}

#define GEN_DUDECT_CONFIG(op)                                       \
    static dudect_config_t config_##op = {.prepare = prepare_all,   \
                                          .compute = compute_##op,  \
                                          .priv = NULL,             \
                                          .chunk_size = CHUNK_SIZE, \
                                          .number_measurements = N_MEASURES}

GEN_DUDECT_CONFIG(insert_head);
GEN_DUDECT_CONFIG(insert_tail);
GEN_DUDECT_CONFIG(remove_head);
GEN_DUDECT_CONFIG(remove_tail);

#define GEN_TEST_FUNC(op)                                           \
    bool is_##op##_const()                                          \
    {                                                               \
        for (int i = 0; i < TEST_TRIES; i++) {                      \
            printf("Testing %s...(%d/%d)\n\n", #op, i, TEST_TRIES); \
            random_string(s, CHUNK_SIZE);                           \
            dudect_ctx_t ctx;                                       \
            dudect_init(&ctx, &config_##op);                        \
            dudect_state_t state = DUDECT_NOT_ENOUGHT_MEASUREMENTS; \
            while (state == DUDECT_NOT_ENOUGHT_MEASUREMENTS)        \
                state = dudect_main(&ctx);                          \
            dudect_free(&ctx);                                      \
            printf("\033[A\033[2K\033[A\033[2K");                   \
            if (state == DUDECT_NO_LEAKAGE_EVIDENCE_YET)            \
                return true;                                        \
        }                                                           \
        return false;                                               \
    }

GEN_TEST_FUNC(insert_head)
GEN_TEST_FUNC(insert_tail)
GEN_TEST_FUNC(remove_head)
GEN_TEST_FUNC(remove_tail)