PLUGIN=		auth_saslauthd.so
CNF=		auth_saslauthd.cnf
HDRS=		saslauthd_client.h
SRCS=		saslauthd_client.c auth_saslauthd.c

SASLAUTHD_PATH=	/var/state/saslauthd/mux
SASLAUTHD_SERVICE=mariadb

CNFDIR=		/etc/my.cnf.d
PLUGINDIR=	`mysql_config --plugindir`

DEFS=		-DMYSQL_DYNAMIC_PLUGIN \
		-DSASLAUTHD_PATH='"${SASLAUTHD_PATH}"' \
		-DSASLAUTHD_SERVICE='"${SASLAUTHD_SERVICE}"'

MYSQLFLAGS=	`mysql_config --cflags`

PICFLAG+=	-fPIC
CFLAGS+=	${OPTIMIZE} ${PICFLAG} ${MYSQLFLAGS} ${DEFS}
SOFLAGS+=	-shared

OBJS=		${SRCS:.c=.o}

INSTALL=	install

all: ${PLUGIN}

${PLUGIN}: ${OBJS}
	${CC} ${SOFLAGS} -o ${PLUGIN} ${OBJS}

${OBJS}: ${SRCS} ${HDRS}

install:
	${INSTALL} -D -m 0755 ${PLUGIN} ${DESTDIR}${PLUGINDIR}/${PLUGIN}

clean:
	rm -f ${PLUGIN} ${OBJS}

.PHONY: all install clean
