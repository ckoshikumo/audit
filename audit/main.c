/*
MIT License

Copyright (c) 2023 Cassio Koshikumo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include "headers/dependencies.h"

#ifndef INCLUDE_AUDIT_H
#define INCLUDE_AUDIT_H

#include "headers/audit_flags.h"
#include "headers/interface.h"
#include "headers/internals.h"
#endif //INCLUDE_AUDIT_H


#ifdef AUDIT_IMPLEMENTATION
#include "structs.c"
#include "functions.c"


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

#endif //AUDIT_IMPLEMENTATION