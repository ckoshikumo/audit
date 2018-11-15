#ifndef _AUDIT_H_
#define _AUDIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

typedef char audit_message[AUDIT_MAX_MESSAGE_LENGTH];
audit_message audit_messages[AUDIT_MAX_MESSAGES];

typedef void (*audit_check_fn)(void);

int audit_total_tests = 0;
int audit_total_asserts = 0;
int audit_local_failed_asserts = 0;
int audit_failed_asserts = 0;
int audit_failed_tests = 0;
int audit_next_message = 0;
int audit_current_dot = 0;

static inline void _audit_print_failures(void)
{
	printf("\n\n");
	if (audit_next_message == 0 ) return;
	for (int i = 0; i < audit_next_message; ++i) {
		printf("%s\n", audit_messages[i]);
	}
	printf("\n");
}

static inline void _audit_check_message_count(void)
{
	if (audit_next_message < AUDIT_MAX_MESSAGES) return;

	_audit_print_failures();
	printf(RED_ "TOO MANY ERRORS! Aborting." RESET_ "\n");
	exit(EXIT_FAILURE);
}

static inline void _audit_print_summary(void)
{
	printf("\n\n");

	if (audit_failed_tests == 0) {
		printf(GREEN_ "AUDITING OK" RESET_);
	} else {
		printf(RED_ "AUDITING FAILED" RESET_);
	}

	_audit_print_failures();

	printf("%i tests (%i failed), %i assertions (%i failed)\n",
	       audit_total_tests, audit_failed_tests,
	       audit_total_asserts, audit_failed_asserts);
}

#define CHECK(name_) void name_(void)

static inline void audit_register(audit_check_fn fn)
{
	audit_total_tests++;
	audit_local_failed_asserts = 0;
	fn();
}

#define AUDIT(assert_, msg_, ...) do {\
		audit_total_asserts++;\
		audit_current_dot++;\
		if (audit_current_dot == 80) {\
			printf("\n");\
			audit_current_dot = 0;\
		}\
		if (assert_) {\
			printf(GREEN_ "." RESET_);\
			break;\
		}\
		printf(RED_ "F" RESET_);\
		audit_failed_asserts++;\
		if (audit_local_failed_asserts == 0) {\
			audit_failed_tests++;\
			_audit_check_message_count();\
			snprintf(audit_messages[audit_next_message], AUDIT_MAX_MESSAGE_LENGTH,\
			         YELLOW_ "%s" RESET_, __func__);\
			audit_next_message++;\
		}\
		_audit_check_message_count();\
		snprintf(audit_messages[audit_next_message], AUDIT_MAX_MESSAGE_LENGTH,\
		         "\t[%i] " msg_, __LINE__, ##__VA_ARGS__);\
		audit_next_message++;\
	} while (0)

#define AUDIT_RUN(test_suite_)\
	int main(void) {\
		printf(GREEN_ "START AUDITING:" RESET_ " " __FILE__ "\n");\
		test_suite_();\
		_audit_print_summary();\
		return audit_failed_tests == 0 ? 0 : -1;\
	}

#endif // _AUDIT_H_
