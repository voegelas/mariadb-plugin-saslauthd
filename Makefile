PLUGIN=		auth_saslauthd.so
CNF=		auth_saslauthd.cnf
HDRS=		saslauthd_client.h
SRCS=		saslauthd_client.c auth_saslauthd.c

CNFDIR=		/etc/my.cnf.d
PLUGINDIR:=	${shell mysql_config --plugindir}

DEFS=		-DMYSQL_DYNAMIC_PLUGIN

MYSQLFLAGS:=	${shell mysql_config --cflags}

OPTIMIZE+=	-O2
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
	${INSTALL} -D -m 0644 ${CNF} ${DESTDIR}${CNFDIR}/${CNF}

clean:
	rm -f ${PLUGIN} ${OBJS}

.PHONY: clean
