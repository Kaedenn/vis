
#include "../kstring.h"
#include "../helper.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

void asserteq_s(const char* s1, const char* s2) {
    DBPRINTF("\"%s\" ?= \"%s\"", s1, s2);
    assert(!strcmp(s1, s2));
}

int main(void) {
    kstr s1 = kstring_new(1);
    assert(kstring_length(s1) == 0);
    assert(kstring_capacity(s1) == 1);
    kstr s2 = kstring_newfrom("this is a string");
    kstr s3 = kstring_newfromvf("this is a %s", "string");
    asserteq_s(kstring_content(s2), kstring_content(s3));

    kstring_free(s3);
    kstring_free(s2);
    kstring_free(s1);

    s1 = kstring_newfrom("test");
    assert(kstring_content(s1)[kstring_length(s1)] == '\0');
    kstring_append(s1, "ing");
    assert(kstring_content(s1)[kstring_length(s1)] == '\0');
    asserteq_s(kstring_content(s1), "testing");
    kstring_assimilate(s1, kstring_newfrom(" 1 2 3"));
    assert(kstring_content(s1)[kstring_length(s1)] == '\0');
    asserteq_s(kstring_content(s1), "testing 1 2 3");
    kstring_free(s1);
    
    s1 = kstring_newfrom("testing");
    assert(kstring_content(s1)[kstring_length(s1)] == '\0');
    kstring_appendvf(s1, " %d %d %d", 1, 2, 3);
    assert(kstring_content(s1)[kstring_length(s1)] == '\0');
    asserteq_s(kstring_content(s1), "testing 1 2 3");
    size_t cap = kstring_capacity(s1);
    kstring_realloc(s1, cap*2);
    assert(kstring_content(s1)[kstring_length(s1)] == '\0');
    assert(kstring_capacity(s1) == cap*2);
    asserteq_s(kstring_content(s1), "testing 1 2 3");
    kstring_free(s1);

    s1 = kstring_newfrom("  test  ");
    assert(kstring_content(s1)[kstring_length(s1)] == '\0');
    kstring_stripl(s1);
    assert(kstring_content(s1)[kstring_length(s1)] == '\0');
    asserteq_s(kstring_content(s1), "test  ");
    kstring_stripr(s1);
    assert(kstring_content(s1)[kstring_length(s1)] == '\0');
    asserteq_s(kstring_content(s1), "test");
    kstring_free(s1);

    return 0;
}
