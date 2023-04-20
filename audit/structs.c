


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
