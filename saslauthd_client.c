/*
 * Verify user name and password with saslauthd
 *
 * Copyright 2017-2018 Andreas Voegele <andreas@andreasvoegele.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "saslauthd_client.h"

static void
set_error(struct saslauthd_credentials *cred, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  (void) vsnprintf(cred->error, sizeof(cred->error), format, ap);
  va_end(ap);
}

static ssize_t
write_string(int fd, const char *s, unsigned short len)
{
  unsigned short count;
  struct iovec iov[2];
  int iovcnt;
  ssize_t n;

  iovcnt = (s != NULL) ? 2 : 1;
  count = htons(len);
  iov[0].iov_base = (void *) &count;
  iov[0].iov_len = sizeof(count);
  iov[1].iov_base = (void *) s;
  iov[1].iov_len = len;

  while ((n = writev(fd, iov, iovcnt)) == -1 && errno == EINTR)
    ;

  return n;
}

static ssize_t
read_string(int fd, char **ps)
{
  unsigned short count;
  char *s;
  ssize_t n;

  while ((n = read(fd, &count, sizeof(count))) == -1 && errno == EINTR)
    ;
  if (n < (ssize_t) sizeof(count))
    return -1;

  count = ntohs(count);

  s = (char *) malloc(count + 1);
  if (s == NULL)
    return -1;

  while ((n = read(fd, s, count)) == -1 && errno == EINTR)
    ;
  if (n < (ssize_t) count)
  {
    free(s);
    return -1;
  }

  s[count] = '\0';
  *ps = s;

  return n;
}

int
saslauthd_checkpass(const char *path, struct saslauthd_credentials *cred)
{
  int fd = -1;
  struct sockaddr_un sun;
  char *reply = NULL;
  ssize_t reply_len;
  int rc = -1;

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    char error[128];
    strerror_r(errno, error, sizeof(error));
    set_error(cred, "Cannot create socket: %s", error);
    goto end;
  }

  memset(&sun, 0, sizeof(sun));
  sun.sun_family = AF_UNIX;
  strncpy(sun.sun_path, path, sizeof(sun.sun_path) - 1);
  if (connect(fd, (struct sockaddr *) &sun, sizeof(sun)) < 0)
  {
    char error[128];
    strerror_r(errno, error, sizeof(error));
    set_error(cred, "Cannot connect to '%s': %s", path, error);
    goto end;
  }

  if (write_string(fd, cred->user, cred->user_len) < 0)
  {
    set_error(cred, "Failed to send userid");
    goto end;
  }

  if (write_string(fd, cred->password, cred->password_len) < 0)
  {
    set_error(cred, "Failed to send password");
    goto end;
  }

  if (write_string(fd, cred->service, cred->service_len) < 0)
  {
    set_error(cred, "Failed to send service name");
    goto end;
  }

  if (write_string(fd, cred->realm, cred->realm_len) < 0)
  {
    set_error(cred, "Failed to send realm");
    goto end;
  }

  if ((reply_len = read_string(fd, &reply)) < 2)
  {
    set_error(cred, "Received invalid reply");
    goto end;
  }

  if (strncmp(reply, "OK", 2) == 0)
    rc = 0;
  else
  {
    const char *reason = "Unknown reason";
    if (strncmp(reply, "NO", 2) == 0)
    {
      rc = 1;
      if (reply_len > 3)
        reason = &reply[3];
    }
    set_error(cred, "%s", reason);
  }

end:
  if (reply != NULL)
    free(reply);
  if (fd >= 0)
    (void) close(fd);

  return rc;
}
