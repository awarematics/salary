
CREATE OR REPLACE FUNCTION g_salary_consistent(internal,int4,int,oid,internal)
RETURNS bool
AS '$libdir/salary_gist','g_salary_consistent'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_compress(internal)
RETURNS internal
AS '$libdir/salary_gist','g_salary_compress'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_decompress(internal)
RETURNS internal
AS '$libdir/salary_gist','g_salary_decompress'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_penalty(internal,internal,internal)
RETURNS internal
AS '$libdir/salary_gist','g_salary_penalty'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_picksplit(internal, internal)
RETURNS internal
AS '$libdir/salary_gist','g_salary_picksplit'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_union(internal, internal)
RETURNS internal
AS '$libdir/salary_gist','g_salary_union'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION g_salary_same(internal, internal, internal)
RETURNS internal
AS '$libdir/salary_gist','g_salary_same'
LANGUAGE C IMMUTABLE STRICT;

