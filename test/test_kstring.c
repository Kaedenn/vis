
#include "../kstring.h"
#include "../helper.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

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

int main(void) {
    kstr s1 = kstring_new(1);
    ASSERT_TRUE(kstring_length(s1) == 0);
    ASSERT_TRUE(kstring_capacity(s1) == 1);
    kstr s2 = kstring_newfrom("this is a string");
    kstr s3 = kstring_newfromvf("this is a %s", "string");
    ASSERT_STR_EQ(kstring_content(s2), kstring_content(s3));

    kstring_free(s3);
    kstring_free(s2);
    kstring_free(s1);

    s1 = kstring_newfrom("test");
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    kstring_append(s1, "ing");
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    ASSERT_STR_EQ(kstring_content(s1), "testing");
    kstring_assimilate(s1, kstring_newfrom(" 1 2 3"));
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    ASSERT_STR_EQ(kstring_content(s1), "testing 1 2 3");
    kstring_free(s1);
    
    s1 = kstring_newfrom("testing");
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    kstring_appendvf(s1, " %d %d %d", 1, 2, 3);
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    ASSERT_STR_EQ(kstring_content(s1), "testing 1 2 3");
    size_t cap = kstring_capacity(s1);
    kstring_realloc(s1, cap*2);
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    ASSERT_TRUE(kstring_capacity(s1) == cap*2);
    ASSERT_STR_EQ(kstring_content(s1), "testing 1 2 3");
    kstring_free(s1);

    s1 = kstring_newfrom("  test  ");
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    kstring_stripl(s1);
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    ASSERT_STR_EQ(kstring_content(s1), "test  ");
    kstring_stripr(s1);
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    ASSERT_STR_EQ(kstring_content(s1), "test");
    kstring_free(s1);

    fprintf(stderr, "Passed %d asserts\n", asserts);

    return 0;
}
