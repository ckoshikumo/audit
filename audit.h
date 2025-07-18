#ifndef INCLUDE_AUDIT_H
#define INCLUDE_AUDIT_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Customization
//

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
#ifndef AUDIT_INITIAL_N_AUDITS
#define AUDIT_INITIAL_N_AUDITS 50
#endif

#ifndef AUDIT_INITIAL_N_VERIFIES
#define AUDIT_INITIAL_N_VERIFIES 100
#endif

#ifndef AUDIT_INITIAL_N_MESSAGES
#define AUDIT_INITIAL_N_MESSAGES 100
#endif

//
// Interface
//
#define audit(...) adt_def(audit_, adt_narg(__VA_ARGS__))(__VA_ARGS__)

#define verify(assertion, desc, msg, ...)                                      \
	do {                                                                       \
		adt_verify_internal(assertion);                                        \
		adt_store_message(adt_pre_msg desc " [" msg "]", __FILE__, __LINE__,   \
		                  ##__VA_ARGS__);                                      \
	} while (0)

#define verify_eq(lhs, rhs, fmt, desc)                                         \
	adt_verify_builtin(==, lhs, rhs, fmt, desc, adt_eq_msg)

#define verify_neq(lhs, rhs, fmt, desc)                                        \
	adt_verify_builtin(!=, lhs, rhs, fmt, desc, adt_neq_msg)

#define verify_lt(lhs, rhs, fmt, desc)                                         \
	adt_verify_builtin(<, lhs, rhs, fmt, desc, adt_lt_msg)

#define verify_gt(lhs, rhs, fmt, desc)                                         \
	adt_verify_builtin(>, lhs, rhs, fmt, desc, adt_gt_msg)

#define verify_lteq(lhs, rhs, fmt, desc)                                       \
	adt_verify_builtin(<=, lhs, rhs, fmt, desc, adt_lteq_msg)

#define verify_gteq(lhs, rhs, fmt, desc)                                       \
	adt_verify_builtin(>=, lhs, rhs, fmt, desc, adt_gteq_msg)

//
// Internals
//
#define adt_verify_builtin(cmp, lhs, rhs, fmt, desc, msg)                      \
	do {                                                                       \
		adt_verify_internal(lhs cmp rhs);                                      \
		adt_store_message(adt_pre_msg desc " " msg(lhs, rhs, fmt));            \
	} while (0)

#define adt_verify_internal(assertion)                                         \
	audit_state.check_count++;                                                 \
	if (assertion) {                                                           \
		adt_store_result(true);                                                \
		break;                                                                 \
	}                                                                          \
	adt_store_result(false);                                                   \
	if (audit_state.first_failed_verify) {                                     \
		audit_state.first_failed_verify = false;                               \
		audit_state.failed_audits++;                                           \
		adt_store_message(AUDIT_PRINT_INFO("\n%i: %s"), this->n, this->name);  \
	}                                                                          \
	audit_state.failed_checks++

#define adt_audit_constructor(name, st, td, line)                              \
	void adt_concat(adt_audit_, line)(adtAudit * this);                        \
	__attribute__((constructor)) static void adt_concat(adt_init_,             \
	                                                    line)(void) {          \
		adt_register(name, adt_concat(adt_audit_, line), st, td);              \
	}                                                                          \
	void adt_concat(adt_audit_, line)(adtAudit * this)

/// Messages
#define AUDIT_PRINT_OK(_str) AUDIT_COLOR_OK _str AUDIT_COLOR_RESET
#define AUDIT_PRINT_FAIL(_str) AUDIT_COLOR_FAIL _str AUDIT_COLOR_RESET
#define AUDIT_PRINT_INFO(_str) AUDIT_COLOR_INFO _str AUDIT_COLOR_RESET

#define adt_pre_msg "\t%s:%i:\t"

#define adt_eq_msg(lhs, rhs, fmt)                                              \
	"[expected " fmt ", actual " fmt "]", __FILE__, __LINE__, rhs, lhs

#define adt_neq_msg(lhs, rhs, fmt)                                             \
	"[unexpected value: " fmt "]", __FILE__, __LINE__, rhs

#define adt_ineq_msg(_op, lhs, rhs, fmt)                                       \
	"[expected " fmt " " _op " " fmt "]", __FILE__, __LINE__, lhs, rhs

#define adt_lt_msg(lhs, rhs, fmt) adt_ineq_msg("<", lhs, rhs, fmt)

#define adt_gt_msg(lhs, rhs, fmt) adt_ineq_msg(">", lhs, rhs, fmt)

#define adt_lteq_msg(lhs, rhs, fmt) adt_ineq_msg("<=", lhs, rhs, fmt)

#define adt_gteq_msg(lhs, rhs, fmt) adt_ineq_msg(">=", lhs, rhs, fmt)

/// Helper macros
#define adt_concat_1(v1, v2) v1##v2
#define adt_concat(v1, v2) adt_concat_1(v1, v2)

/// Default arguments
#define adt_narg(...) adt_arg_1(__VA_ARGS__, adt_rseq_n())
#define adt_arg_1(...) adt_arg_n(__VA_ARGS__)
#define adt_arg_n(_1, _2, _3, N, ...) N
#define adt_rseq_n() 3, 2, 1, 0

#define adt_def_1(name, num) adt_##name##num
#define adt_def(name, num) adt_def_1(name, num)

#define adt_audit_1(name) adt_audit_constructor(name, NULL, NULL, __LINE__)
#define adt_audit_2(name, st_fn)                                               \
	adt_audit_constructor(name, st_fn, NULL, __LINE__)
#define adt_audit_3(name, st_fn, td_fn)                                        \
	adt_audit_constructor(name, st_fn, td_fn, __LINE__)

/// Types
typedef struct adtAudit adtAudit;

typedef void (*adtAuditFn)(adtAudit *this);
typedef void (*adtSetupFn)(void);
typedef adtSetupFn adtTeardownFn;

struct adtAudit {
	char *name;
	size_t n;
	adtAuditFn fn;
	adtSetupFn setup;
	adtTeardownFn teardown;
};

struct adtState {
	size_t check_count;
	size_t failed_audits;
	size_t failed_checks;
	bool first_failed_verify;
};

extern struct adtState audit_state;

/// Forward declarations
void adt_register(char *name, adtAuditFn fn, adtSetupFn st, adtTeardownFn td);
void adt_store_message(const char *fmt, ...);
void adt_store_result(bool res);

#endif // INCLUDE_AUDIT_H

#ifdef AUDIT_IMPLEMENTATION

struct adtState audit_state = {.first_failed_verify = true};

struct adtAuditRegistry {
	size_t count, max;
	adtAudit *data;
} adt_audits = {.max = AUDIT_INITIAL_N_AUDITS};

struct adtselectedRegistry {
	size_t count, max;
	size_t *data;
} adt_selected = {.max = AUDIT_INITIAL_N_AUDITS};

struct adtResultRegistry {
	size_t count, max;
	size_t *data;
} adt_results = {.max = AUDIT_INITIAL_N_VERIFIES};

struct adtMessageRegistry {
	size_t count, max;
	char **data;
} adt_messages = {.max = AUDIT_INITIAL_N_MESSAGES};

#define adt_ensure_capacity(name)                                              \
	do {                                                                       \
		if (name.count == name.max) {                                          \
			name.max *= 2;                                                     \
			name.data = realloc(name.data, sizeof(*name.data) * name.max);     \
			if (!name.data) exit(EXIT_FAILURE);                                \
		}                                                                      \
	} while (0)

void adt_register(char *name, adtAuditFn fn, adtSetupFn st, adtSetupFn td) {
	adt_ensure_capacity(adt_audits);
	size_t audit_idx = adt_audits.count++;
	adt_audits.data[audit_idx] = (adtAudit) {
	    .name = name, .n = audit_idx, .fn = fn, .setup = st, .teardown = td};
}

void adt_store_message(const char *fmt, ...) {
	va_list args, args_copy;
	va_start(args, fmt);

	va_copy(args_copy, args);
	size_t s = (size_t) vsnprintf(NULL, 0, fmt, args_copy) + 1;
	va_end(args_copy);

	char *str = malloc(s);
	if (!str) exit(EXIT_FAILURE);

	vsnprintf(str, s, fmt, args);
	va_end(args);

	adt_ensure_capacity(adt_messages);
	adt_messages.data[adt_messages.count++] = str;
}

void adt_store_result(bool res) {
	adt_ensure_capacity(adt_results);
	adt_results.data[adt_results.count++] = res;
}

static void adt_print_dots(void) {
	for (size_t i = 0; i < adt_results.count; i++) {
		if (i % 80 == 0) printf("\n");
		adt_results.data[i] ? printf(AUDIT_PRINT_OK(AUDIT_PASS_CHECK_STR))
		                    : printf(AUDIT_PRINT_FAIL(AUDIT_FAIL_CHECK_STR));
	}
	printf("\n\n");
}

static void adt_print_failures(void) {
	if (audit_state.failed_checks == 0) {
		printf(AUDIT_PRINT_OK("AUDIT OK\n"));
		return;
	}

	printf(AUDIT_PRINT_FAIL("AUDIT FAILED\n"));

	for (size_t i = 0; i < adt_messages.count; i++)
		printf("%s\n", adt_messages.data[i]);

	printf("\n");
}

static void adt_print_summary(void) {
	audit_state.failed_checks == 0 ? printf(AUDIT_COLOR_OK)
	                               : printf(AUDIT_COLOR_FAIL);

	printf("%zu audits (%zu failed), %zu verifies (%zu "
	       "failed)\n\n" AUDIT_COLOR_RESET,
	       adt_audits.count, audit_state.failed_audits, audit_state.check_count,
	       audit_state.failed_checks);
}

static void adt_print_results(void) {
	adt_print_dots();
	adt_print_failures();
	adt_print_summary();
}

static void adt_run(size_t audit_idx) {
	audit_state.first_failed_verify = true;

	adtAudit *audit = &adt_audits.data[audit_idx];
	adtSetupFn setup = audit->setup;
	adtTeardownFn teardown = audit->teardown;

	if (setup) setup();
	audit->fn(audit);
	if (teardown) teardown();
}

static void adt_run_selected(void) {
	printf("Running selected audits:\n\n");
	for (size_t i = 0; i < adt_selected.count; i++) {
		size_t audit_idx = adt_selected.data[i];
		printf(AUDIT_PRINT_INFO("%lu: %s\n"), i,
		       adt_audits.data[audit_idx].name);
	}

	for (size_t i = 0; i < adt_selected.count; i++)
		adt_run(adt_selected.data[i]);
}

static void adt_run_all(void) {
	printf("Running all audits.\n");
	for (size_t i = 0; i < adt_audits.count; i++) adt_run(i);
}

static void adt_print_available(void) {
	for (size_t i = 0; i < adt_audits.count; i++)
		printf(AUDIT_PRINT_INFO("%lu: %s\n"), i, adt_audits.data[i].name);
}

static void adt_select(char *input) {
	char *end = NULL;
	size_t audit_idx = (size_t) strtol(input, &end, 10);

	if (*end) {
		printf(
		    AUDIT_PRINT_FAIL("ERROR: ") "Couldn't load argument number: %s\n\n",
		    input);
		return;
	}

	if (audit_idx >= adt_audits.count) {
		printf(AUDIT_PRINT_FAIL("ERROR: ") "Audit %lu doesn't exist.\n\n",
		       audit_idx);
		return;
	}

	adt_ensure_capacity(adt_selected);
	adt_selected.data[adt_selected.count++] = audit_idx;
}

static void adt_free(void) {
	free(adt_audits.data);
	free(adt_selected.data);
	free(adt_results.data);
	for (size_t i = 0; i < adt_messages.count; i++) free(adt_messages.data[i]);
	free(adt_messages.data);
}

__attribute__((constructor)) static void audit_init(void) {
	adt_audits.data = malloc(sizeof(*adt_audits.data) * adt_audits.max);
	adt_selected.data = malloc(sizeof(*adt_selected.data) * adt_selected.max);
	adt_messages.data = malloc(sizeof(*adt_messages.data) * adt_messages.max);
	adt_results.data = malloc(sizeof(*adt_results.data) * adt_results.max);

	if (!adt_audits.data || !adt_selected.data || !adt_messages.data ||
	    !adt_results.data)
		exit(EXIT_FAILURE);
}
int main(int argc, char **argv) {
	atexit(adt_free);
	bool tried_to_select = false;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--list") == 0) {
			adt_print_available();
			return 0;
		}

		tried_to_select = true;
		adt_select(argv[i]);
	}

	if (tried_to_select && adt_selected.count == 0) {
		printf(AUDIT_PRINT_FAIL("Couldn't run any audits.\n"));
		return -1;
	}

	printf(AUDIT_PRINT_OK("AUDIT START\n\n"));
	adt_selected.count > 0 ? adt_run_selected() : adt_run_all();
	adt_print_results();

	return audit_state.failed_checks == 0 ? 0 : -1;
}

#endif // AUDIT_IMPLEMENTATION
