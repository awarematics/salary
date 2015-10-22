CREATE TYPE tpoint as (
	point	geometry,
	ts	timestamp with time zone);

CREATE TYPE trajectory as (
	traj	tpoint[]);

CREATE TABLE g_salary(id int, traj trajectory);

CREATE OR REPLACE FUNCTION g_salary_consistent(internal,int4,int,oid,internal)
RETURNS bool
AS '/usr/local/pgsql/share/contrib/salary_gist/lib/salary_gist','g_salary_consistent'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_compress(internal)
RETURNS internal
AS '/usr/local/pgsql/share/contrib/salary_gist/lib/salary_gist','g_salary_compress'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_decompress(internal)
RETURNS internal
AS '/usr/local/pgsql/share/contrib/salary_gist/lib/salary_gist','g_salary_decompress'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_penalty(internal,internal,internal)
RETURNS internal
AS '/usr/local/pgsql/share/contrib/salary_gist/lib/salary_gist','g_salary_penalty'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_picksplit(internal, internal)
RETURNS internal
AS '/usr/local/pgsql/share/contrib/salary_gist/lib/salary_gist','g_salary_picksplit'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_union(internal, internal)
RETURNS internal
AS '/usr/local/pgsql/share/contrib/salary_gist/lib/salary_gist','g_salary_union'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_same(internal, internal, internal)
RETURNS internal
AS '/usr/local/pgsql/share/contrib/salary_gist/lib/salary_gist','g_salary_same'
LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR CLASS g_salary_ops
DEFAULT FOR TYPE trajectory USING gist
AS
	OPERATOR	1	<  ,
	OPERATOR	2	<= ,
	OPERATOR	3	=  ,
	OPERATOR	4	>= ,
	OPERATOR	5	>  ,
	FUNCTION	1	g_salary_consistent (internal, int4, int, oid, internal),
	FUNCTION	2	g_salary_union (internal, internal),
	FUNCTION	3	g_salary_compress (internal),
	FUNCTION	4	g_salary_decompress (internal),
	FUNCTION	5	g_salary_penalty (internal, internal, internal),
	FUNCTION	6	g_salary_picksplit (internal, internal),
	FUNCTION	7	g_salary_same (internal, internal, internal);
