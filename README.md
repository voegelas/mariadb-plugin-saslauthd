# mariadb-plugin-saslauthd

This MariaDB plugin authenticates database users against the system password
file, LDAP or other mechanisms supported by saslauthd.

## ENABLE SASLAUTHD

On Slackware Linux, enable the saslauthd daemon with:

```
chmod +x /etc/rc.d/rc.saslauthd
/etc/rc.d/rc.saslauthd start
```

On OpenBSD, install the cyrus-sasl package and enable the saslauthd daemon
with rcctl(8).

On Ubuntu, install the sasl2-bin package and enable the saslauthd daemon. Add
the mysql user to the sasl group and restart the database system.

If saslauthd uses PAM, create the file /etc/pam.d/mariadb.

```
#%PAM-1.0
@include common-auth
@include common-account
```

If saslauthd uses LDAP, create /etc/saslauthd.conf.

```
ldap_servers: ldap://ad1.example.com ldap://ad2.example.com
ldap_start_tls: yes
ldap_search_base: OU=Users,DC=EXAMPLE,DC=COM
ldap_filter: (sAMAccountName=%u)
ldap_bind_dn: CN=saslauthd,OU=Users,DC=EXAMPLE,DC=COM
ldap_password: secret
```

## PLUGIN INSTALLATION

To install the plugin, run the following commands:

```
make OPTIMIZE="-O2" SASLAUTHD_PATH="/var/run/saslauthd/mux"
make install
```

Depending on your system the communications socket is named
/run/saslauthd/mux, /var/run/saslauthd/mux, /var/state/saslauthd/mux or
/var/sasl2/mux.

Enable the plugin with:

```
mysql -h localhost -u root -p \
      -e "INSTALL PLUGIN saslauthd SONAME 'auth_saslauthd.so'"
```

Or add the following section to your database server's configuration and
restart the database system:

```INI
[mariadb]
plugin-load-add=auth_saslauthd.so
```

List the installed plugins with:

```
mysql -h localhost -u root -p -e 'SHOW PLUGINS\G'

   Name: saslauthd
 Status: ACTIVE
   Type: AUTHENTICATION
Library: auth_saslauthd.so
License: BSD
```

## DATABASE USERS

Add a database account for the system user "jennifer" as follows:

```SQL
CREATE USER 'jennifer'@'localhost' IDENTIFIED WITH saslauthd;
```

Optionally, a different system user as well as a realm can be specified.
Example:

```SQL
CREATE USER 'jekyll' IDENTIFIED WITH saslauthd AS 'hyde@EXAMPLE.COM';
```

Run the mysql command-line tool and enter the system user's password to
check the database account. Example:

```
mysql -h localhost -u jennifer -p
Enter password:
```

## SECURE CONNECTIONS

saslauthd needs clear text passwords. Secure non-local connections
between your database clients and the server with TLS. See the MariaDB
documentation for more information.

## DEPENDENCIES

The plugin requires MariaDB and the saslauthd daemon from Cyrus SASL.

Building the plugin requires a C compiler and GNU Make 4 or a BSD make.

## LICENSE AND COPYRIGHT

Copyright 2017 Andreas Vögele

This plugin is free software; you can redistribute and modify it under the
terms of the ISC license.
