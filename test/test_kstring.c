
#include "../kstring.h"
#include "../helper.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

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

#define ASSERT_TRUE_MSG(cond, msg) \
    do { \
        asserts += 1; \
        if (!(cond)) { \
            fprintf(stderr, "Condition FAILED: %s: %s\n", #cond, msg); \
            assert(cond); \
        } else { \
            fprintf(stderr, "Condition passed: %s: %s\n", #cond, msg); \
        } \
    } while(0)

#define ASSERT_STR_EQ(x, y) \
    do { \
        if (strcmp((x), (y))) { \
            fprintf(stderr, "Condition FAILED: \"%s\" == \"%s\"\n", (x), (y)); \
            assert(!strcmp((x), (y))); \
        } \
    } while (0)

int main(void) {
    kstr s1 = kstring_new(1);
    /* Test basic operations */
    ASSERT_TRUE(kstring_empty(s1));
    ASSERT_TRUE(kstring_length(s1) == 0);
    ASSERT_TRUE(kstring_capacity(s1) == 1);
    kstr s2 = kstring_newfrom("this is a string");
    kstr s3 = kstring_newfromvf("this is a %s", "string");
    ASSERT_TRUE(!kstring_empty(s2));
    ASSERT_TRUE(!kstring_empty(s3));
    ASSERT_STR_EQ(kstring_content(s2), kstring_content(s3));

    kstring_free(s3);
    kstring_free(s2);
    kstring_free(s1);

    s1 = kstring_newfrom("test");
    /* Test append, assimilate */
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    kstring_append(s1, "ing");
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    ASSERT_STR_EQ(kstring_content(s1), "testing");
    kstring_assimilate(s1, kstring_newfrom(" 1 2 3"));
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    ASSERT_STR_EQ(kstring_content(s1), "testing 1 2 3");
    kstring_free(s1);
    
    s1 = kstring_newfrom("testing");
    /* Test appendvf, realloc */
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
    s2 = kstring_newfrom(kstring_content(s1));
    /* Test stripl, stripr, strip */
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    kstring_stripl(s1);
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    ASSERT_STR_EQ(kstring_content(s1), "test  ");
    kstring_stripr(s1);
    ASSERT_TRUE(kstring_content(s1)[kstring_length(s1)] == '\0');
    ASSERT_STR_EQ(kstring_content(s1), "test");
    kstring_strip(s2);
    ASSERT_STR_EQ(kstring_content(s2), "test");
    kstring_free(s1);
    kstring_free(s2);

    const char* test_string = "abc 123 def";
    size_t len = strlen(test_string);

    s1 = kstring_newfrom(test_string);
    /* Test ltrim, rtrim */
    ASSERT_TRUE(kstring_ltrim(s1, "abc"));
    ASSERT_TRUE(kstring_length(s1) == len - 3);
    ASSERT_STR_EQ(kstring_content(s1), " 123 def");
    ASSERT_TRUE(kstring_rtrim(s1, "def"));
    ASSERT_TRUE(kstring_length(s1) == len - 6);
    ASSERT_STR_EQ(kstring_content(s1), " 123 ");
    kstring_free(s1);

    s1 = kstring_newfrom(test_string);
    /* Test failure to trim (string does not match) */
    ASSERT_TRUE(!kstring_ltrim(s1, "123"));
    ASSERT_TRUE(!kstring_rtrim(s1, "123"));
    ASSERT_STR_EQ(kstring_content(s1), test_string);

    /* Test substr where offset is larger than s1 */
    kstring_substr_l(s1, len + 1);
    ASSERT_STR_EQ(kstring_content(s1), test_string);
    kstring_substr_r(s1, len + 1);
    ASSERT_STR_EQ(kstring_content(s1), test_string);

    kstring_free(s1);

    fprintf(stderr, "Passed %d asserts\n", asserts);

    return 0;
}
