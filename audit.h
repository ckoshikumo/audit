#ifndef INCLUDE_AUDIT_H
#define INCLUDE_AUDIT_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef AUDIT_PASS_ASSERT_STR
#define AUDIT_PASS_ASSERT_STR "."
#endif // AUDIT_PASS_ASSERT_STR

#ifndef AUDIT_FAIL_ASSERT_STR
#define AUDIT_FAIL_ASSERT_STR "X"
#endif // AUDIT_FAIL_ASSERT_STR

#ifdef AUDIT_NO_COLORS
#define AUDIT_COLOR_FAIL
#define AUDIT_COLOR_OK
#define AUDIT_COLOR_INFO
#define AUDIT_RESET
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
#define AUDIT_RESET "\x1b[0m"
#endif // AUDIT_NO_COLORS

// INTERFACE:
#define audit(...) _audit_def(audit_, _audit_narg(__VA_ARGS__))(__VA_ARGS__)

#define review(_assert, _msg, ...)                                                                 \
	do {                                                                                       \
		audit_assert_count++;                                                              \
		if (_assert) {                                                                     \
			audit_store_assert_result(true);                                           \
			break;                                                                     \
		}                                                                                  \
		audit_store_assert_result(false);                                                  \
		if (audit_first_failed_assert) {                                                   \
			audit_first_failed_assert = false;                                         \
			audit_failed_count++;                                                \
			audit_store_message(AUDIT_COLOR_INFO "\n%i: %s" AUDIT_RESET, this->n,      \
					    this->name);                                           \
		}                                                                                  \
		audit_failed_asserts_count++;                                                      \
		audit_store_message("\tline %i: " _msg, __LINE__, ##__VA_ARGS__);                  \
	} while (0)

// INTERNALS:

// You may define these, but they are not super relevant. Just some initial
// values, audit will resize the arrays as needed.
#define AUDIT_INITIAL_N_TESTS 50
#define AUDIT_INITIAL_N_ASSERTS 100
#define AUDIT_INITIAL_N_MESSAGES 100

// Macro trickery to make default arguments work:
#define _audit_narg(...) _audit_arg_1(__VA_ARGS__, _audit_rseq_n())
#define _audit_arg_1(...) _audit_arg_n(__VA_ARGS__)
#define _audit_arg_n(_1, _2, _3, _4, N, ...) N
#define _audit_rseq_n() 4, 3, 2, 1, 0

#define _audit_def_1(_name, _n) _##_name##_n
#define _audit_def(_name, _n) _audit_def_1(_name, _n)

#define _audit_1(_name) _audit_internal(_name, NULL, NULL, __LINE__)
#define _audit_2(_name, _setup) _audit_internal(_name, _setup, NULL, __LINE__)
#define _audit_3(_name, _setup, _teardown) _audit_internal(_name, _setup, _teardown, __LINE__)

// Maybe I could consolidate these with _audit_def, but I don't want to think about it.
#define _audit_concat_1(v1, v2) v1##v2
#define _audit_concat(v1, v2) _audit_concat_1(v1, v2)

typedef struct audit_v audit_v;
typedef void (*audit_check_fn)(audit_v *this);
typedef void (*audit_setup_teardown)(void);

typedef struct audit_v {
	char *name;
	int n;
	audit_check_fn fn;
	audit_setup_teardown setup;
	audit_setup_teardown teardown;
} audit_v;

#define _audit_internal(_name, _setup, _teardown, _line)                                           \
	void _audit_concat(audit_test__, _line)(audit_v * this);                                   \
	__attribute__((constructor)) static void _audit_concat(audit_init_, _line)(void)           \
	{                                                                                          \
		audit_register_test(_name, _audit_concat(audit_test__, _line), _setup, _teardown); \
	}                                                                                          \
	void _audit_concat(audit_test__, _line)(audit_v * this)

extern audit_v *audit_tests;
extern size_t audit_tests_count;
extern size_t audit_tests_max;

extern size_t *audit_selected;
extern size_t audit_selected_count;
extern size_t audit_selected_max;

extern char **audit_messages;
extern size_t audit_messages_count;
extern size_t audit_messages_max;

extern char *audit_assert_results;
extern size_t audit_assert_results_count;
extern size_t audit_assert_results_max;

extern size_t audit_assert_count;
extern size_t audit_failed_count;
extern size_t audit_failed_asserts_count;
extern bool audit_first_failed_assert;

void audit_register_test(char *name, audit_check_fn fn, audit_setup_teardown setup,
			 audit_setup_teardown teardown);
void audit_store_message(const char *fmt, ...);
void audit_store_assert_result(bool res);

#endif // INCLUDE_AUDIT_H

#ifdef AUDIT_IMPLEMENTATION

audit_v *audit_tests = NULL;
size_t audit_tests_count = 0;
size_t audit_tests_max = 0;

size_t *audit_selected = NULL;
size_t audit_selected_count = 0;
size_t audit_selected_max = 0;

char **audit_messages = NULL;
size_t audit_messages_count = 0;
size_t audit_messages_max = 0;

char *audit_assert_results = NULL;
size_t audit_assert_results_count = 0;
size_t audit_assert_results_max = 0;

size_t audit_assert_count = 0;
size_t audit_failed_count = 0;
size_t audit_failed_asserts_count = 0;
bool audit_first_failed_assert = true;

#define audit_ensure_capacity(_name)                                                               \
	do {                                                                                       \
		if (_audit_concat(_name, _count) == _audit_concat(_name, _max)) {                  \
			size_t new_max = _audit_concat(_name, _max) * 2;                           \
			_name = realloc(_name, sizeof(*_name) * new_max);                          \
			if (!_name) {                                                              \
				exit(EXIT_FAILURE);                                                \
			}                                                                          \
			_audit_concat(_name, _max) = new_max;                                      \
		}                                                                                  \
	} while (0)

void audit_register_test(char *name, audit_check_fn fn, audit_setup_teardown setup,
			 audit_setup_teardown teardown)
{
	static int counter = 0;

	audit_ensure_capacity(audit_tests);
	audit_tests[audit_tests_count++] =
	    (audit_v){.name = name, .n = counter++, .fn = fn, .setup = setup, .teardown = teardown};
}

void audit_store_message(const char *fmt, ...)
{
	va_list args, args_copy;
	va_start(args, fmt);

	va_copy(args_copy, args);
	size_t s = (size_t)vsnprintf(NULL, 0, fmt, args_copy) + 1;
	va_end(args_copy);

	char *str = malloc(s);
	if (!str) {
		exit(EXIT_FAILURE);
	}

	vsnprintf(str, s, fmt, args);
	va_end(args);

	audit_ensure_capacity(audit_messages);
	audit_messages[audit_messages_count++] = str;
}

void audit_store_assert_result(bool res)
{
	audit_ensure_capacity(audit_assert_results);
	audit_assert_results[audit_assert_results_count++] = res;
}

void audit_print_dots(void)
{
	for (size_t i = 0; i < audit_assert_results_count; i++) {
		if (i % 80 == 0) {
			printf("\n");
		}

		if (audit_assert_results[i]) {
			printf(AUDIT_COLOR_OK AUDIT_PASS_ASSERT_STR AUDIT_RESET);
		} else {
			printf(AUDIT_COLOR_FAIL AUDIT_FAIL_ASSERT_STR AUDIT_RESET);
		}
	}
	printf("\n\n");
}

void audit_print_failures(void)
{
	if (audit_failed_count == 0) {
		printf(AUDIT_COLOR_OK "AUDIT OK\n" AUDIT_RESET);
		return;
	}

	printf(AUDIT_COLOR_FAIL "AUDIT FAILED\n" AUDIT_RESET);

	for (size_t i = 0; i < audit_messages_count; i++) {
		printf("%s\n", audit_messages[i]);
	}
	printf("\n");
}

void audit_print_summary(void)
{
	if (audit_failed_count == 0) {
		printf(AUDIT_COLOR_OK);
	} else {
		printf(AUDIT_COLOR_FAIL);
	}

	printf("%zu tests (%zu failed), %zu assertions (%zu failed)" AUDIT_RESET "\n\n",
	       audit_tests_count, audit_failed_count, audit_assert_count,
	       audit_failed_asserts_count);
}

void audit_print_results(void)
{
	audit_print_dots();
	audit_print_failures();
	audit_print_summary();
}

void audit_run(size_t test_n)
{
	audit_setup_teardown setup;
	audit_setup_teardown teardown;

	audit_first_failed_assert = true;
	setup = audit_tests[test_n].setup;
	teardown = audit_tests[test_n].teardown;

	if (setup) {
		setup();
	}

	audit_tests[test_n].fn(&audit_tests[test_n]);

	if (teardown) {
		teardown();
	}
}

void audit_run_selected(void)
{
	for (size_t i = 0; i < audit_selected_count; i++) {
		size_t test_n = audit_selected[i];
		if (test_n > audit_tests_count) {
			printf(AUDIT_COLOR_FAIL "Test %lu doesn't exist", i);
			return;
		} else {
			printf(AUDIT_COLOR_INFO "%lu: %s" AUDIT_RESET "\n", test_n,
			       audit_tests[test_n].name);
		}
	}

	for (size_t i = 0; i < audit_selected_count; i++) {
		audit_run(audit_selected[i]);
	}
}

void audit_run_all(void)
{
	for (size_t i = 0; i < audit_tests_count; i++) {
		audit_run(i);
	}
}

void audit_print_available(void)
{
	for (size_t i = 0; i < audit_tests_count; i++) {
		printf(AUDIT_COLOR_INFO "%i: %s" AUDIT_RESET "\n", audit_tests[i].n,
		       audit_tests[i].name);
	}
}

void audit_choose(char *test_n)
{
	char *end = NULL;
	size_t n = (size_t)strtol(test_n, &end, 10);

	if (*end) {
		printf(AUDIT_COLOR_FAIL "\nERROR: " AUDIT_RESET
					"Couldn't load argument as test number: %s"
					"\n\n",
		       test_n);
		return;
	}

	if (n >= audit_tests_count) {
		printf(AUDIT_COLOR_FAIL "Test %lu doesn't exist." AUDIT_RESET "\n", n);
		printf("Run audit --list to see available tests.\n");
		return;
	}

	audit_ensure_capacity(audit_selected);
	audit_selected[audit_selected_count++] = n;
}

void audit_free_resources(void)
{
	free(audit_tests);
	free(audit_selected);
	// TODO: Actually free all the string messages:
	free(audit_messages);
	free(audit_assert_results);
}

__attribute__((constructor)) static void audit_init(void)
{
	audit_tests = malloc(sizeof(*audit_tests) * AUDIT_INITIAL_N_TESTS);
	audit_tests_max = AUDIT_INITIAL_N_TESTS;

	audit_selected = malloc(sizeof(*audit_selected) * AUDIT_INITIAL_N_TESTS);
	audit_selected_max = AUDIT_INITIAL_N_TESTS;

	audit_messages = malloc(sizeof(*audit_messages) * AUDIT_INITIAL_N_MESSAGES);
	audit_messages_max = AUDIT_INITIAL_N_MESSAGES;

	audit_assert_results = malloc(sizeof(*audit_assert_results) * AUDIT_INITIAL_N_ASSERTS);
	audit_assert_results_max = AUDIT_INITIAL_N_ASSERTS;

	if (!audit_tests || !audit_selected || !audit_messages || !audit_assert_results) {
		exit(EXIT_FAILURE);
	}
}
int main(int argc, char **argv)
{
	atexit(audit_free_resources);
	bool tried_to_select = false;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--list") == 0) {
			audit_print_available();
			return 0;
		}

		tried_to_select = true;
		audit_choose(argv[i]);
	}

	if (tried_to_select && audit_selected_count == 0) {
		printf(AUDIT_COLOR_FAIL "Couldn't run any tests." AUDIT_RESET "\n");
		return -1;
	}

	printf(AUDIT_COLOR_OK "AUDIT START" AUDIT_RESET "\n\n");

	if (audit_selected_count > 0) {
		printf("Running selected tests:\n\n");
		audit_run_selected();
	} else {
		printf("Running all tests.\n");
		audit_run_all();
	}

	audit_print_results();

	return audit_failed_count == 0 ? 0 : -1;
}

#endif // AUDIT_IMPLEMENTATION
