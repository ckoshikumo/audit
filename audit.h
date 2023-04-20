#ifndef INCLUDE_AUDIT_H
#define INCLUDE_AUDIT_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// INTERFACE:
#define audit(...) _audit_def(audit_, _audit_narg(__VA_ARGS__))(__VA_ARGS__)

#define check(_assert, _desc, _msg, ...)                                                           \
	do {                                                                                       \
		_check_internals(_assert);                                                         \
		_audit_store_message(_audit_pre_msg _desc " [" _msg "]", __FILE__, __LINE__,       \
				     ##__VA_ARGS__);                                               \
	} while (0)

#define check_eq(_lhs, _rhs, _fmt, _desc) _check(==, _lhs, _rhs, _fmt, _desc, _eq_msg)
#define check_neq(_lhs, _rhs, _fmt, _desc) _check(!=, _lhs, _rhs, _fmt, _desc, _audit_neq_msg)
#define check_lt(_lhs, _rhs, _fmt, _desc) _check(<, _lhs, _rhs, _fmt, _desc, _audit_lt_msg)
#define check_gt(_lhs, _rhs, _fmt, _desc) _check(>, _lhs, _rhs, _fmt, _desc, _audit_gt_msg)
#define check_lteq(_lhs, _rhs, _fmt, _desc) _check(<=, _lhs, _rhs, _fmt, _desc, _audit_lteq_msg)
#define check_gteq(_lhs, _rhs, _fmt, _desc) _check(>=, _lhs, _rhs, _fmt, _desc, _audit_gteq_msg)

// INTERNALS:
#define _check(_cmp, _lhs, _rhs, _fmt, _desc, _msg)                                                \
	do {                                                                                       \
		_check_internals(_lhs _cmp _rhs);                                                  \
		_audit_store_message(_audit_pre_msg _desc " " _msg(_lhs, _rhs, _fmt));             \
	} while (0)

#define _check_internals(_assert)                                                                  \
	audit_state.check_count++;                                                                 \
	if (_assert) {                                                                             \
		_audit_store_result(true);                                                         \
		break;                                                                             \
	}                                                                                          \
	_audit_store_result(false);                                                                \
	if (audit_state.first_failed_check) {                                                      \
		audit_state.first_failed_check = false;                                            \
		audit_state.failed_tests++;                                                        \
		_audit_store_message(AUDIT_PRINT_INFO("\n%i: %s"), this->n, this->name);           \
	}                                                                                          \
	audit_state.failed_checks++;

#define _audit_internal(_name, _setup, _teardown, _line)                                           \
	void _audit_concat(_audit_test, _line)(audit_test_s * this);                               \
	__attribute__((constructor)) static void _audit_concat(audit_init_, _line)(void)           \
	{                                                                                          \
		_audit_register(_name, _audit_concat(_audit_test, _line), _setup, _teardown);      \
	}                                                                                          \
	void _audit_concat(_audit_test, _line)(audit_test_s * this)

#define _audit_pre_msg "\t%s:%i:\t"
#define _eq_msg(_lhs, _rhs, _fmt)                                                                  \
	"[expected " _fmt ", actual " _fmt "]", __FILE__, __LINE__, _rhs, _lhs
#define _audit_neq_msg(_lhs, _rhs, _fmt) "[unexpected value: " _fmt "]", __FILE__, __LINE__, _rhs
#define _audit_ineq_msg(_op, _lhs, _rhs, _fmt)                                                     \
	"[expected " _fmt " " _op " " _fmt "]", __FILE__, __LINE__, _lhs, _rhs
#define _audit_lt_msg(_lhs, _rhs, _fmt) _audit_ineq_msg("<", _lhs, _rhs, _fmt)
#define _audit_gt_msg(_lhs, _rhs, _fmt) _audit_ineq_msg(">", _lhs, _rhs, _fmt)
#define _audit_lteq_msg(_lhs, _rhs, _fmt) _audit_ineq_msg("<=", _lhs, _rhs, _fmt)
#define _audit_gteq_msg(_lhs, _rhs, _fmt) _audit_ineq_msg(">=", _lhs, _rhs, _fmt)

// Maybe I could consolidate these with _audit_def, but I don't want to think about it.
#define _audit_concat_1(v1, v2) v1##v2
#define _audit_concat(v1, v2) _audit_concat_1(v1, v2)

// Macro trickery to make default arguments work:
#define _audit_narg(...) _audit_arg_1(__VA_ARGS__, _audit_rseq_n())
#define _audit_arg_1(...) _audit_arg_n(__VA_ARGS__)
#define _audit_arg_n(_1, _2, _3, N, ...) N
#define _audit_rseq_n() 3, 2, 1, 0

#define _audit_def_1(_name, _n) _##_name##_n
#define _audit_def(_name, _n) _audit_def_1(_name, _n)

#define _audit_1(_name) _audit_internal(_name, NULL, NULL, __LINE__)
#define _audit_2(_name, _setup) _audit_internal(_name, _setup, NULL, __LINE__)
#define _audit_3(_name, _setup, _teardown) _audit_internal(_name, _setup, _teardown, __LINE__)

#define AUDIT_PRINT_OK(_str) AUDIT_COLOR_OK _str AUDIT_COLOR_RESET
#define AUDIT_PRINT_FAIL(_str) AUDIT_COLOR_FAIL _str AUDIT_COLOR_RESET
#define AUDIT_PRINT_INFO(_str) AUDIT_COLOR_INFO _str AUDIT_COLOR_RESET

typedef struct audit_test_s audit_test_s;

typedef void (*audit_test_fn)(audit_test_s *this);
typedef void (*audit_setup_fn)(void);
typedef audit_setup_fn audit_teardown_fn;

struct audit_test_s {
	char *name;
	size_t n;
	audit_test_fn fn;
	audit_setup_fn setup;
	audit_teardown_fn teardown;
};

struct audit_state_s {
	size_t check_count;
	size_t failed_tests;
	size_t failed_checks;
	bool first_failed_check;
};

extern struct audit_state_s audit_state;

void _audit_register(char *name, audit_test_fn fn, audit_setup_fn st, audit_teardown_fn td);
void _audit_store_message(const char *fmt, ...);
void _audit_store_result(bool res);

#endif // INCLUDE_AUDIT_H

#ifdef AUDIT_IMPLEMENTATION

struct audit_state_s audit_state = {.first_failed_check = true};

struct audit_tests_s {
	size_t count, max;
	audit_test_s *data;
} audit_tests = {.max = AUDIT_INITIAL_N_TESTS};

struct audit_selected_s {
	size_t count, max;
	size_t *data;
} audit_selected = {.max = AUDIT_INITIAL_N_TESTS};

struct audit_results_s {
	size_t count, max;
	size_t *data;
} audit_results = {.max = AUDIT_INITIAL_N_CHECKS};

struct audit_messages_s {
	size_t count, max;
	char **data;
} audit_messages = {.max = AUDIT_INITIAL_N_MESSAGES};

#define audit_ensure_capacity(_name)                                                               \
	do {                                                                                       \
		if (_name.count == _name.max) {                                                    \
			_name.max *= 2;                                                            \
			_name.data = realloc(_name.data, sizeof(*_name.data) * _name.max);         \
			if (!_name.data) {                                                         \
				exit(EXIT_FAILURE);                                                \
			}                                                                          \
		}                                                                                  \
	} while (0)

void _audit_register(char *name, audit_test_fn fn, audit_setup_fn st, audit_setup_fn td)
{
	audit_ensure_capacity(audit_tests);
	size_t test_n = audit_tests.count++;
	audit_tests.data[test_n] =
	    (audit_test_s){.name = name, .n = test_n, .fn = fn, .setup = st, .teardown = td};
}

void _audit_store_message(const char *fmt, ...)
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
	audit_messages.data[audit_messages.count++] = str;
}

void _audit_store_result(bool res)
{
	audit_ensure_capacity(audit_results);
	audit_results.data[audit_results.count++] = res;
}

void audit_print_dots(void)
{
	for (size_t i = 0; i < audit_results.count; i++) {
		if (i % 80 == 0) {
			printf("\n");
		}

		audit_results.data[i] ? printf(AUDIT_PRINT_OK(AUDIT_PASS_CHECK_STR))
				      : printf(AUDIT_PRINT_FAIL(AUDIT_FAIL_CHECK_STR));
	}
	printf("\n\n");
}

void audit_print_failures(void)
{
	if (audit_state.failed_checks == 0) {
		printf(AUDIT_PRINT_OK("AUDIT OK\n"));
		return;
	}

	printf(AUDIT_PRINT_FAIL("AUDIT FAILED\n"));

	for (size_t i = 0; i < audit_messages.count; i++) {
		printf("%s\n", audit_messages.data[i]);
	}
	printf("\n");
}

void audit_print_summary(void)
{
	audit_state.failed_checks == 0 ? printf(AUDIT_COLOR_OK) : printf(AUDIT_COLOR_FAIL);

	printf("%zu tests (%zu failed), %zu checks (%zu failed)\n\n" AUDIT_COLOR_RESET,
	       audit_tests.count, audit_state.failed_tests, audit_state.check_count,
	       audit_state.failed_checks);
}

void audit_print_results(void)
{
	audit_print_dots();
	audit_print_failures();
	audit_print_summary();
}

void audit_run(size_t test_n)
{
	audit_state.first_failed_check = true;

	audit_setup_fn setup = audit_tests.data[test_n].setup;
	audit_setup_fn teardown = audit_tests.data[test_n].teardown;

	if (setup) {
		setup();
	}

	audit_tests.data[test_n].fn(&audit_tests.data[test_n]);

	if (teardown) {
		teardown();
	}
}

void audit_run_selected(void)
{
	printf("Running selected tests:\n\n");
	for (size_t i = 0; i < audit_selected.count; i++) {
		size_t test_n = audit_selected.data[i];
		printf(AUDIT_PRINT_INFO("%lu: %s\n"), i, audit_tests.data[test_n].name);
	}

	for (size_t i = 0; i < audit_selected.count; i++) {
		audit_run(audit_selected.data[i]);
	}
}

void audit_run_all(void)
{
	printf("Running all tests.\n");
	for (size_t i = 0; i < audit_tests.count; i++) {
		audit_run(i);
	}
}

void audit_print_available(void)
{
	for (size_t i = 0; i < audit_tests.count; i++) {
		printf(AUDIT_PRINT_INFO("%lu: %s\n"), i, audit_tests.data[i].name);
	}
}

void audit_select(char *input)
{
	char *end = NULL;
	size_t test_n = (size_t)strtol(input, &end, 10);

	if (*end) {
		printf(AUDIT_PRINT_FAIL("ERROR: ") "Couldn't load argument number: %s\n\n", input);
		return;
	}

	if (test_n >= audit_tests.count) {
		printf(AUDIT_PRINT_FAIL("ERROR: ") "Test %lu doesn't exist.\n\n", test_n);
		return;
	}

	audit_ensure_capacity(audit_selected);
	audit_selected.data[audit_selected.count++] = test_n;
}

void audit_free_resources(void)
{
	free(audit_tests.data);
	free(audit_selected.data);
	free(audit_results.data);
	for (size_t i = 0; i < audit_messages.count; i++) {
		free(audit_messages.data[i]);
	}
	free(audit_messages.data);
}

__attribute__((constructor)) static void audit_init(void)
{
	audit_tests.data = malloc(sizeof(*audit_tests.data) * audit_tests.max);
	audit_selected.data = malloc(sizeof(*audit_selected.data) * audit_selected.max);
	audit_messages.data = malloc(sizeof(*audit_messages.data) * audit_messages.max);
	audit_results.data = malloc(sizeof(*audit_results.data) * audit_results.max);

	if (!audit_tests.data || !audit_selected.data || !audit_messages.data ||
	    !audit_results.data) {
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
		audit_select(argv[i]);
	}

	if (tried_to_select && audit_selected.count == 0) {
		printf(AUDIT_PRINT_FAIL("Couldn't run any tests.\n"));
		return -1;
	}

	printf(AUDIT_PRINT_OK("AUDIT START\n\n"));
	audit_selected.count > 0 ? audit_run_selected() : audit_run_all();
	audit_print_results();

	return audit_state.failed_checks == 0 ? 0 : -1;
}

#endif // AUDIT_IMPLEMENTATION
