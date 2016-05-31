#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "overlay/util.h"

int ovrly_copy(char *src, char *dest) {
#define BUF_SIZE 1 << 14
  char buf[BUF_SIZE];
  int n;

  int src_fd = open(src, O_RDONLY);
  int dest_fd = open(dest, O_CREAT | O_TRUNC);

  while ((n = read(src_fd, buf, BUF_SIZE)) > 0) {
    if (write(dest_fd, buf, n) != n) {
      perror("couldn't write whole buffer");
      close(src_fd);
      close(dest_fd);
      return 1;
    }
  }

  close(src_fd);
  close(dest_fd);
  return 0;
}

// TODO: so bad...
char *ovrly_joinpath(char *p1, char *p2) {
  int l1 = strlen(p1);
  int l2 = strlen(p2);
  char *out = malloc(sizeof(char) * (l1 + l2 + 2));
  *out = '\0';
  strcat(out, p1);
  if ((p1[l1 - 1] != '/') && (p2[0] != '/'))
    strcat(out, "/");
  strcat(out, p2);
  return out;
}
