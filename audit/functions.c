
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

