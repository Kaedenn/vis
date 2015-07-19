#include "../klist.h"
#include "../helper.h"
#include "../defines.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int asserts = 0;

#define ASSERT_TRUE(cond) \
do { \
    asserts += 1; \
    if (!(cond)) { \
        fprintf(stderr, "Condition FAILED: %s\n", #cond); \
        assert(cond); \
    } else { \
        fprintf(stderr, "Condition passed: %s\n", #cond); \
    } \
} while (0)

#define ASSERT_STR_EQ(x, y) ASSERT_TRUE(!strcmp((x), (y)))

void assert_empty_or_not(klist l, BOOL empty) {
    ASSERT_TRUE(empty == klist_empty(l));
    ASSERT_TRUE(empty == (klist_length(l) == 0));
    if (empty) {
        ASSERT_TRUE(klist_shift(l) == NULL);
        ASSERT_TRUE(klist_pop(l) == NULL);
        ASSERT_TRUE(klist_empty(l));
        ASSERT_TRUE(klist_length(l) == 0);
    }
}

int main(void) {
    klist l;
    const char* s = "Hi mom";
    const char* s2 = "Sup?";
    char* copy = NULL;
    
    /* test set 1: empty-ness */
    l = klist_new();
    assert_empty_or_not(l, TRUE);
    klist_free(l);

    /* test set 2: single item */
    l = klist_new();
    klist_unshift(l, s);
    assert_empty_or_not(l, FALSE);
    ASSERT_TRUE(klist_length(l) == 1);
    ASSERT_STR_EQ(klist_getfirst(l), s);
    ASSERT_STR_EQ(klist_getlast(l), s);
    ASSERT_STR_EQ(klist_getn(l, 0), s);
    copy = klist_shift(l);
    ASSERT_TRUE(klist_empty(l));
    ASSERT_TRUE(klist_length(l) == 0);
    ASSERT_TRUE(klist_shift(l) == NULL);
    ASSERT_TRUE(klist_pop(l) == NULL);
    ASSERT_TRUE(klist_empty(l));
    ASSERT_TRUE(klist_length(l) == 0);
    ASSERT_STR_EQ(copy, s);
    DBFREE(copy);
    klist_free(l);

    /* test set 3: multiple items */
    l = klist_new();
    klist_unshift(l, s);
    klist_unshift(l, s2);
    assert_empty_or_not(l, FALSE);
    ASSERT_TRUE(klist_length(l) == 2);
    ASSERT_STR_EQ(klist_getfirst(l), s2);
    ASSERT_STR_EQ(klist_getlast(l), s);
    ASSERT_STR_EQ(klist_getn(l, 0), s2);
    ASSERT_STR_EQ(klist_getn(l, 1), s);
    ASSERT_TRUE(klist_getn(l, 0) == klist_getfirst(l));
    ASSERT_TRUE(klist_getn(l, 1) == klist_getlast(l));
    copy = klist_pop(l);
    ASSERT_STR_EQ(copy, s);
    DBFREE(copy);
    ASSERT_TRUE(klist_getn(l, 0) == klist_getfirst(l));
    ASSERT_TRUE(klist_getn(l, 0) == klist_getlast(l));
    copy = klist_pop(l);
    ASSERT_STR_EQ(copy, s2);
    DBFREE(copy);
    klist_free(l);

    fprintf(stderr, "Passed %d asserts\n", asserts);

    return 0;
}
