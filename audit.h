#ifndef INCLUDE_AUDIT_H
#define INCLUDE_AUDIT_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef AUDIT_PASS_ASSERT_STR_
#define AUDIT_PASS_ASSERT_STR_ "."
#endif // AUDIT_PASS_ASSERT_STR_

#ifndef AUDIT_FAIL_ASSERT_STR_
#define AUDIT_FAIL_ASSERT_STR_ "X"
#endif // AUDIT_FAIL_ASSERT_STR_

#ifdef AUDIT_NO_COLORS
#define AUDIT_FAIL_
#define AUDIT_OK_
#define AUDIT_INFO_
#define AUDIT_RESET_
#else // AUDIT_NO_COLORS
#define AUDIT_FAIL_ "\x1b[31m"
#define AUDIT_OK_ "\x1b[32m"
#define AUDIT_INFO_ "\x1b[33m"
#define AUDIT_RESET_ "\x1b[0m"
#endif // AUDIT_NO_COLORS

// INTERFACE:
#define audit(...) audit_def_1(audit_, audit_narg(__VA_ARGS__))(__VA_ARGS__)

#define review(assert__, msg__, ...)                                                               \
	do {                                                                                       \
		audit_assert_count++;                                                              \
		if (assert__) {                                                                    \
			audit_store_assert_result(true);                                           \
			break;                                                                     \
		}                                                                                  \
		audit_store_assert_result(false);                                                  \
		if (audit_first_failed_assert) {                                                   \
			audit_first_failed_assert = false;                                         \
			audit_failed_tests_count++;                                                \
			audit_store_message(AUDIT_INFO_ "\n%i: %s" AUDIT_RESET_, this->n,          \
					    this->name);                                           \
		}                                                                                  \
		audit_failed_asserts_count++;                                                      \
		audit_store_message("\tline %i: " msg__, __LINE__, ##__VA_ARGS__);                 \
	} while (0)

// INTERNALS:

// Macro trickery to make default arguments work:
#define audit_narg(...) audit_arg_1(__VA_ARGS__, audit_rseq_n())
#define audit_arg_1(...) audit_arg_n(__VA_ARGS__)
#define audit_arg_n(_1, _2, _3, _4, N, ...) N
#define audit_rseq_n() 4, 3, 2, 1, 0
#define audit_def_2(_name, _n) _name##_n
#define audit_def_1(_name, _n) audit_def_2(_name, _n)

#define audit_1(_name) audit_internal(_name, NULL, NULL, __LINE__)
#define audit_2(_name, _setup) audit_internal(_name, _setup, NULL, __LINE__)
#define audit_3(_name, _setup, _teardown) audit_internal(_name, _setup, _teardown, __LINE__)

// These are not super relevant; they're just some initial values.
// Audit will resize the arrays as needed.
#define AUDIT_INITIAL_N_TESTS 50
#define AUDIT_INITIAL_N_ASSERTS 100
#define AUDIT_INITIAL_N_MESSAGES 100

#define AUDIT_CONCAT2(v1, v2) v1##v2
#define AUDIT_CONCAT(v1, v2) AUDIT_CONCAT2(v1, v2)

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

#define audit_internal(str_name__, setup__, teardown__, line__)                                    \
	void AUDIT_CONCAT(audit_test__, line__)(audit_v * this);                                   \
	__attribute__((constructor)) static void AUDIT_CONCAT(audit_init_, line__)(void)           \
	{                                                                                          \
		audit_register_test(str_name__, AUDIT_CONCAT(audit_test__, line__), setup__,       \
				    teardown__);                                                   \
	}                                                                                          \
	void AUDIT_CONCAT(audit_test__, line__)(audit_v * this)

extern audit_v *audit_tests;
extern size_t audit_tests_count;
extern size_t audit_tests_max;

extern size_t *audit_chosen_tests;
extern size_t audit_chosen_tests_count;
extern size_t audit_chosen_tests_max;

extern char **audit_messages;
extern size_t audit_messages_count;
extern size_t audit_messages_max;

extern char *audit_assert_results;
extern size_t audit_assert_results_count;
extern size_t audit_assert_results_max;

extern size_t audit_assert_count;
extern size_t audit_failed_tests_count;
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

size_t *audit_chosen_tests = NULL;
size_t audit_chosen_tests_count = 0;
size_t audit_chosen_tests_max = 0;

char **audit_messages = NULL;
size_t audit_messages_count = 0;
size_t audit_messages_max = 0;

char *audit_assert_results = NULL;
size_t audit_assert_results_count = 0;
size_t audit_assert_results_max = 0;

size_t audit_assert_count = 0;
size_t audit_failed_tests_count = 0;
size_t audit_failed_asserts_count = 0;
bool audit_first_failed_assert = true;

#define audit_ensure_capacity(_name)                                                               \
	do {                                                                                       \
		if (AUDIT_CONCAT(_name, _count) == AUDIT_CONCAT(_name, _max)) {                    \
			size_t new_max = AUDIT_CONCAT(_name, _max) * 2;                            \
			_name = realloc(_name, sizeof(*_name) * new_max);                          \
			if (!_name) {                                                              \
				exit(EXIT_FAILURE);                                                \
			}                                                                          \
			AUDIT_CONCAT(_name, _max) = new_max;                                       \
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
			printf(AUDIT_OK_ AUDIT_PASS_ASSERT_STR_ AUDIT_RESET_);
		} else {
			printf(AUDIT_FAIL_ AUDIT_FAIL_ASSERT_STR_ AUDIT_RESET_);
		}
	}
	printf("\n\n");
}

void audit_print_failures(void)
{
	if (audit_failed_tests_count == 0) {
		printf(AUDIT_OK_ "AUDIT OK\n" AUDIT_RESET_);
		return;
	}

	printf(AUDIT_FAIL_ "AUDIT FAILED\n" AUDIT_RESET_);

	for (size_t i = 0; i < audit_messages_count; i++) {
		printf("%s\n", audit_messages[i]);
	}
	printf("\n");
}

void audit_print_summary(void)
{
	if (audit_failed_tests_count == 0) {
		printf(AUDIT_OK_);
	} else {
		printf(AUDIT_FAIL_);
	}

	printf("%zu tests (%zu failed), %zu assertions (%zu failed)" AUDIT_RESET_ "\n\n",
	       audit_tests_count, audit_failed_tests_count, audit_assert_count,
	       audit_failed_asserts_count);
}

void audit_print_results(void)
{
	audit_print_dots();
	audit_print_failures();
	audit_print_summary();
}

void audit_run_test(size_t test_n)
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
	printf("Running selected tests:\n");
	for (size_t i = 0; i < audit_chosen_tests_count; i++) {
		size_t test_n = audit_chosen_tests[i];
		if (test_n > audit_tests_count) {
			printf(AUDIT_FAIL_ "Test %lu doesn't exist", i);
			return;
		} else {
			printf(AUDIT_INFO_ "%lu: %s" AUDIT_RESET_ "\n", test_n,
			       audit_tests[test_n].name);
		}
	}

	for (size_t i = 0; i < audit_chosen_tests_count; i++) {
		audit_run_test(audit_chosen_tests[i]);
	}
}

void audit_run_all(void)
{
	if (audit_chosen_tests_count > 0) {
		audit_run_selected();
		return;
	}

	printf("Running all tests.\n");

	for (size_t i = 0; i < audit_tests_count; i++) {
		audit_run_test(i);
	}
}

void audit_print_available_tests(void)
{
	for (size_t i = 0;; i++) {
		if (!audit_tests[i].name) {
			break;
		}
		printf(AUDIT_INFO_ "%i: %s" AUDIT_RESET_ "\n", audit_tests[i].n,
		       audit_tests[i].name);
	}
}

bool audit_save_test(char *test_n)
{
	char *end = NULL;
	size_t n = (size_t)strtol(test_n, &end, 10);

	if (*end) {
		fprintf(stderr, "Argument can't be read as number: %s\n", test_n);
		exit(EXIT_FAILURE);
	}

	audit_ensure_capacity(audit_chosen_tests);
	audit_chosen_tests[audit_chosen_tests_count++] = n;
	return true;
}

void audit_free_resources(void)
{
	free(audit_tests);
	free(audit_chosen_tests);
	free(audit_messages);
	free(audit_assert_results);
}

__attribute__((constructor)) static void audit_init(void)
{
	audit_tests = malloc(sizeof(*audit_tests) * AUDIT_INITIAL_N_TESTS);
	audit_tests_max = AUDIT_INITIAL_N_TESTS;

	audit_chosen_tests = malloc(sizeof(*audit_chosen_tests) * AUDIT_INITIAL_N_TESTS);
	audit_chosen_tests_max = AUDIT_INITIAL_N_TESTS;

	audit_messages = malloc(sizeof(*audit_messages) * AUDIT_INITIAL_N_MESSAGES);
	audit_messages_max = AUDIT_INITIAL_N_MESSAGES;

	audit_assert_results = malloc(sizeof(*audit_assert_results) * AUDIT_INITIAL_N_ASSERTS);
	audit_assert_results_max = AUDIT_INITIAL_N_ASSERTS;

	if (!audit_tests || !audit_chosen_tests || !audit_messages || !audit_assert_results) {
		exit(EXIT_FAILURE);
	}
}
int main(int argc, char **argv)
{
	atexit(audit_free_resources);

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--list") == 0) {
			audit_print_available_tests();
			return 0;
		}

		if (!audit_save_test(argv[i])) {
			fprintf(stderr, "Test %s not found.\n", argv[i]);
			exit(EXIT_FAILURE);
		};
	}

	printf(AUDIT_OK_ "AUDIT START" AUDIT_RESET_ "\n\n");

	audit_run_all();
	audit_print_results();

	return audit_failed_tests_count == 0 ? 0 : -1;
}

#endif // AUDIT_IMPLEMENTATION
