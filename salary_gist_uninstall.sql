DROP TABLE g_salary;


DROP FUNCTION g_salary_compress(internal);

DROP FUNCTION g_salary_consistent(internal, integer, integer, oid, internal);

DROP FUNCTION g_salary_decompress(internal);

DROP FUNCTION g_salary_penalty(internal, internal, internal);

DROP FUNCTION g_salary_picksplit(internal, internal);

DROP FUNCTION g_salary_same(internal, internal, internal);

DROP FUNCTION g_salary_union(internal, internal);


DROP TYPE trajectory;

DROP TYPE tpoint;

DROP OPERATOR CLASS g_salary_ops using gist
