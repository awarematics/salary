# **********************************************************************
# * $Id$
# *
# * PostGIS - Spatial Types for PostgreSQL
# * http://postgis.net
# * Copyright 2008 Mark Cave-Ayland
# *
# * This is free software; you can redistribute and/or modify it under
# * the terms of the GNU General Public Licence. See the COPYING file.
# *
# **********************************************************************

POSTGIS_PGSQL_VERSION=93
MODULE_big=salary_gist
MODULEDIR=share/contrib/$(MODULE_big)

psql = /usr/local/pgsql/bin/psql

pgsql = /usr/local/pgsql
libdir = $(MODULEDIR)/lib
includedir = /usr/local/pgsql/include

includedir_server = $(includedir)/server

CC=gcc

# PostTrajectory objects
PG_OBJS = salary_gist.o
PG_SO = salary_gist.so
PG_SQL = salary_gist.sql
PG_SQL_uninstall =  salary_gist_uninstall.sql


all:
	$(CC) -fpic -c salary_gist.c -I$(includedir_server)
	$(CC) -shared -o $(PG_SO) $(PG_OBJS)

clean:
	rm -f $(PG_OBJS)
	rm -f $(PG_SO)

install: installdirs
	cp $(PG_SO) $(pgsql)/$(libdir)
	cp $(PG_SQL) $(pgsql)/$(MODULEDIR)
	cp $(PG_SQL_uninstall) $(pgsql)/$(MODULEDIR)
	$(psql) -U postgres postgres < /usr/local/pgsql/share/contrib/salary_gist/salary_gist.sql

installdirs:
	mkdir $(pgsql)/$(MODULEDIR)
	mkdir $(pgsql)/$(libdir)

uninstall: 
#	rm -f $(libdir)/$(PG_SO)
	$(psql) -U postgres postgres < /usr/local/pgsql/share/contrib/salary_gist/salary_gist_uninstall.sql
	rm -rf $(pgsql)/$(MODULEDIR)
	
