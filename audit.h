#ifndef _TRUST_H_
#define _TRUST_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../types/types.h"

#ifdef TRUST_NO_COLORS
#define TRUST_FAIL_
#define TRUST_OK_
#define TRUST_INFO_
#define TRUST_RESET_
#else
#define TRUST_FAIL_  "\x1b[31m"
#define TRUST_OK_    "\x1b[32m"
#define TRUST_INFO_  "\x1b[33m"
#define TRUST_RESET_ "\x1b[0m"
#endif

// Interface:

#define trust_setup void trust_setup_fn(void);\
	__attribute__((constructor (101))) static void trust_setup_init(void){\
		trust_setup_fn_ptr = trust_setup_fn;\
	} void trust_setup_fn(void)\

#define trust_teardown void trust_teardown_fn(void);\
	__attribute__((constructor (101))) static void trust_teardown_init(void){\
		trust_teardown_fn_ptr = trust_teardown_fn;\
	} void trust_teardown_fn(void)\

#define trust(str_name__) trust_internal(str_name__, __LINE__, true)

#define trust_direct(str_name__) trust_internal(str_name__, __LINE__, false)

#define verify(assert__, msg__, ...) do {\
		trust_assert_count++;\
		if (assert__) { \
			trust_store_dot(true);\
			break;\
		} \
		trust_store_dot(false);\
		if (trust_first_failed_assert) {\
			trust_first_failed_assert = false;\
			trust_failed_tests_count++;\
			trust_store_message(TRUST_INFO_ "\n%s" TRUST_RESET_, str_name);\
		}\
		{\
			trust_store_message("\t[%i] " msg__, __LINE__, ##__VA_ARGS__);\
			trust_failed_asserts_count++;\
		} \
	} while (0)

// Internals:

// These are not super relevant; they're just some initial values.
// Trust will resize the arrays as needed.
#define TRUST_INITIAL_N_TESTS 100
#define TRUST_INITIAL_N_ASSERTS 100
#define TRUST_INITIAL_N_MESSAGES 50

#define TRUST_CONCAT2(v1, v2) v1 ## v2
#define TRUST_CONCAT(v1, v2) TRUST_CONCAT2(v1, v2)

#define trust_internal(str_name__, num__, setup__)\
	void TRUST_CONCAT(trust_test__, num__)(char *str_name);\
	__attribute__((constructor (103))) static void TRUST_CONCAT(trust_init_, num__)(void) {\
		trust_register_test(str_name__, TRUST_CONCAT(trust_test__, num__), setup__);\
		if (setup__) { trust_needs_setup = true; }\
	} void TRUST_CONCAT(trust_test__, num__)(char *str_name)

typedef void (*trust_setup_teardown)(void);

trust_setup_teardown trust_setup_fn_ptr = NULL ;
trust_setup_teardown trust_teardown_fn_ptr = NULL;
bool trust_needs_setup = false;

typedef void (*trust_check_fn)(char *str_name);

typedef struct trust_v {
	char* name;
	bool setup;
	trust_check_fn fn;
} trust_v;

trust_v *trust_tests = NULL;
size_t trust_tests_count = 0;
size_t trust_tests_max = 0;

char **trust_messages = NULL;
size_t trust_messages_count = 0;
size_t trust_messages_max = 0;

char *trust_dots = NULL;
size_t trust_dot_count = 0;
size_t trust_max_dots = 0;

size_t trust_assert_count = 0;
size_t trust_failed_tests_count = 0;
size_t trust_failed_asserts_count = 0;
bool trust_first_failed_assert = true;

static inline void trust_check_test_count(void);

static inline void trust_register_test(char *name, trust_check_fn fn, bool setup) {
	trust_check_test_count();
	trust_tests[trust_tests_count++] = (trust_v) {.name = name, .setup = setup, .fn = fn};
}

static inline void trust_init_tests_array(void) {
	trust_tests = malloc(sizeof(trust_v) * TRUST_INITIAL_N_TESTS);
	if (!trust_tests) { exit(EXIT_FAILURE); }
	trust_tests_max = TRUST_INITIAL_N_TESTS;
}

static inline void trust_init_message_array(void) {
	trust_messages = malloc(sizeof(char*) * TRUST_INITIAL_N_MESSAGES);
	if (!trust_messages) { exit(EXIT_FAILURE); }
	trust_messages_max = TRUST_INITIAL_N_MESSAGES;
}

static inline void trust_init_dots_array(void) {
	trust_dots = malloc(sizeof(char) * TRUST_INITIAL_N_ASSERTS);
	if (!trust_dots) { exit(EXIT_FAILURE); }
	trust_max_dots = TRUST_INITIAL_N_ASSERTS;
}

static inline void trust_check_message_count(void) {
	if (trust_messages_count == trust_messages_max) {
		size_t new_max = trust_messages_max * 2;
		trust_messages = realloc(trust_messages, sizeof(char*) * new_max);
		if (!trust_messages) { exit(EXIT_FAILURE); }
		trust_messages_max = new_max;
	}
}

static inline void trust_check_test_count(void) {
	if (trust_tests_count == trust_tests_max) {
		size_t new_max = trust_tests_max * 2;
		trust_tests = realloc(trust_tests, sizeof(trust_v) * new_max);
		if (!trust_tests) { exit(EXIT_FAILURE); }
		trust_tests_max = new_max;
	}
}

static inline void trust_check_dot_count(void) {
	if (trust_dot_count == trust_max_dots) {
		size_t new_max = trust_max_dots * 2;
		trust_dots = realloc(trust_dots, sizeof(char) * new_max);
		if (!trust_dots) { exit(EXIT_FAILURE); }
		trust_max_dots = new_max;
	}
}

static inline void trust_store_message(const char *fmt, ...) {
	trust_check_message_count();

	va_list args, copy;
	va_start(args, fmt);
	va_copy(copy, args);

	size_t s = vsnprintf(NULL, 0, fmt, copy) + 1;
	va_end(copy);

	char *str = malloc(s);
	vsnprintf(str, s, fmt, args);
	va_end(args);

	trust_messages[trust_messages_count++] = str;
}

static inline void trust_store_dot(bool ok) {
	trust_check_dot_count();

	char dot = ok ? '.' : 'X';
	trust_dots[trust_dot_count++] = dot;
}

static inline void trust_print_dots(void) {
	for (size_t i = 0; i < trust_dot_count; i++) {
		if (i % 80 == 0) {
			printf("\n");
		}
		if (trust_dots[i] == '.') {
			printf(TRUST_OK_ "." TRUST_RESET_);
		} else {
			printf(TRUST_FAIL_ "X" TRUST_RESET_);
		}
	}
}

static inline void trust_print_failures(void) {
	if (trust_failed_tests_count == 0) {
		printf(TRUST_OK_ "TRUST WAS UPHELD\n" TRUST_RESET_);
		return;
	}

	printf(TRUST_FAIL_ "TRUST WAS BROKEN\n" TRUST_RESET_);

	for (size_t i = 0; i < trust_messages_count; i++) {
		printf("%s\n", trust_messages[i]);
	}
}

static inline void trust_print_summary(void) {
	if (trust_failed_tests_count == 0) {
		printf(TRUST_OK_);
	} else {
		printf(TRUST_FAIL_);
	}

	printf("%zu tests (%zu failed), %zu assertions (%zu failed)" TRUST_RESET_ "\n",
	       trust_tests_count, trust_failed_tests_count,
	       trust_assert_count, trust_failed_asserts_count);
}

static inline void trust_print_results(void) {
	trust_print_dots();
	printf("\n\n");
	trust_print_failures();
	printf("\n");
	trust_print_summary();
	printf("\n");
}

static inline void trust_run_tests(void) {
	if (trust_needs_setup && !trust_setup_fn_ptr && !trust_teardown_fn_ptr) {
		fprintf(stderr, "\n" TRUST_FAIL_ "ERROR: No setup or teardown functions provided!"
		        TRUST_RESET_ "\n\n");
		exit(EXIT_FAILURE);
	}

	if (trust_needs_setup && !trust_setup_fn_ptr) {
		fprintf(stderr, "\n" TRUST_FAIL_ "ERROR: No setup function provided!"
		        TRUST_RESET_ "\n\n");
		exit(EXIT_FAILURE);
	}

	if (trust_needs_setup && !trust_teardown_fn_ptr) {
		fprintf(stderr, "\n" TRUST_FAIL_ "ERROR: No teardown function provided!"
		        TRUST_RESET_ "\n\n");
		exit(EXIT_FAILURE);
	}

	bool setup;

	for (size_t i = 0; ; i++) {
		if (!trust_tests[i].fn) {
			break;
		}

		trust_first_failed_assert = true;

		setup = trust_tests[i].setup;
		if (setup) { trust_setup_fn_ptr(); }
		trust_tests[i].fn(trust_tests[i].name);
		if (setup) { trust_teardown_fn_ptr(); }
	}
}

static inline void trust_free_resources(void) {
	free(trust_tests);
	free(trust_messages);
	free(trust_dots);
}

__attribute__((constructor (102))) static void trust_init(void) {
	trust_init_tests_array();
	trust_init_message_array();
	trust_init_dots_array();
} int main(void) {
	printf(TRUST_OK_ "BEGIN TRUST VERIFICATION:" TRUST_RESET_ " " __FILE__ "\n");
	trust_run_tests();
	trust_print_results();
	trust_free_resources();
	return trust_failed_tests_count == 0 ? 0 : -1;
}

#endif // _TRUST_H_
