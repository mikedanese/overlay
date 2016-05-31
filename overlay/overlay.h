#ifndef OVERLAY_OVERLAY_H_
#define OVERLAY_OVERLAY_H_

#include <stdbool.h>

typedef struct {
  // cfg
  bool verbose;
  char *bin_path;
  char *target_path;
  char *config_path;
  // bookeeping
  int depth;
} overlayer;

#endif
