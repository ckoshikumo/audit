

#ifndef AUDIT_PASS_CHECK_STR
#define AUDIT_PASS_CHECK_STR "."
#endif // AUDIT_PASS_CHECK_STR

#ifndef AUDIT_FAIL_CHECK_STR
#define AUDIT_FAIL_CHECK_STR "X"
#endif // AUDIT_FAIL_CHECK_STR

#ifdef AUDIT_NO_COLORS
#define AUDIT_COLOR_FAIL
#define AUDIT_COLOR_OK
#define AUDIT_COLOR_INFO
#define AUDIT_COLOR_RESET
#else // AUDIT_NO_COLORS
#ifndef AUDIT_COLOR_FAIL
#define AUDIT_COLOR_FAIL "\x1b[31m"
#endif
#ifndef AUDIT_COLOR_OK
#define AUDIT_COLOR_OK "\x1b[32m"
#endif
#ifndef AUDIT_COLOR_INFO
#define AUDIT_COLOR_INFO "\x1b[33m"
#endif
#define AUDIT_COLOR_RESET "\x1b[0m"
#endif // AUDIT_NO_COLORS

// You may define these, but they are not super relevant. Just some initial
// values, audit will resize the arrays as needed.
#ifndef AUDIT_INITIAL_N_TESTS
#define AUDIT_INITIAL_N_TESTS 50
#endif

#ifndef AUDIT_INITIAL_N_CHECKS
#define AUDIT_INITIAL_N_CHECKS 100
#endif

#ifndef AUDIT_INITIAL_N_MESSAGES
#define AUDIT_INITIAL_N_MESSAGES 100
#endif