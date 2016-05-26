#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "libjsonnet.h"
#include "jansson.h"

#include "actions.h"

overlayer new_overlayer() {
	overlayer o = {
		.verbose = false,
		.config_path = "",
		.target_path = "",
		.bin_path = "",
	};
	return o;
}

int
validate_cfg(overlayer* o) {
	return (o->target_path == NULL) || (o->config_path == NULL);
}

void
usage(char* p, int code) {
	fprintf(stderr,
"%s:\n"
"\t--target_path\n"
"\t--config_path\n"
"\t--verbose|-v\n"
"\t--help|-h\n", p);
	exit(code);
}

int
parse_opts(overlayer* o, int argc, char** argv) {
	int cur = 0;
	o->bin_path = argv[0];

	cur++;
	while (argc > cur) {
		if (strcmp(argv[cur], "--target_path") == 0) {
			o->target_path = argv[++cur];
			cur++;
		} else if (strcmp(argv[cur], "--config_path") == 0) {
			o->config_path = argv[++cur];
			cur++;
		} else if ((strcmp(argv[cur], "--verbose")&strcmp(argv[cur], "-v")) == 0) {
			o->verbose = true;
			cur++;
		} else if ((strcmp(argv[cur], "--help")&strcmp(argv[cur], "-h")) == 0) {
			usage(o->bin_path, 0);
		} else {
			fprintf(stderr, "Unknown option '%s'.\n\n", argv[cur]);
			usage(o->bin_path, 1);
		}
	}
	return validate_cfg(o);
}

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

int
ovly_prln(int depth, const char *format, ...) {

#define FIRST KCYN
#define DELIM "--"
#define FINAL ">"KNRM
	va_list args;
	printf(FIRST);
	while (depth) {
		depth--;
		printf(DELIM);
	}
	printf(FINAL);
	va_start(args, format);
	vprintf(format, args);
	printf("\n");
}

#define oprf(o, fmt, ...) ovly_prln(o->depth, fmt, ##__VA_ARGS__)

int
do_action(overlayer* o, const char* name, json_t* obj) {
	oprf(o, "action '%s'", name);
	json_t* atype_obj = json_object_get(obj, "type");
	if (!atype_obj) {
		oprf(o, "action has no type field");
		return 1;
	}
	if (!json_is_string(atype_obj)) {
		oprf(o, "type field in action is not a string");
		return 1;
	}
	const char* atype = json_string_value(atype_obj);
	struct overlay_action* action = get_action(atype);
	if (!action) {
		oprf(o, "no action with name, %s", atype);
		return 1;
	}
	int err = action->handler(o, obj);
	oprf(o, "end action '%s'", name);
	return err;
}

int
do_module(overlayer* o, const char* name, json_t* obj) {
	oprf(o, "module '%s'", name);
	const char * key;
	json_t* val;

	if (!json_is_object(obj))
		return 1;

	int err = 0;

	json_t* before_actions = json_object_get(obj, "before_actions");
	if (before_actions) {
		oprf(o, "before actions");
		json_object_foreach(before_actions, key, val) {
			o->depth += 1;
			int err = do_action(o, key, val);
			o->depth -= 1;
			if (err)
				break;
		}
		oprf(o, "end before actions");
	}

	json_t* actions = json_object_get(obj, "actions");
	if (actions) {
		json_object_foreach(actions, key, val) {
			o->depth += 1;
			int err = do_action(o, key, val);
			o->depth -= 1;
			if (err)
				break;
		}
	}

	json_t* after_actions = json_object_get(obj, "after_actions");
	if (after_actions) {
		oprf(o, "after actions");
		json_object_foreach(after_actions, key, val) {
			o->depth += 1;
			int err = do_action(o, key, val);
			o->depth -= 1;
			if (err)
				break;
		}
		oprf(o, "end after actions");
	}

	if (err)
		return err;

	json_t* modules = json_object_get(obj, "modules");
	json_object_foreach(modules, key, val) {
		o->depth += 1;
		int err = do_module(o, key, val);
		o->depth -= 1;
		if (err)
			return err;
	}
	oprf(o, "end module '%s'", name);

	return 0;
}

int
main(int argc, char** argv) {

	overlayer o;

	if (parse_opts(&o, argc, argv)) {
		printf("cli opt parse error\n");
		return 1;
	}

	struct JsonnetVm* jsnt = jsonnet_make();

	int err = 0;
	const char* out = jsonnet_evaluate_file(jsnt, o.config_path, &err);
	if (err) {
		printf("error: %s\n", out);
		return 1;
	}
	jsonnet_destroy(jsnt);

	json_error_t jerr;
	if (o.verbose) {
		printf("effective config:\n%s\n", out);
	}
	json_t* root = json_loads(out, 0, &jerr);

	register_all_actions();

	err = do_module(&o, "root", root);
	if (err) {
		return 1;
	}
	json_decref(root);
	return 0;
}
