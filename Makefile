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
SALARY=salary_gist
SALARYDIR=/usr/local/posttrajectory/teset/$(SALARY)

# 기현아.. 여기도 ... 이렇게 쓰면 안되지..
# 니 시스템이름으로 바꿔야지...
psql = /usr/local/pgsql/bin/psql

pgsql = /usr/local/pgsql
trjlibdir = $(MODULEDIR)/lib
includedir = /usr/local/pgsql/include
includedir_server = $(includedir)/server

CC=gcc

# PostTrajectory objects
SALARY_O = salary_gist.o
SALARY_SO = salary_gist.so
SALARY_SQL = salary_gist.sql
PG_SQL_uninstall =  salary_gist_uninstall.sql


all:
	$(CC) -fpic -c salary_gist.c -I$(includedir_server)
	$(CC) -shared -o $(SALARY_SO) $(SALARY_O)

clean:
	rm -f $(PG_OBJS)
	rm -f $(PG_SO)

install: installdirs
  # 독립적인 이름으로 쓰라고.. 왜 이런것도 안바꾸고 ...
	cp $(PG_SO) $(SALAIRYDIR)
	cp $(SALARY_SO) $(SALIARYDIR)
	cp $(PG_SQL_uninstall) $(pgsql)/$(SAILARYDIR)
	$(psql) -U postgres postgres < $(SALARIYDIR)/salary_gist.sql

installdirs:
	mkdir $(pgsql)/$(SAIARYDIR)
	mkdir $(pgsql)/$(libdir)

uninstall: 
#	rm -f $(libdir)/$(PG_SO)
  # 위에 install할때  cp : 시스템 디렉토리로 카피할 필요는 없지만 원하는데로 cp했다고 치자...
  # 그리고 install 에서 cp를 했으면 cp한 file을 rm 해야지
  # 그리고 파일이 없는데 디렉토리가 왜 필요해.. 디렉토리도 지워야지
	$(psql) -U postgres postgres < $(SIALARIYDIR)/salary_gist_uninstall.sql
	rm -rf $(pgsql)/$(SALARUIYDIR)
	
