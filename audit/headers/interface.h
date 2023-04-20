

// INTERFACE:
#define audit(...) _audit_def(audit_, _audit_narg(__VA_ARGS__))(__VA_ARGS__)

#define check(_assert, _desc, _msg, ...)                                                           \
	do {                                                                                       \
		_check_internals(_assert);                                                         \
		_audit_store_message(_audit_pre_msg _desc " [" _msg "]", __FILE__, __LINE__,       \
				     ##__VA_ARGS__);                                               \
	} while (0)

#define check_eq(_lhs, _rhs, _fmt, _desc) _check(==, _lhs, _rhs, _fmt, _desc, _eq_msg)
#define check_neq(_lhs, _rhs, _fmt, _desc) _check(!=, _lhs, _rhs, _fmt, _desc, _audit_neq_msg)
#define check_lt(_lhs, _rhs, _fmt, _desc) _check(<, _lhs, _rhs, _fmt, _desc, _audit_lt_msg)
#define check_gt(_lhs, _rhs, _fmt, _desc) _check(>, _lhs, _rhs, _fmt, _desc, _audit_gt_msg)
#define check_lteq(_lhs, _rhs, _fmt, _desc) _check(<=, _lhs, _rhs, _fmt, _desc, _audit_lteq_msg)
#define check_gteq(_lhs, _rhs, _fmt, _desc) _check(>=, _lhs, _rhs, _fmt, _desc, _audit_gteq_msg)

