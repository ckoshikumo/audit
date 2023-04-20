



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
