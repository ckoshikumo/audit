#ifndef _AUDIT_H_
#define _AUDIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define AUDIT_MAX_MESSAGES_ 50
#define AUDIT_MAX_MESSAGE_LENGTH_ 256

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

int audit_total_tests_ = 0;
int audit_total_asserts_ = 0;
int audit_local_failed_asserts_ = 0;
int audit_failed_asserts_ = 0;
int audit_failed_tests_ = 0;
int audit_next_message_ = 0;
int audit_current_dot_ = 0;

typedef char audit_message_[AUDIT_MAX_MESSAGE_LENGTH_];
audit_message_ audit_messages_[AUDIT_MAX_MESSAGES_];

#define AUDIT(assert_, msg_, ...) do {\
		audit_total_asserts_++;\
		audit_current_dot_++;\
		if (audit_current_dot_ == 80) {\
			printf("\n");\
			audit_current_dot_ = 0;\
		}\
		if (assert_) {\
			printf(GREEN_ "." RESET_);\
			break;\
		}\
		printf(RED_ "F" RESET_);\
		audit_failed_asserts_++;\
		if (audit_local_failed_asserts_ == 0) {\
			audit_failed_tests_++;\
			audit_check_message_count_();\
			snprintf(audit_messages_[audit_next_message_], AUDIT_MAX_MESSAGE_LENGTH_,\
			         YELLOW_ "%s" RESET_, __func__);\
			audit_next_message_++;\
		}\
		audit_check_message_count_();\
		snprintf(audit_messages_[audit_next_message_], AUDIT_MAX_MESSAGE_LENGTH_,\
		         "\t%i: " msg_, __LINE__, ##__VA_ARGS__);\
		audit_next_message_++;\
	} while (0)

static inline void audit_run(void (*testFn)(void))
{
	audit_total_tests_++;
	audit_local_failed_asserts_ = 0;
	testFn();
}

static inline void audit_print_failures_(void)
{
	printf("\n\n");
	if (audit_next_message_ == 0 ) return;
	for (int i = 0; i < audit_next_message_; ++i) {
		printf("%s\n", audit_messages_[i]);
	}
	printf("\n");
}

static inline void audit_check_message_count_(void)
{
	if (audit_next_message_ < AUDIT_MAX_MESSAGES_) return;

	audit_print_failures_();
	printf(RED_ "TOO MANY ERRORS! Aborting." RESET_ "\n");
	exit(EXIT_FAILURE);
}

static inline void audit_summary_(void)
{
	printf("\n\n");

	if (audit_failed_tests_ == 0) {
		printf(GREEN_ "ALL TESTS PASSED" RESET_);
	} else {
		printf(RED_ "TEST SUITE FAILED" RESET_);
	}

	audit_print_failures_();

	printf("%i tests (%i failed), %i assertions (%i failed)\n",
	       audit_total_tests_, audit_failed_tests_,
	       audit_total_asserts_, audit_failed_asserts_);
}

#define CHECK(name_) void name_(void)

#define AUDIT_RUN(test_suite_)\
	int main(void) {\
		printf(GREEN_ "BEGIN AUDITING" RESET_ "\n");\
		printf("Test suite: %s\n\n", __FILE__);\
		test_suite_();\
		audit_summary_();\
		return audit_failed_tests_ == 0 ? 0 : -1;\
	}

#endif // _AUDIT_H_
