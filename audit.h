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

int totalTests_ = 0;
int totalAsserts_ = 0;
int localFailedAsserts_ = 0;
int totalFailedAsserts_ = 0;
int failedTests_ = 0;
int nextMessage_ = 0;
int currentDot_ = 0;

typedef char message_[MAX_MESSAGE_LENGTH_];
message_ messages_[MAX_MESSAGES_];

#define auditAssert(assert_, msg_, ...) do {\
		totalAsserts_++;\
		currentDot_++;\
		if (currentDot_ == 80) {\
			printf("\n");\
			currentDot_ = 0;\
		}\
		if (assert_) {\
			printf(GREEN_ "." RESET_);\
			break;\
		}\
		printf(RED_ "F" RESET_);\
		totalFailedAsserts_++;\
		if (localFailedAsserts_ == 0) {\
			failedTests_++;\
			checkMessageCount_();\
			snprintf(messages_[nextMessage_], MAX_MESSAGE_LENGTH_,\
			         "" YELLOW_ "%s" RESET_ "", __func__);\
			nextMessage_++;\
		}\
		checkMessageCount_();\
		snprintf(messages_[nextMessage_], MAX_MESSAGE_LENGTH_,\
		         "\t%i: " msg_,\
		         __LINE__, ##__VA_ARGS__);\
		nextMessage_++;\
	} while (0)

static inline void auditRun(void (*testFn)(void))
{
	totalTests_++;
	localFailedAsserts_ = 0;
	testFn();
}

static inline void auditPrintFailures_(void)
{
	printf("\n\n");
	if (nextMessage_ == 0 ) return;
	for (int i = 0; i < nextMessage_; ++i) {
		printf("%s\n", messages_[i]);
	}
	printf("\n");
}

static inline void checkMessageCount_(void)
{
	if (nextMessage_ < MAX_MESSAGES_) return;

	auditPrintFailures_();
	printf(RED_ "TOO MANY ERRORS! Aborting." RESET_ "\n");
	exit(EXIT_FAILURE);
}

static inline int postface_(void)
{
	printf("\n\n");

	if (failedTests_ == 0) {
		printf(GREEN_ "ALL TESTS PASSED" RESET_);
	} else {
		printf(RED_ "TEST SUITE FAILED" RESET_);
	}

	auditPrintFailures_();

	printf("%i tests (%i failed), %i assertions (%i failed)\n",
	       totalTests_, failedTests_, totalAsserts_, totalFailedAsserts_);

	return failedTests_ == 0 ? 0 : -1;
}

#define AUDIT(testSuite_)\
	int main(void) {\
		printf(GREEN_ "BEGIN AUDITING" RESET_ "\n");\
		printf("Test suite: %s\n\n", __FILE__);\
		testSuite_();\
		return postface_();\
	}

#endif // _AUDIT_H_
