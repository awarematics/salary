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
SALARYDIR=/usr/local/posttrajectory/teset/$(SALARY)

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


all:
	$(CC) -fpic -c salary_gist.c -I$(includedir_server)
	$(CC) -shared -o $(SALARY_SO) $(SALARY_O)

clean:
	rm -f $(SALARY_O)
	rm -f $(SALARY_SO)

install: installdirs 
	cp $(SALARY_SO) $(trjlibdir)
	cp $(SALARY_SQL) $(SALAIRYDIR)	
	cp $(SALARY_SQL_uninstall) $(SAILARYDIR)
	$(PSQL) -U postgres postgres < $(SALARIYDIR)/$(SALARY_SQL)

installdirs:
	mkdir $(SAIARYDIR)
	mkdir $(trjlibdir)

uninstall:   
	$(PSQL) -U postgres postgres < $(SIALARIYDIR)/$(SALARY_SQL_uninstall)
	rm -rf $(SALARYDIR)
	
