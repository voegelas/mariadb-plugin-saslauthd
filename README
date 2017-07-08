auth_saslauthd.so is a MariaDB plugin that authenticates database users with
saslauthd against the system password database, e.g. /etc/shadow.

SECURE CONNECTIONS

saslauthd requires the passwords in clear text. Make sure that non-local
connections between your database clients and the server are secured with TLS.
See the MariaDB documentation for more information.

INSTALLATION

To install the plugin, run the following commands:

  make
  sudo make install

Add the following settings to your MariaDB configuration, for example by
creating the file /etc/my.cnf.d/auth_saslauthd.cnf:

  [mariadb]
  plugin-load=auth_saslauthd.so
  saslauthd_path=/var/state/saslauthd/mux
  saslauthd_service=mariadb

Sometimes the communications socket is named /var/run/saslauthd/mux.

On Slackware Linux, enable the saslauthd daemon and restart MariaDB with:

  sudo chmod +x /etc/rc.d/rc.saslauthd
  sudo /etc/rc.d/rc.saslauthd start
  sudo /etc/rc.d/rc.mysqld restart

Connect to MariaDB and check the database system for the plugin:

  SHOW PLUGINS\G

     Name: saslauthd
   Status: ACTIVE
     Type: AUTHENTICATION
  Library: auth_saslauthd.so
  License: BSD

DATABASE USERS

Add a database account for the system user "jennifer" as follows:

  CREATE USER 'jennifer'@'localhost' IDENTIFIED WITH saslauthd;

You can specify a different system user:

  CREATE USER 'jekyll'@'localhost' IDENTIFIED WITH saslauthd AS 'hyde';

And you can add a realm to the system user:

  CREATE USER 'david' IDENTIFIED WITH saslauthd AS 'david@EXAMPLE.COM';

In order to verify the new database account, run the mysql command-line tool
and enter the system user's password. Example:

  mysql -h localhost -u jennifer -p
  Enter password:

DEPENDENCIES

The plugin requires MariaDB and the saslauthd daemon from Cyrus SASL.

LICENSE AND COPYRIGHT

Copyright 2017 Andreas Voegele

This plugin is free software; you can redistribute it and/or modify it
under the terms of the ISC license.
