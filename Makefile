# **********************************************************************
# * 
# *
# * 
# * 
# * 
# *
# * 
# * 
# *
# **********************************************************************

POSTGIS_PGSQL_VERSION=93
SALARY=salary_gist
ROOTDIR=/usr/local/posttrajectory
SALARYDIR=$(ROOTDIR)/test/$(SALARY)

#GeoHash Library
GEOHASHDIR=./geohash

# PostgreSQL psql
PSQL = /usr/local/pgsql/bin/psql

# PostgreSQL Directory 
pgsql = /usr/local/pgsql
includedir = /usr/local/pgsql/include
includedir_server = $(includedir)/server

# .so Directory
trjlibdir = $(SALARYDIR)/lib

CC=gcc

# SAIARY Files
SALARY_O = salary_gist.o
SALARY_SO = salary_gist.so
SALARY_SQL = salary_gist.sql
SALARY_SQL_uninstall =  salary_gist_uninstall.sql


all: geohashLib
	$(CC) -fpic -c salary_gist.c -I$(includedir_server) -I$(GEOHASHDIR)
	$(CC) -shared -o $(SALARY_SO) $(SALARY_O)

geohashLib: 
	$(CC) -c $(GEOHASHDIR)/geohash.c -o $(GEOHASHDIR)/geohash.o 
	ar rcs $(GEOHASHDIR)/libgeohash.a $(GEOHASHDIR)/geohash.o
	rm $(GEOHASHDIR)/geohash.o

clean:
	rm -f $(SALARY_O)
	rm -f $(SALARY_SO)
	rm -f $(GEOHASHDIR)/libgeohash.a

install: installdirs 
	cp $(SALARY_SO) $(trjlibdir)
	cp $(SALARY_SQL) $(SALARYDIR)	
	cp $(SALARY_SQL_uninstall) $(SALARYDIR)
	$(PSQL) -U postgres postgres < $(SALARYDIR)/$(SALARY_SQL)

installdirs:
	mkdir -p $(SALARYDIR)
	mkdir $(trjlibdir)

uninstall:   
	$(PSQL) -U postgres postgres < $(SALARYDIR)/$(SALARY_SQL_uninstall)
	rm -rf $(SALARYDIR)
	
