#ifndef INCLUDE_AUDIT_H
#define INCLUDE_AUDIT_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef AUDIT_INITIAL_N_AUDITS
#define AUDIT_INITIAL_N_AUDITS 20
#endif

#ifndef ADT_API
#define ADT_API
#endif

//
// API
//
#define AUDIT_FILE_DESCRIPTION(description)                                    \
	__attribute__((constructor(200))) static void adt_register_desc(void) {    \
		adt_current_file.desc = description;                                   \
	}

#define AUDIT_FILE_SETUP()                                                     \
	static void adt_file_setup(void);                                          \
	__attribute__((constructor(200))) static void adt_register_setup(void) {   \
		adt_current_file.setup = adt_file_setup;                               \
	}                                                                          \
	static void adt_file_setup(void)

#define AUDIT_FILE_TEARDOWN()                                                  \
	static void adt_file_teardown(void);                                       \
	__attribute__((constructor(200))) static void adt_register_teardown(       \
	    void) {                                                                \
		adt_current_file.teardown = adt_file_teardown;                         \
	}                                                                          \
	static void adt_file_teardown(void)

#define AUDIT(...) adt_def(audit_, adt_narg(__VA_ARGS__))(__VA_ARGS__)

#define VERIFY(assertion, desc, msg, ...)                                      \
	adt_run_verify(&adt_current_file, this, __LINE__, (assertion), desc,       \
	               msg __VA_OPT__(, ) __VA_ARGS__);

#define VERIFY_EQ(lhs, rhs, desc) adt_builtin_comp(==, lhs, rhs, desc)

#define VERIFY_NEQ(lhs, rhs, desc) adt_builtin_comp(!=, lhs, rhs, desc)

#define VERIFY_LT(lhs, rhs, desc) adt_builtin_comp(<, lhs, rhs, desc)

#define VERIFY_LTEQ(lhs, rhs, desc) adt_builtin_comp(<, lhs, rhs, desc)

#define VERIFY_GT(lhs, rhs, desc) adt_builtin_comp(>, lhs, rhs, desc)

#define VERIFY_GTEQ(lhs, rhs, desc) adt_builtin_comp(>=, lhs, rhs, desc)

#define VERIFY_TRUE(v, desc) VERIFY((v), desc, "expected true")

#define VERIFY_FALSE(v, desc) VERIFY(!(v), desc, "expected false")

#define VERIFY_NULL(v, desc) VERIFY((v == NULL), desc, "expected NULL")

#define VERIFY_NN(v, desc) VERIFY((v != NULL), desc, "expected not NULL")

//
// Internals
//

/// Must run AFTER adt_file_init
#define adt_audit_constructor(audit_name, st, td, line)                        \
	static void adt_concat(adt_audit_, line)(adtAudit * this);                 \
	__attribute__((constructor(300))) static void adt_concat(adt_init_,        \
	                                                         line)(void) {     \
		adt_register_audit(&adt_current_file, audit_name,                      \
		                   adt_concat(adt_audit_, line), st, td);              \
		adt_current_file.name = __FILE__;                                      \
	}                                                                          \
	static void adt_concat(adt_audit_, line)(adtAudit * this)

/// Automatic string specifiers
#define adt_fmt(x)                                                             \
	_Generic((x),                                                              \
	    char: "%c",                                                            \
	    unsigned char: "%d",                                                   \
	    signed char: "%d",                                                     \
	    unsigned short: "%d",                                                  \
	    short: "%d",                                                           \
	    unsigned int: "%u",                                                    \
	    int: "%d",                                                             \
	    unsigned long: "%u",                                                   \
	    long: "%d",                                                            \
	    unsigned long long: "%llu",                                            \
	    long long: "%lld",                                                     \
	    float: "%f",                                                           \
	    double: "%f",                                                          \
	    long double: "%Lf",                                                    \
	    char *: "%s",                                                          \
	    void *: "%p",                                                          \
	    int *: "%p",                                                           \
	    unsigned char *: "%p",                                                 \
	    signed char *: "%p",                                                   \
	    unsigned short *: "%p",                                                \
	    short *: "%p",                                                         \
	    unsigned int *: "%p",                                                  \
	    unsigned long *: "%p",                                                 \
	    long *: "%p",                                                          \
	    unsigned long long *: "%p",                                            \
	    long long *: "%p",                                                     \
	    float *: "%p",                                                         \
	    double *: "%p",                                                        \
	    long double *: "%p",                                                   \
	    default: _Generic((x), size_t: "%zu", size_t *: "%zu", default: "%d"))

#define adt_builtin_comp(op, lhs, rhs, desc)                                   \
	do {                                                                       \
		auto _lhs = (lhs);                                                     \
		auto _rhs = (rhs);                                                     \
		auto _res = _lhs op _rhs;                                              \
		const char *_lhs_fmt = adt_fmt(lhs);                                   \
		const char *_rhs_fmt = adt_fmt(rhs);                                   \
		char _msg_buf[128];                                                    \
		snprintf(_msg_buf, sizeof(_msg_buf), "expected %s " #op " %s",         \
		         _lhs_fmt, _rhs_fmt);                                          \
		char _final_buf[128];                                                  \
		snprintf(_final_buf, sizeof(_final_buf), _msg_buf, _lhs, _rhs);        \
		VERIFY(_res, desc, _final_buf);                                        \
	} while (0)

/// Helpers
#define adt_concat_1(v1, v2) v1##v2
#define adt_concat(v1, v2) adt_concat_1(v1, v2)

/// Default arguments
#define adt_narg(...) adt_arg_1(__VA_ARGS__, adt_rseq_n())
#define adt_arg_1(...) adt_arg_n(__VA_ARGS__)
#define adt_arg_n(_1, _2, _3, _4, N, ...) N
#define adt_rseq_n() 4, 3, 2, 1, 0

#define adt_def_1(name, num) adt_##name##num
#define adt_def(name, num) adt_def_1(name, num)

#define adt_audit_1(name) adt_audit_constructor(name, NULL, NULL, __LINE__)
#define adt_audit_2(name, st_fn)                                               \
	adt_audit_constructor(name, st_fn, NULL, __LINE__)
#define adt_audit_3(name, st_fn, td_fn)                                        \
	adt_audit_constructor(name, st_fn, td_fn, __LINE__)

/// Types
typedef struct adtAudit adtAudit;
typedef struct adtFile adtFile;

typedef void (*adtAuditFn)(adtAudit *this);
typedef void (*adtSetupFn)(void);
typedef adtSetupFn adtTeardownFn;

typedef struct adtFile {
	bool selected, failed;
	bool audits_selected;
	size_t idx;
	adtSetupFn setup;
	adtTeardownFn teardown;
	const char *name;
	const char *desc;

	size_t count, max;
	adtAudit *data;
} adtFile;

typedef struct adtAudit {
	bool selected, failed;
	size_t idx;
	adtAuditFn fn;
	adtSetupFn setup;
	adtTeardownFn teardown;
	const char *description;

	size_t count, max; // Count of FAILED verifies
	char **data;       // Messages of FAILED verifies
} adtAudit;

typedef struct {
	bool empty_selection;
	bool use_colors;
	size_t total_files, failed_files;
	size_t total_audits, failed_audits;
	size_t total_verifies, failed_verifies;

	size_t count, max;
	adtFile **data;
} adtState;

/// Statics
static adtFile adt_current_file = {
    .selected = false,
    .audits_selected = false,
    .failed = false,
    .count = 0,
    .max = AUDIT_INITIAL_N_AUDITS,
    .desc = NULL,
    .setup = NULL,
    .teardown = NULL,
};

/// Forward declarations
ADT_API void adt_register_file(adtFile *file);
ADT_API void adt_register_audit(adtFile *file, const char *desc, adtAuditFn fn,
                        adtSetupFn st, adtSetupFn td);
ADT_API void adt_run_verify(adtFile *file, adtAudit *audit, int linum, bool result,
                    const char *desc, const char *error_fmt, ...);

/// File constructor (must run AFTER AUDIT_FILE_DESCRIPTION, SETUP and
/// TEARDOWN):
__attribute__((constructor(250))) static void adt_file_init(void) {
	adt_register_file(&adt_current_file);
}

/// Globals
extern adtState adt_state;

#endif // INCLUDE_AUDIT_H

// #define AUDIT_IMPLEMENTATION

#ifdef AUDIT_IMPLEMENTATION

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"
#include <unistd.h>

//
// Customization
//

#ifndef AUDIT_PASS_VERIFY_STR
#define AUDIT_PASS_VERIFY_STR "."
#endif // AUDIT_PASS_VERIFY_STR

#ifndef AUDIT_FAIL_VERIFY_STR
#define AUDIT_FAIL_VERIFY_STR "X"
#endif // AUDIT_FAIL_VERIFY_STR

#ifdef AUDIT_DEFAULT_NO_COLOR
#define AUDIT_USE_COLORS false
#else
#define AUDIT_USE_COLORS true
#endif // AUDIT_DEFAULT_NO_COLOR

#ifndef AUDIT_CLR_FAIL
#define AUDIT_CLR_FAIL "\x1b[31m"
#endif

#ifndef AUDIT_CLR_OK
#define AUDIT_CLR_OK "\x1b[32m"
#endif

#ifndef AUDIT_CLR_INFO
#define AUDIT_CLR_INFO "\x1b[33m"
#endif

#define AUDIT_CLR_RESET "\x1b[0m"

#ifndef AUDIT_INITIAL_N_FILES
#define AUDIT_INITIAL_N_FILES 10
#endif

#ifndef AUDIT_INITIAL_N_VERIFIES
#define AUDIT_INITIAL_N_VERIFIES 20
#endif

//
// Implementation
//

/// Globals
adtState adt_state = {.empty_selection = true,
                      .max = AUDIT_INITIAL_N_FILES,
                      .use_colors = AUDIT_USE_COLORS};

// Printer fns
static void (*out_ok)(const char *fmt, ...);
static void (*out_fail)(const char *fmt, ...);
static void (*out_info)(const char *fmt, ...);
static void (*out_plain)(const char *fmt, ...);
static void (*err_fail)(const char *fmt, ...);
static void (*err_info)(const char *fmt, ...);
static void (*err_plain)(const char *fmt, ...);

static void colored_out(const char *clr, const char *fmt, va_list ap) {
	fprintf(stdout, "%s", clr);
	vfprintf(stdout, fmt, ap);
	fprintf(stdout, AUDIT_CLR_RESET);
}

static void colored_err(const char *clr, const char *fmt, va_list ap) {
	fprintf(stderr, "%s", clr);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, AUDIT_CLR_RESET);
}

static void colored_ok(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	colored_out(AUDIT_CLR_OK, fmt, ap);
	va_end(ap);
}

static void colored_out_fail(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	colored_out(AUDIT_CLR_FAIL, fmt, ap);
	va_end(ap);
}

static void colored_out_info(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	colored_out(AUDIT_CLR_INFO, fmt, ap);
	va_end(ap);
}

static void colored_err_fail(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	colored_err(AUDIT_CLR_FAIL, fmt, ap);
	va_end(ap);
}

static void colored_err_info(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	colored_err(AUDIT_CLR_INFO, fmt, ap);
	va_end(ap);
}

static void plain_out(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}

static void plain_err(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

static void setup_colors(void) {
	out_plain = plain_out;
	err_plain = plain_err;

	if (adt_state.use_colors) {
		out_ok = colored_ok;
		out_fail = colored_out_fail;
		out_info = colored_out_info;
		err_fail = colored_err_fail;
		err_info = colored_err_info;

	} else {
		out_ok = plain_out;
		out_fail = plain_out;
		out_info = plain_out;
		err_fail = plain_err;
		err_info = plain_err;
	}
}

#define check_mem(assertion)                                                   \
	if ((assertion)) {                                                         \
		err_fail("Fatal: ");                                                   \
		fprintf(stderr, "memory error");                                       \
		fprintf(stderr, "\n");                                                 \
		exit(EXIT_FAILURE);                                                    \
	}

#define adt_ensure_capacity(registry)                                          \
	do {                                                                       \
		if ((registry).count == (registry).max) {                              \
			(registry).max *= 1.5;                                             \
			(registry).data = realloc(                                         \
			    (registry).data, sizeof(*(registry).data) * (registry).max);   \
			check_mem(!(registry).data);                                       \
		}                                                                      \
	} while (0)

static const char linum_str[] = "line %d: ";

/// Public functions
ADT_API void adt_register_file(adtFile *file) {
	adt_ensure_capacity(adt_state);

	file->data = malloc(sizeof(*file->data) * file->max);
	check_mem(!file->data);

	size_t idx = adt_state.count++;
	file->idx = idx;
	adt_state.data[idx] = file;
}

ADT_API void adt_register_audit(adtFile *file, const char *desc, adtAuditFn fn,
                        adtSetupFn st, adtSetupFn td) {
	adt_ensure_capacity(*file);

	size_t audit_idx = file->count++;
	adtAudit audit = {
	    .description = desc,
	    .idx = audit_idx,
	    .fn = fn,
	    .setup = st,
	    .teardown = td,
	    .failed = false,
	    .count = 0,
	    .max = AUDIT_INITIAL_N_VERIFIES,
	    .data = malloc(sizeof(*audit.data) * AUDIT_INITIAL_N_VERIFIES)};

	check_mem(!audit.data);
	file->data[audit_idx] = audit;
}

ADT_API void adt_run_verify(adtFile *file, adtAudit *audit, int linum, bool result,
                    const char *desc, const char *error_fmt, ...) {

	if (adt_state.total_verifies > 0 && adt_state.total_verifies % 80 == 0)
		out_plain("\n");

	adt_state.total_verifies++;

	if (result) {
		out_ok(AUDIT_PASS_VERIFY_STR);
		return;
	}

	out_fail(AUDIT_FAIL_VERIFY_STR);
	adt_state.failed_verifies++;

	if (!file->failed) {
		adt_state.failed_files++;
		file->failed = true;
	}

	if (!audit->failed) {
		adt_state.failed_audits++;
		audit->failed = true;
	}

	int prefix_len = snprintf(NULL, 0, linum_str, linum);
	int desc_len = (int) strlen(desc);

	va_list ap;
	va_start(ap, error_fmt);
	int msg_len = vsnprintf(NULL, 0, error_fmt, ap);
	va_end(ap);

	int open_color_len = adt_state.use_colors ? strlen(AUDIT_CLR_INFO) : 0;
	int close_color_len = adt_state.use_colors ? strlen(AUDIT_CLR_RESET) : 0;

	// 2 for " [", 1 for "]", 1 for '\0'
	size_t total = (size_t) prefix_len + desc_len + open_color_len + msg_len +
	               close_color_len + 4;

	char *buf = malloc(total);
	check_mem(!buf);

	char *p = buf;
	p += snprintf(p, prefix_len + 1, linum_str, linum);
	memcpy(p, desc, desc_len);
	p += desc_len;

	if (adt_state.use_colors) {
		memcpy(p, AUDIT_CLR_INFO, open_color_len);
		p += open_color_len;
	}

	memcpy(p, " [", 2);
	p += 2;

	va_start(ap, error_fmt);
	vsnprintf(p, msg_len + 1, error_fmt, ap);
	va_end(ap);
	p += msg_len;

	*p++ = ']';

	if (adt_state.use_colors) {
		memcpy(p, AUDIT_CLR_RESET, close_color_len);
		p += close_color_len;
	}

	*p = '\0';

	adt_ensure_capacity(*audit);
	audit->data[audit->count++] = buf;
}

typedef struct adtParseListArgs {
	const char *spec;
	const char *list;
	char delim;
	size_t file_idx;
	size_t max;
	void (*on_valid)(size_t idx, void *ctx);
	void *ctx;
} adtParseListArgs;

static void bail(const char *fmt, ...) {
	err_fail("Error: ");
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	err_plain("\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}

static void bail_file_or_audit(size_t idx, size_t max, size_t file_idx,
                               const char *spec) {
	if (file_idx == SIZE_MAX) {
		if (spec) bail("file index range %s is invalid (0-%zu)", spec, max);
		else bail("file index %zu out of range (0..%zu)", idx, max);
	} else {
		if (spec)
			bail("audit index range %s is invalid in "
			     "file index %zu (0-%zu)",
			     spec, file_idx, max);
		else
			bail("audit index %zu out of range in "
			     "file index %zu (0-%zu)",
			     idx, file_idx, max);
	}
}

static int parse_index_list(adtParseListArgs args) {
	char *copy = strdup(args.list);
	check_mem(!copy);

	char sep[2] = {args.delim, '\0'};
	char *saveptr = NULL;
	char *tok = strtok_r(copy, sep, &saveptr);

	while (tok) {
		char *dash = strchr(tok, '-');

		if (dash) { // Range
			*dash = '\0';
			char *start_str = tok;
			char *end_str = dash + 1;
			char *end_ptr = NULL;

			size_t start = strtoul(start_str, &end_ptr, 10);
			if (*end_ptr != '\0') bail("invalid spec: '%s'", args.spec);

			end_ptr = NULL;
			size_t end = strtoul(end_str, &end_ptr, 10);
			if (*end_ptr != '\0' || end < start)
				bail("invalid spec: '%s'", args.spec);

			for (size_t idx = start; idx <= end; idx++) {
				if (idx >= args.max)
					bail_file_or_audit(idx, args.max - 1, args.file_idx,
					                   args.spec);
				args.on_valid(idx, args.ctx);
			}

		} else { // Single index
			char *end_ptr = NULL;
			size_t idx = strtoul(tok, &end_ptr, 10);
			if (*end_ptr != '\0') bail("invalid spec: %s", args.spec);
			if (idx >= args.max)
				bail_file_or_audit(idx, args.max - 1, args.file_idx, NULL);
			args.on_valid(idx, args.ctx);
		}

		tok = strtok_r(NULL, sep, &saveptr);
	}

	free(copy);
	return 0;
}

static void mark_file_selected(size_t idx, void *ctx) {
	(void) ctx;
	adtFile *file = adt_state.data[idx];
	if (file->count > 0) {
		file->selected = true;
		adt_state.empty_selection = false;
	}
}

static void select_files(const char *str) {
	parse_index_list((adtParseListArgs) {.file_idx = SIZE_MAX,
	                                     .spec = str,
	                                     .list = str,
	                                     .delim = ',',
	                                     .max = adt_state.count,
	                                     .on_valid = mark_file_selected,
	                                     .ctx = NULL});
}

typedef struct {
	adtFile *file;
	const char *spec;
} adtCtx;

static void mark_audit_selected(size_t idx, void *vctx) {
	adtCtx *ctx = vctx;
	ctx->file->data[idx].selected = true;
	adt_state.empty_selection = false;
}

static int select_audits(const char *spec) {
	char *copy = strdup(spec);
	check_mem(!copy);

	char *colon = strchr(copy, ':');
	if (!colon) bail("invalid audit spec: '%s'", spec);

	*colon = '\0';
	char *file_str = copy;
	char *audit_str = colon + 1;
	char *end_ptr = NULL;

	size_t file_idx = strtoul(file_str, &end_ptr, 10);

	if (*end_ptr != '\0') bail("invalid audit spec: %s", spec);
	if (file_idx >= adt_state.count)
		bail("file index %zu out of range (0..%zu)", file_idx,
		     adt_state.count - 1);

	adtFile *file = adt_state.data[file_idx];
	adtCtx ctx = {.file = file, .spec = spec};

	int ret =
	    parse_index_list((adtParseListArgs) {.file_idx = file_idx,
	                                         .spec = spec,
	                                         .list = audit_str,
	                                         .delim = ',',
	                                         .max = file->count,
	                                         .on_valid = mark_audit_selected,
	                                         .ctx = &ctx});

	if (ret == 0) file->audits_selected = true;

	free(copy);
	return ret;
}

static void print_file_info(adtFile *file, bool fail) {
	void (*prn_fn)(const char *fmt, ...) = fail ? out_fail : out_info;
	out_plain("File [");
	prn_fn("%zu", file->idx);
	out_plain("] ");

	if (file->desc) out_plain("%s (%s)\n", file->desc, file->name);
	else out_plain("%s\n", file->name);
}

static void print_audit_info(adtAudit *audit, bool fail) {
	void (*prn_fn)(const char *fmt, ...) = fail ? out_fail : out_info;
	out_plain("  Audit [");
	prn_fn("%zu", audit->idx);
	out_plain("] %s\n", audit->description);
}

static void print_failures(void) {
	for (size_t i = 0; i < adt_state.count; i++) {
		adtFile *file = adt_state.data[i];
		if (file->failed) {
			print_file_info(file, true);

			for (size_t j = 0; j < file->count; j++) {
				adtAudit *audit = &file->data[j];
				if (audit->failed) {
					print_audit_info(audit, true);

					for (size_t k = 0; k < audit->count; k++)
						out_plain("    %s\n", audit->data[k]);

					out_plain("\n");
				}
			}
		}
	}
}

static void print_available(void) {
	out_ok("Available files and audits:\n");

	for (size_t i = 0; i < adt_state.count; i++) {
		adtFile *file = adt_state.data[i];
		out_plain("\n");
		print_file_info(file, false);

		for (size_t j = 0; j < file->count; j++)
			print_audit_info(&file->data[j], false);
	}

	out_plain("\n");
}

static void print_help(void) {
	out_plain(
	    "Usage: audit [options]\n\n"
	    "If no options are provided, all files and audits will be run.\n\n"
	    "Options:\n"
	    "  -l, --list       List available files and audits with indices.\n"
	    "  -f, --files <file_indices>\n"
	    "                   Select files (comma-separated list or range).\n"
	    "  -a, --audits <file_index:audit_indices>\n"
	    "                   Select audits (comma-separated list or range) "
	    "in file.\n"
	    "  -c, --color      Force colored output.\n"
	    "  -n, --no-color   Force uncolored output.\n"
	    "  -h, --help       Show this help.\n\n"
	    "Examples:\n"
	    "  # Selecting files:\n"
	    "  audit -f 1,3,5   # Run files 1, 3 and 5\n"
	    "  audit -f 0-2     # Run files 0, 1 and 2\n\n"
	    "  # Selecting audits:\n"
	    "  audit -a 0:2,4,5 # Run audits 2, 4 and 5 of file 0\n"
	    "  audit -a 3:4-7   # Run audits 4, 5, 6 and 7 of file 3\n\n"
	    "  # Multiple selections:\n"
	    "  audit -f 1,2 -a 3:3,4 -a 5:0-4\n");
}

static void run_audit(adtFile *file, adtAudit *audit) {
	// TODO: Initial value for functions should be a SENTINEL, not NULL (so
	// that, if the user specifies NULL at the audit level, we don't run the
	// file level function).
	adt_state.total_audits++;

	adtSetupFn setup = audit->setup ? audit->setup : file->setup;
	adtTeardownFn teardown = audit->teardown ? audit->teardown : file->teardown;

	if (setup) setup();
	audit->fn(audit);
	if (teardown) teardown();
}

static void run_file(adtFile *file) {
	adt_state.total_files++;
	for (size_t i = 0; i < file->count; i++) run_audit(file, &file->data[i]);
}

static void run_selected(void) {
	out_plain("Running selected files and audits.\n\n");
	for (size_t i = 0; i < adt_state.count; i++) {
		adtFile *file = adt_state.data[i];

		if (file->selected) run_file(file);
		else if (file->audits_selected)
			for (size_t j = 0; j < file->count; j++)
				if (file->data[j].selected) run_audit(file, &file->data[j]);
	}
}

static void run_all(void) {
	out_plain("Running all audits.\n\n");
	for (size_t i = 0; i < adt_state.count; i++) run_file(adt_state.data[i]);
}

__attribute__((destructor(101))) static void audit_shutdown(void) {
	if (!adt_state.data) return;

	for (size_t i = 0; i < adt_state.count; i++) {
		adtFile *file = adt_state.data[i];
		if (!file) continue;

		if (file->data) {
			for (size_t j = 0; j < file->count; j++) {
				adtAudit *audit = &file->data[j];
				if (audit->data) {
					for (size_t k = 0; k < audit->count; k++)
						free(audit->data[k]);
					free(audit->data);
				}
			}
			free(file->data);
		}
	}
	free(adt_state.data);
}

__attribute__((constructor(101))) static void audit_init(void) {
	adt_state.data = malloc(sizeof(*adt_state.data) * adt_state.max);
	check_mem(!adt_state.data);

	adt_state.use_colors = isatty(STDOUT_FILENO) && !getenv("NO_COLOR");
	setup_colors();
}

int main(int argc, char **argv) {
	bool selected = false;
	bool available = false;
	bool help = false;

	if (argc >= 2) {
		struct optparse_long longopts[] = {{"list", 'l', OPTPARSE_NONE},
		                                   {"help", 'h', OPTPARSE_NONE},
		                                   {"files", 'f', OPTPARSE_REQUIRED},
		                                   {"audits", 'a', OPTPARSE_REQUIRED},
		                                   {"no-color", 'n', OPTPARSE_NONE},
		                                   {"color", 'c', OPTPARSE_NONE},
		                                   {0}};

		struct optparse opts;
		optparse_init(&opts, argv);
		int option;

		while ((option = optparse_long(&opts, longopts, NULL)) != -1) {
			switch (option) {
			case 'l':
				available = true;
				break;
			case 'h':
				help = true;
				break;
			case 'c':
				adt_state.use_colors = true;
				break;
			case 'n':
				adt_state.use_colors = false;
				break;
			case 'f':
				select_files(opts.optarg);
				selected = true;
				break;
			case 'a':
				select_audits(opts.optarg);
				selected = true;
				break;
			case '?':
				fprintf(stderr, "Error: %s: %s\n", argv[0], opts.errmsg);
				exit(EXIT_FAILURE);
			}
		}

		setup_colors();

		if (help) {
			print_help();
			return 0;
		}

		if (available) {
			print_available();
			return 0;
		}

		if (opts.optind < argc) {
			err_fail("Error: ");
			err_plain("unexpected argument '%s'\n", optparse_arg(&opts));
			return -1;
		}
	}

	if (!selected) {
		for (size_t i = 0; i < adt_state.count; i++) {
			if (adt_state.data[i]->count > 0) {
				adt_state.empty_selection = false;
				break;
			}
		}
	}

	if (adt_state.empty_selection) {
		err_fail("\nError: ");
		err_plain("no available files or audits to run.\n\n");
		return -1;
	}

	out_ok("AUDIT START\n\n");
	selected ? run_selected() : run_all();
	out_plain("\n\n");

	if (adt_state.failed_audits == 0) {
		out_ok("AUDIT OK\n\n");
	} else {
		out_fail("AUDIT FAILED\n\n");
		print_failures();
	}

	out_info("%zu files (%zu failed), %zu audits (%zu failed), %zu "
	         "verifications "
	         "(%zu failed)\n",
	         adt_state.total_files, adt_state.failed_files,
	         adt_state.total_audits, adt_state.failed_audits,
	         adt_state.total_verifies, adt_state.failed_verifies);

	return adt_state.failed_audits;
}

#endif // AUDIT_IMPLEMENTATION
