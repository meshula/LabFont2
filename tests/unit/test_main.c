#include <munit.h>

/* Forward declarations of test suites */
extern MunitSuite context_suite;
extern MunitSuite resource_suite;
extern MunitSuite error_suite;

int main(int argc, char* argv[]) {
    MunitSuite suites[] = {
        context_suite,
        resource_suite,
        error_suite,
        { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
    };

    MunitSuite main_suite = {
        "/labfont",
        NULL,
        suites,
        1,
        MUNIT_SUITE_OPTION_NONE
    };

    return munit_suite_main(&main_suite, NULL, argc, argv);
}
