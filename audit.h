#ifndef _AUDIT_H_
#define _AUDIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../types/types.h"

#define AUDIT_MAX_MESSAGES 50
#define AUDIT_MAX_MESSAGE_LENGTH 256

#ifdef AUDIT_NO_COLORS
#define RED_
#define GREEN_
#define YELLOW_
#define RESET_
#else
#define RED_     "\x1b[31m"
#define GREEN_   "\x1b[32m"
#define YELLOW_  "\x1b[33m"
#define RESET_   "\x1b[0m"
#endif

// Typedefs and forward declares:

typedef void (*audit_check_fn)(void);

typedef struct audit_s {
	char* name;
	audit_check_fn fn;
} audit_s;

typedef char audit_message[AUDIT_MAX_MESSAGE_LENGTH];
audit_message audit_messages[AUDIT_MAX_MESSAGES];

void audit_setup(void);
void audit_teardown(void);

audit_s audit_bare_tests[];
audit_s audit_tests[];

// User API:
#define AUDIT_SETUP void audit_setup(void)
#define AUDIT_TEARDOWN void audit_teardown(void)

#define AUDIT(name_) void name_(void)
#define AUDIT_BARE(name_) void name_(void)

#define CHECK(assert_, msg_, ...) do {\
		audit_total_asserts++;\
		if (audit_current_dot == 80) {\
			printf("\n");\
			audit_current_dot = 0;\
		}\
		audit_current_dot++;\
		if (assert_) {\
			printf(GREEN_ "." RESET_);\
			break;\
		}\
		printf(RED_ "F" RESET_);\
		audit_failed_asserts++;\
		if (audit_local_failed_asserts == 0) {\
			audit_local_failed_asserts++;\
			audit_failed_tests++;\
			_audit_check_message_count();\
			snprintf(audit_messages[audit_next_message], AUDIT_MAX_MESSAGE_LENGTH,\
			         YELLOW_ "\n%s" RESET_, current_name);\
			audit_next_message++;\
		}\
		_audit_check_message_count();\
		snprintf(audit_messages[audit_next_message], AUDIT_MAX_MESSAGE_LENGTH,\
		         "\t[%i] " msg_, __LINE__, ##__VA_ARGS__);\
		audit_next_message++;\
	} while (0)

#define AUDIT_RUN(_)\
	int main(void) {\
		printf(GREEN_ "START AUDITING:" RESET_ " " __FILE__ "\n");\
		_audit_run_tests();\
		_audit_print_summary();\
		return audit_failed_tests == 0 ? 0 : -1;\
	}

#define AUDIT_REGISTER(...) audit_s audit_tests[] = { __VA_ARGS__, { 0 }}
#define AUDIT_REGISTER_BARE(...) audit_s audit_bare_tests[] = { __VA_ARGS__, { 0 }}

// Internals:
u32 audit_total_tests = 0;
u32 audit_total_asserts = 0;
u32 audit_local_failed_asserts = 0;
u32 audit_failed_asserts = 0;
u32 audit_failed_tests = 0;
u32 audit_next_message = 0;
u32 audit_current_dot = 0;

static inline void _audit_print_failures(void) {
	if (audit_next_message == 0 ) {
		printf("\n");
		return;
	}

	for (int i = 0; i < audit_next_message; ++i) {
		printf("%s\n", audit_messages[i]);
	}
	printf("\n");
}

static inline void _audit_check_message_count(void) {
	if (audit_next_message < AUDIT_MAX_MESSAGES) {
		return;
	}

	_audit_print_failures();

	printf(RED_ "TOO MANY ERRORS! Aborting..." RESET_ "\n");
	exit(EXIT_FAILURE);
}

static inline void _audit_print_summary(void) {
	printf("\n\n");

	if (audit_failed_tests == 0) {
		printf(GREEN_ "AUDITING OK\n" RESET_);
	} else {
		printf(RED_ "AUDITING FAILED\n" RESET_);
	}

	_audit_print_failures();

	printf("%i tests (%i failed), %i assertions (%i failed)\n",
	       audit_total_tests, audit_failed_tests,
	       audit_total_asserts, audit_failed_asserts);
}

char *current_name;

static inline void _audit_run_tests(void) {
	for (u32 i = 0; ; i++) {
		if (!audit_bare_tests[i].fn) {
			break;
		}

		audit_total_tests++;
		audit_local_failed_asserts = 0;

		current_name = audit_bare_tests[i].name;
		audit_bare_tests[i].fn();
	}

	for (u32 i = 0; ; i++) {
		if (!audit_tests[i].fn) {
			break;
		}

		audit_total_tests++;
		audit_local_failed_asserts = 0;

		current_name = audit_tests[i].name;
		audit_setup();
		audit_tests[i].fn();
		audit_teardown();
	}
}

#endif // _AUDIT_H_
