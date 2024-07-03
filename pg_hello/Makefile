# contrib/pg_hello/Makefile

MODULE_big = pg_hello
OBJS = \
	$(WIN32RES) \
	hello.o

EXTENSION = pg_hello
DATA = pg_hello--1.0.sql pg_hello--1.0--1.1.sql

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_hello
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
