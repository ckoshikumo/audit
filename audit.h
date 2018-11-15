#ifndef _AUDIT_H_
#define _AUDIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_MESSAGES_ 50
#define MAX_MESSAGE_LENGTH_ 256

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

int auditTotalTests_ = 0;
int auditTotalAsserts_ = 0;
int auditLocalFailedAsserts_ = 0;
int auditFailedAsserts_ = 0;
int auditFailedTests_ = 0;
int auditNextMessage_ = 0;
int auditCurrentDot_ = 0;

typedef char auditMessage_[MAX_MESSAGE_LENGTH_];
auditMessage_ auditMessages_[MAX_MESSAGES_];

#define audit(assert_, msg_, ...) do {\
		auditTotalAsserts_++;\
		auditCurrentDot_++;\
		if (auditCurrentDot_ == 80) {\
			printf("\n");\
			auditCurrentDot_ = 0;\
		}\
		if (assert_) {\
			printf(GREEN_ "." RESET_);\
			break;\
		}\
		printf(RED_ "F" RESET_);\
		auditFailedAsserts_++;\
		if (auditLocalFailedAsserts_ == 0) {\
			auditFailedTests_++;\
			auditCheckMessageCount_();\
			snprintf(auditMessages_[auditNextMessage_], MAX_MESSAGE_LENGTH_,\
			         YELLOW_ "%s" RESET_, __func__);\
			auditNextMessage_++;\
		}\
		auditCheckMessageCount_();\
		snprintf(auditMessages_[auditNextMessage_], MAX_MESSAGE_LENGTH_,\
		         "\t%i: " msg_, __LINE__, ##__VA_ARGS__);\
		auditNextMessage_++;\
	} while (0)

static inline void auditRun(void (*testFn)(void))
{
	auditTotalTests_++;
	auditLocalFailedAsserts_ = 0;
	testFn();
}

static inline void auditPrintFailures_(void)
{
	printf("\n\n");
	if (auditNextMessage_ == 0 ) return;
	for (int i = 0; i < auditNextMessage_; ++i) {
		printf("%s\n", auditMessages_[i]);
	}
	printf("\n");
}

static inline void auditCheckMessageCount_(void)
{
	if (auditNextMessage_ < MAX_MESSAGES_) return;

	auditPrintFailures_();
	printf(RED_ "TOO MANY ERRORS! Aborting." RESET_ "\n");
	exit(EXIT_FAILURE);
}

static inline void auditPostface_(void)
{
	printf("\n\n");

	if (auditFailedTests_ == 0) {
		printf(GREEN_ "ALL TESTS PASSED" RESET_);
	} else {
		printf(RED_ "TEST SUITE FAILED" RESET_);
	}

	auditPrintFailures_();

	printf("%i tests (%i failed), %i assertions (%i failed)\n",
	       auditTotalTests_, auditFailedTests_,
	       auditTotalAsserts_, auditFailedAsserts_);
}

#define AUDIT(testSuite_)\
	int main(void) {\
		printf(GREEN_ "BEGIN AUDITING" RESET_ "\n");\
		printf("Test suite: %s\n\n", __FILE__);\
		testSuite_();\
		auditPostface_();\
		return auditFailedTests_ == 0 ? 0 : -1;\
	}

#endif // _AUDIT_H_
