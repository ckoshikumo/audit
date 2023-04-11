#ifndef INCLUDE_AUDIT_H
#define INCLUDE_AUDIT_H

#include <assert.h>
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

#define audit_setup                                                                                \
	void audit_setup_fn(void);                                                                 \
	__attribute__((constructor)) static void audit_setup_init(void)                            \
	{                                                                                          \
		audit_setup_fn_ptr = audit_setup_fn;                                               \
	}                                                                                          \
	void audit_setup_fn(void)

#define audit_teardown                                                                             \
	void audit_teardown_fn(void);                                                              \
	__attribute__((constructor)) static void audit_teardown_init(void)                         \
	{                                                                                          \
		audit_teardown_fn_ptr = audit_teardown_fn;                                         \
	}                                                                                          \
	void audit_teardown_fn(void)

#define audit(str_name__) audit_internal(str_name__, __LINE__, true)

#define audit_direct(str_name__) audit_internal(str_name__, __LINE__, false)

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
		{                                                                                  \
			audit_store_message("\tline %i: " msg__, __LINE__, ##__VA_ARGS__);         \
			audit_failed_asserts_count++;                                              \
		}                                                                                  \
	} while (0)

// IMPLEMENTATION:

// These are not super relevant; they're just some initial values.
// Audit will resize the arrays as needed.
#define AUDIT_INITIAL_N_TESTS 100
#define AUDIT_INITIAL_N_ASSERTS 100
#define AUDIT_INITIAL_N_MESSAGES 50

#define AUDIT_CONCAT2(v1, v2) v1##v2
#define AUDIT_CONCAT(v1, v2) AUDIT_CONCAT2(v1, v2)

typedef struct audit_v audit_v;
typedef void (*audit_check_fn)(audit_v *this);

typedef struct audit_v {
	char *name;
	int n;
	bool setup;
	audit_check_fn fn;
} audit_v;

#define audit_internal(str_name__, line__, setup__)                                                \
	void AUDIT_CONCAT(audit_test__, line__)(audit_v * this);                                   \
	__attribute__((constructor)) static void AUDIT_CONCAT(audit_init_, line__)(void)           \
	{                                                                                          \
		audit_register_test(str_name__, AUDIT_CONCAT(audit_test__, line__), setup__);      \
		if (setup__) {                                                                     \
			audit_needs_setup = true;                                                  \
		}                                                                                  \
	}                                                                                          \
	void AUDIT_CONCAT(audit_test__, line__)(audit_v * this)

typedef void (*audit_setup_teardown)(void);

static audit_setup_teardown audit_setup_fn_ptr = NULL;
static audit_setup_teardown audit_teardown_fn_ptr = NULL;

static bool audit_needs_setup = false;

static audit_v *audit_tests = NULL;
static size_t audit_tests_count = 0;
static size_t audit_tests_max = 0;

static audit_v *audit_chosen_tests = NULL;
static size_t audit_chosen_tests_count = 0;
static size_t audit_chosen_tests_max = 0;

static char **audit_messages = NULL;
static size_t audit_messages_count = 0;
static size_t audit_messages_max = 0;

static char *audit_assert_results = NULL;
static size_t audit_assert_results_count = 0;
static size_t audit_max_assert_results = 0;

static size_t audit_assert_count = 0;
static size_t audit_failed_tests_count = 0;
static size_t audit_failed_asserts_count = 0;
static bool audit_first_failed_assert = true;

static inline void audit_check_test_count(void);

static inline void audit_register_test(char *name, audit_check_fn fn, bool setup)
{
	static int counter = 0;
	audit_check_test_count();
	audit_tests[audit_tests_count++] =
	    (audit_v){.name = name, .n = counter++, .setup = setup, .fn = fn};
}

static inline void audit_init_tests_array(void)
{
	audit_tests = malloc(sizeof(audit_v) * AUDIT_INITIAL_N_TESTS);
	if (!audit_tests) {
		exit(EXIT_FAILURE);
	}
	audit_tests_max = AUDIT_INITIAL_N_TESTS;
}

static inline void audit_init_chosen_tests_array(void)
{
	audit_chosen_tests = malloc(sizeof(audit_v) * AUDIT_INITIAL_N_TESTS);
	if (!audit_chosen_tests) {
		exit(EXIT_FAILURE);
	}
	audit_chosen_tests_max = AUDIT_INITIAL_N_TESTS;
}

static inline void audit_init_message_array(void)
{
	audit_messages = malloc(sizeof(char *) * AUDIT_INITIAL_N_MESSAGES);
	if (!audit_messages) {
		exit(EXIT_FAILURE);
	}
	audit_messages_max = AUDIT_INITIAL_N_MESSAGES;
}

static inline void audit_init_assert_results_array(void)
{
	audit_assert_results = malloc(sizeof(bool) * AUDIT_INITIAL_N_ASSERTS);
	if (!audit_assert_results) {
		exit(EXIT_FAILURE);
	}
	audit_max_assert_results = AUDIT_INITIAL_N_ASSERTS;
}

static inline void audit_check_message_count(void)
{
	if (audit_messages_count < audit_messages_max) {
		return;
	}

	size_t new_max = audit_messages_max * 2;
	audit_messages = realloc(audit_messages, sizeof(char *) * new_max);
	if (!audit_messages) {
		exit(EXIT_FAILURE);
	}
	audit_messages_max = new_max;
}

static inline void audit_check_test_count(void)
{
	if (audit_tests_count < audit_tests_max) {
		return;
	}

	size_t new_max = audit_tests_max * 2;
	audit_tests = realloc(audit_tests, sizeof(audit_v) * new_max);
	if (!audit_tests) {
		exit(EXIT_FAILURE);
	}
	audit_tests_max = new_max;
}

static inline void audit_check_assert_result_count(void)
{
	if (audit_assert_results_count < audit_max_assert_results) {
		return;
	}

	size_t new_max = audit_max_assert_results * 2;
	audit_assert_results = realloc(audit_assert_results, sizeof(bool) * new_max);
	if (!audit_assert_results) {
		exit(EXIT_FAILURE);
	}
	audit_max_assert_results = new_max;
}

static inline void audit_store_message(const char *fmt, ...)
{
	audit_check_message_count();

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

	audit_messages[audit_messages_count++] = str;
}

static inline void audit_store_assert_result(bool res)
{
	audit_check_assert_result_count();
	audit_assert_results[audit_assert_results_count++] = res;
}

static inline void audit_print_dots(void)
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

static inline void audit_print_failures(void)
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

static inline void audit_print_summary(void)
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

static inline void audit_print_results(void)
{
	audit_print_dots();
	audit_print_failures();
	audit_print_summary();
}

static inline void audit_run_tests(void)
{
	if (audit_needs_setup && !audit_setup_fn_ptr && !audit_teardown_fn_ptr) {
		fprintf(stderr, "\n" AUDIT_FAIL_ "ERROR: No setup or teardown functions "
				"provided." AUDIT_RESET_ "\n\n");
		exit(EXIT_FAILURE);
	}

	if (audit_needs_setup && !audit_setup_fn_ptr) {
		fprintf(stderr,
			"\n" AUDIT_FAIL_ "ERROR: No setup function provided." AUDIT_RESET_ "\n\n");
		exit(EXIT_FAILURE);
	}

	if (audit_needs_setup && !audit_teardown_fn_ptr) {
		fprintf(stderr, "\n" AUDIT_FAIL_
				"ERROR: No teardown function provided." AUDIT_RESET_ "\n\n");
		exit(EXIT_FAILURE);
	}

	audit_v *test_array;

	printf("\n");

	if (audit_chosen_tests_count > 0) {
		test_array = audit_chosen_tests;
		printf("Reviewing selected:\n");
		for (size_t i = 0;; i++) {
			if (!test_array[i].name) {
				break;
			}
			printf(AUDIT_INFO_ "%i: %s" AUDIT_RESET_ "\n", test_array[i].n,
			       test_array[i].name);
		}
	} else {
		test_array = audit_tests;
		printf("Reviewing all.\n");
	}

	bool setup;

	for (size_t i = 0;; i++) {
		if (!test_array[i].fn) {
			break;
		}

		audit_first_failed_assert = true;
		setup = test_array[i].setup;
		if (setup) {
			audit_setup_fn_ptr();
		}
		test_array[i].fn(&audit_tests[i]);
		if (setup) {
			audit_teardown_fn_ptr();
		}
	}
}

static void audit_print_available_tests(void)
{
	for (size_t i = 0;; i++) {
		if (!audit_tests[i].name) {
			break;
		}
		printf(AUDIT_INFO_ "%i: %s" AUDIT_RESET_ "\n", audit_tests[i].n,
		       audit_tests[i].name);
	}
}

static audit_v *audit_get_test_by_index(size_t n)
{
	if (n > audit_tests_count) {
		return NULL;
	}
	return &audit_tests[n];
}

static bool audit_save_test(char *test_n)
{
	char *end = NULL;
	size_t n = (size_t)strtol(test_n, &end, 10);

	if (*end) {
		fprintf(stderr, "Argument can't be read as number: %s\n", test_n);
		exit(EXIT_FAILURE);
	}

	audit_v *test = audit_get_test_by_index(n);
	if (!test) {
		return false;
	}

	audit_chosen_tests[audit_chosen_tests_count++] = *test;
	return true;
}

static inline void audit_free_resources(void)
{
	free(audit_tests);
	free(audit_chosen_tests);
	free(audit_messages);
	free(audit_assert_results);
}

__attribute__((constructor)) static void audit_init(void)
{
	audit_init_tests_array();
	audit_init_chosen_tests_array();
	audit_init_message_array();
	audit_init_assert_results_array();
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

	printf(AUDIT_OK_ "AUDIT START" AUDIT_RESET_ "\n");

	audit_run_tests();
	audit_print_results();

	return audit_failed_tests_count == 0 ? 0 : -1;
}

#endif // INCLUDE_AUDIT_H
