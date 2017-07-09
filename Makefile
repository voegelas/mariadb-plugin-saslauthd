PLUGIN=		auth_saslauthd.so
CNF=		auth_saslauthd.cnf
HDRS=		saslauthd_client.h
SRCS=		saslauthd_client.c auth_saslauthd.c

SASLAUTHD_PATH=	/var/state/saslauthd/mux
SASLAUTHD_SERVICE=mariadb

PLUGINDIR!=	mysql_config --plugindir
MYLIBDIR!=	mysql_config --variable=pkglibdir
MYFLAGS!=	mysql_config --cflags

DEFS=		-DMYSQL_DYNAMIC_PLUGIN \
		-DSASLAUTHD_PATH='"${SASLAUTHD_PATH}"' \
		-DSASLAUTHD_SERVICE='"${SASLAUTHD_SERVICE}"'

PICFLAGS+=	-fPIC
CFLAGS+=	${OPTIMIZE} ${PICFLAGS} ${MYFLAGS} ${DEFS}
SOFLAGS+=	-shared
SOLIBS+=	-lpthread -L${MYLIBDIR} -lmysqlservices

OBJS=		${SRCS:.c=.o}

INSTALL=	install

all: ${PLUGIN}

${PLUGIN}: ${OBJS}
	${CC} ${SOFLAGS} -o ${PLUGIN} ${OBJS} ${SOLIBS}

${OBJS}: ${SRCS} ${HDRS}

install: ${PLUGIN}
	${INSTALL} -D -m 0755 ${PLUGIN} ${DESTDIR}${PLUGINDIR}/${PLUGIN}

clean:
	rm -f ${PLUGIN} ${OBJS}

.PHONY: all install clean
