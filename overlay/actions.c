#include "overlay/actions.h"
#include "overlay/util.h"

#define check_required_key(key, args)                                          \
  do {                                                                         \
    if (!json_object_get(args, key)) {                                         \
      printf("object does not have required key: '%s'", key);                  \
      return 1;                                                                \
    }                                                                          \
  } while (0);

#define yank_string(var, key, args)                                            \
  do {                                                                         \
    if (!json_is_string(json_object_get(args, key))) {                         \
      printf("'%s' is not a string", key);                                     \
      return 1;                                                                \
    }                                                                          \
    var = json_string_value(json_object_get(args, key));                       \
  } while (0);

// actions
int exec_action_helper(overlayer *o, json_t *args, bool host) {
  pid_t pid = fork();
  if (pid) {
    int status = 0;
    waitpid(pid, &status, 0);
    return status;
  }

  char *cmd;
  check_required_key("cmd", args);
  yank_string(cmd, "cmd", args);

  if (!host && chroot(o->target_path)) {
    int err = errno;
    perror("chroot failed");
    exit(err);
  }

  char *const argv[] = {"/bin/bash", "-c", cmd, NULL};
  char *const envp[] = {NULL};
  if (execve("/bin/bash", argv, envp)) {
    int err = errno;
    perror("exec failed");
    exit(err);
  }
  printf("unreachable");
  return 99;
}

int do_exec_action(overlayer *o, json_t *args) {
  return exec_action_helper(o, args, false);
}

int do_host_exec_action(overlayer *o, json_t *args) {
  return exec_action_helper(o, args, true);
}

int do_copy_action(overlayer *o, json_t *args) {
  char *src, *dest, *fulldest;

  check_required_key("src", args);
  yank_string(src, "src", args);

  check_required_key("dest", args);
  yank_string(dest, "dest", args);

  fulldest = ovrly_joinpath(o->target_path, dest);
  return ovrly_copy(src, fulldest);
}

// end actions

struct overlay_action *get_action(const char *name) {
  int i;
  for (i = 0; actions[i]; i++)
    if (strcmp(actions[i]->name, name) == 0)
      return actions[i];
  return NULL;
}

int register_all_actions() {
  // TODO enforce run once
  struct overlay_action exec_action = {
      .name = "exec", .handler = do_exec_action,
  };
  register_action(exec_action);

  struct overlay_action host_exec_action = {
      .name = "host_exec", .handler = do_host_exec_action,
  };
  register_action(host_exec_action);

  struct overlay_action copy_action = {
      .name = "copy", .handler = do_copy_action,
  };
  register_action(copy_action);

  return 0;
}

int register_action(struct overlay_action action) {
  int i;
  for (i = 0; i < MAX_ACTIONS; i++) {
    if (actions[i])
      continue;
    actions[i] = malloc(sizeof(struct overlay_action));
    *actions[i] = action;
    break;
  }
  // no space
  return 1;
}
