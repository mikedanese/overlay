#ifndef ACTIONS_H_
#define ACTIONS_H_

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "jansson.h"

#include "overlay.h"

#define MAX_ACTIONS 1 << 6

struct overlay_action {
  char *name;
  int (*handler)(overlayer *, json_t *);
};

struct overlay_action *actions[MAX_ACTIONS];

int register_action(struct overlay_action);

int register_all_actions(void);

struct overlay_action *get_action(const char *name);

#endif
