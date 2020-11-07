
#include "test.h"

int main() {
    return RUN_TESTS(
        TEST(an_empty_test_sould_succeed, {
        }),
        TEST(one_should_be_equal_to_one, {
            ASSERT(1 == 1);
        })
    );
}
