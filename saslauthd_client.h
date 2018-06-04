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

struct saslauthd_credentials
{
  const char *user;
  unsigned short user_len;

  const char *password;
  unsigned short password_len;

  const char *service;
  unsigned short service_len;

  const char *realm;
  unsigned short realm_len;

  char error[256];
};

/**
 * Verifies a user name and password with saslauthd.
 *
 * @param  path the saslauthd communications socket
 * @param  cred a filled saslauthd_credentials object
 * @return      0 on success, 1 on authentication failure and -1 on error
 */
extern int saslauthd_checkpass(const char *, struct saslauthd_credentials *);
