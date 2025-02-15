#include <munit.h>

/* Forward declarations of test suites */
extern "C" {
    extern MunitSuite context_suite;
    extern MunitSuite resource_suite;
    extern MunitSuite error_suite;
    extern MunitSuite memory_suite;
    extern MunitSuite backend_suite;
}

static const char* suite_name = "/labfont";

int main(int argc, char* argv[]) {
    MunitSuite suites[] = {
        context_suite,
        resource_suite,
        error_suite,
        memory_suite,
        backend_suite,
        { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
    };

    MunitSuite main_suite = {
        (char*)suite_name,
        NULL,
        suites,
        1,
        MUNIT_SUITE_OPTION_NONE
    };

    return munit_suite_main(&main_suite, NULL, argc, argv);
}
