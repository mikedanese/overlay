#include <stdio.h>
#include <stdlib.h>
#include "ucl.h"

#define ucl_parser_check(ucl) do {					\
	if (!ucl_parser_get_error(ucl)) 				\
		break;							\
	printf("Error occurred: %s\n", ucl_parser_get_error(ucl));	\
	return 0;							\
} while(0)


struct overlay_ucl_macro {
	char* name;
	ucl_macro_handler handler;
	void* ud;
};

struct overlay_ucl_macro* macros[] = {NULL};

struct overlay_cfg {
	char* bin_path;
	char* target_path;
	char* config_path;
};

int
validate_cfg(struct overlay_cfg* cfg) {
	return (cfg->target_path == NULL) || (cfg->config_path == NULL);
}

int
parse_opts(struct overlay_cfg* cfg, int argc, char** argv) {
	int cur = 0;
	cfg->bin_path = argv[0];
	cur++;
	while (argc > cur) {
		if (strcmp(argv[cur], "--target_path") == 0) {
			cfg->target_path = argv[++cur];
			cur++;
		} else if (strcmp(argv[cur], "--config_path") == 0) {
			cfg->config_path = argv[++cur];
			cur++;
		} else {
			return 1;
		}
	}
	return validate_cfg(cfg);
}

int
main(int argc, char** argv) {

	struct overlay_cfg cfg;

	if (parse_opts(&cfg, argc, argv)) {
		printf("cli opt parse error\n");
		return 1;
	}

	struct ucl_parser* ucl = ucl_parser_new(0);

	for (int i = 0; macros[i]; i++)
		ucl_parser_register_macro(ucl,
				macros[i]->name,
				macros[i]->handler,
				macros[i]->ud);


	ucl_parser_set_filevars(ucl, cfg.config_path, true);
	ucl_parser_check(ucl);

	ucl_parser_add_file(ucl, cfg.config_path);
	ucl_parser_check(ucl);

	ucl_object_t* obj = ucl_parser_get_object(ucl);
	unsigned char* str = ucl_object_emit(obj, UCL_EMIT_JSON);

	printf("%s\n",str);
	free(str);



	return 0;
}
