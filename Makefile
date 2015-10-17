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
MODULEDIR=contrib/$(MODULE_big)

# Files to be copied to the contrib/ directory
SQL_built=salary_gist.sql



#TODO
#-----------------------------------------------------------------------------------
# SQL objects (files requiring pre-processing)
SQL_objs=postgis.sql legacy.sql legacy_minimal.sql

GEOM_BACKEND_OBJ = lwgeom_geos.o
SFCGAL_BACKEND_OBJ = lwgeom_sfcgal.o

ifeq (,sfcgal)
DATA_built=$(SQL_built) sfcgal.sql
SQL_OBJS=$(SQL_objs) sfcgal.sql
BACKEND_OBJ=$(GEOM_BACKEND_OBJ) $(SFCGAL_BACKEND_OBJ)
else
BACKEND_OBJ=$(GEOM_BACKEND_OBJ)
DATA_built=$(SQL_built)
SQL_OBJS=$(SQL_objs)
endif
#-----------------------------------------------------------------------------------




# SQL preprocessor
SQLPP = /usr/bin/cpp -w -traditional-cpp -P

# PostTrajectory objects
PG_OBJS= salary_gist.o

# Objects to build using PGXS
OBJS=$(PG_OBJS)


# PGXS information
PG_CONFIG = /usr/local/pgsql/bin/pg_config 
PGXS := /usr/local/pgsql/lib/pgxs/src/makefiles/pgxs.mk
include $(PGXS)

# Set PERL _after_ the include of PGXS
PERL=/usr/bin/perl

# Borrow the $libdir substitution from PGXS but customise by running the preprocessor
# and adding the version number
%.sql: %.sql.in
	$(SQLPP) -I../libpgcommon $< | grep -v '^#' | \
	$(PERL) -lpe "s'MODULE_PATHNAME'\$$libdir/salary_gist'g" > $@

