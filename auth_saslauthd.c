/*
 * Verify user name and password with saslauthd
 *
 * Copyright 2017 Andreas Voegele <andreas@andreasvoegele.com>
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

#include <mysql/plugin_auth.h>
#include <my_sys.h>
#include <mysqld_error.h>

#include <stdlib.h>
#include <string.h>

#include "saslauthd_client.h"

#ifndef SASLAUTHD_PATH
#define SASLAUTHD_PATH "/var/state/saslauthd/mux"
#endif

#ifndef SASLAUTHD_SERVICE
#define SASLAUTHD_SERVICE "mariadb"
#endif

static char *saslauthd_path;
static char *saslauthd_service;

static int
get_user(MYSQL_SERVER_AUTH_INFO *info, struct saslauthd_credentials *cred,
         char **pbuf)
{
  const char *user, *realm, *extuser;
  unsigned short user_len, realm_len, extuser_len;
  char *buf;

  user = info->user_name;
  user_len = info->user_name_length;
  realm = NULL;
  realm_len = 0;
  buf = NULL;

  extuser = user;
  extuser_len = user_len;

  if (info->auth_string != NULL && info->auth_string_length > 0)
  {
    char *sep;
    size_t buf_len;

    extuser = info->auth_string;
    extuser_len = info->auth_string_length;

    buf_len = info->auth_string_length;
    buf = (char *) malloc(buf_len + 1);
    if (buf == NULL)
      return 0;
    memcpy(buf, info->auth_string, buf_len);
    buf[buf_len] = '\0';

    sep = strrchr(buf, '@');
    if (sep == NULL)
    {
      user = buf;
      user_len = buf_len;
    }
    else
    {
      sep[0] = '\0';
      realm = &sep[1];
      realm_len = buf_len - (realm - buf);
      if (sep > buf)
      {
        user = buf;
        user_len = buf_len - realm_len - 1;
      }
    }
  }

  if (extuser != NULL)
  {
    size_t max_len = sizeof(info->external_user) - 1;
    size_t len = (extuser_len > max_len) ? max_len : extuser_len;
    strncpy(info->external_user, extuser, len);
    info->external_user[len] = '\0';
  }

  cred->user = user;
  cred->user_len = user_len;
  cred->realm = realm;
  cred->realm_len = realm_len;
  *pbuf = buf;

  return 1;
}

static int
saslauthd_auth(MYSQL_PLUGIN_VIO *vio, MYSQL_SERVER_AUTH_INFO *info)
{
  unsigned char *pkt;
  int pkt_len;
  struct saslauthd_credentials cred[1];
  char *buf = NULL;

  memset(cred, 0, sizeof(struct saslauthd_credentials));
  cred->service = saslauthd_service;
  cred->service_len = (saslauthd_service != NULL) ? strlen(saslauthd_service) : 0;
  if (!get_user(info, cred, &buf))
    return CR_ERROR;

  if (vio->write_packet(vio, (const unsigned char *) "\4Enter password:", 16))
    return CR_ERROR;

  if ((pkt_len = vio->read_packet(vio, &pkt)) < 0)
    return CR_ERROR;

  info->password_used = PASSWORD_USED_YES;

  cred->password = (const char *) pkt;
  cred->password_len = pkt_len;
  int rc = saslauthd_checkpass(saslauthd_path, cred);
  if (rc != 0)
    my_printf_error(ER_ACCESS_DENIED_ERROR,
                    "saslauthd authentication failed: %s",
                    MYF(0),
                    cred->error);
  if (buf != NULL)
    free(buf);
  return (rc == 0) ? CR_OK : CR_ERROR;
}

static MYSQL_SYSVAR_STR(path, saslauthd_path,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
                        "saslauthd communications socket",
                        NULL,
                        NULL,
                        SASLAUTHD_PATH);

static MYSQL_SYSVAR_STR(service, saslauthd_service,
                        PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_READONLY,
                        "saslauthd service name",
                        NULL,
                        NULL,
                        SASLAUTHD_SERVICE);

static struct st_mysql_sys_var *vars[] =
{
  MYSQL_SYSVAR(path),
  MYSQL_SYSVAR(service),
  NULL
};

static struct st_mysql_auth info =
{
  MYSQL_AUTHENTICATION_INTERFACE_VERSION,
  "mysql_clear_password",
  saslauthd_auth
};

mysql_declare_plugin(saslauthd)
{
  MYSQL_AUTHENTICATION_PLUGIN,
  &info,
  "saslauthd",
  "Andreas Voegele",
  "saslauthd based authentication",
  PLUGIN_LICENSE_BSD,
  NULL,
  NULL,
  0x0100,
  NULL,
  vars,
  NULL,
  0
}
mysql_declare_plugin_end;
