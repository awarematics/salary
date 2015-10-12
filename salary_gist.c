/* 
**  수정 테스트
*/

#include "postgres.h"

#include "access/gist.h"
#include "access/skey.h"

Datum
g_salary_consistent(PG_FUNCTION_ARGS);
Datum
g_salary_union(PG_FUNCTION_ARGS);
Datum
g_salary_compress(PG_FUNCTION_ARGS);
Datum
g_salary_decompress(PG_FUNCTION_ARGS);
Datum
g_salary_penalty(PG_FUNCTION_ARGS);
Datum
g_salary_same(PG_FUNCTION_ARGS);
Datum
g_salary_picksplit(PG_FUNCTION_ARGS);


/*
** GiST support methods
*/
PG_FUNCTION_INFO_V1(g_salary_consistent);
PG_FUNCTION_INFO_V1(g_salary_compress);
PG_FUNCTION_INFO_V1(g_salary_decompress);
PG_FUNCTION_INFO_V1(g_salary_penalty);
PG_FUNCTION_INFO_V1(g_salary_picksplit);
PG_FUNCTION_INFO_V1(g_salary_union);
PG_FUNCTION_INFO_V1(g_salary_same);


/* define for comparison */

static bool
gbt_int4gt(const void *a, const void *b)
{
	return (*((const int32 *) a) > *((const int32 *) b));
}
static bool
gbt_int4ge(const void *a, const void *b)
{
	return (*((const int32 *) a) >= *((const int32 *) b));
}
static bool
gbt_int4eq(const void *a, const void *b)
{
	return (*((const int32 *) a) == *((const int32 *) b));
}
static bool
gbt_int4le(const void *a, const void *b)
{
	return (*((const int32 *) a) <= *((const int32 *) b));
}
static bool
gbt_int4lt(const void *a, const void *b)
{
	return (*((const int32 *) a) < *((const int32 *) b));
}

static int
gbt_int4key_cmp(const void *a, const void *b)
{
	int32KEY   *ia = (int32KEY *) (((const Nsrt *) a)->t);
	int32KEY   *ib = (int32KEY *) (((const Nsrt *) b)->t);

	if (ia->lower == ib->lower)
	{
		if (ia->upper == ib->upper)
			return 0;

		return (ia->upper > ib->upper) ? 1 : -1;
	}

	return (ia->lower > ib->lower) ? 1 : -1;
}


Datum
g_salary_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	void  *query = PG_GETARG_ARRAYTYPE_P_COPY(1);
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	bool		retval;

	/* Oid		subtype = PG_GETARG_OID(3); */
	bool	   *recheck = (bool *) PG_GETARG_POINTER(4);
	/* We set recheck to false to avoid repeatedly pulling every "possibly matched" geometry
	   out during index scans. For cases when the geometries are large, rechecking
	   can make things twice as slow. */
	*recheck = false;
		
	/* Quick sanity check on query argument. */
	if ( DatumGetPointer(PG_GETARG_DATUM(1)) == NULL )
	{		
		PG_RETURN_BOOL(FALSE); /* NULL query! This is screwy! */
	}
	
	/* Quick sanity check on entry key. */
	if ( DatumGetPointer(entry->key) == NULL )
	{	
		PG_RETURN_BOOL(FALSE); /* NULL entry! */
	}
	
	/* Treat leaf node tests different from internal nodes */
	if (GIST_LEAF(entry))
	{
		retval = g_salary_consistent_leaf(
		             (BOX2DF*)DatumGetPointer(entry->key),
		             &query_gbox_index, strategy);
	}
	else
	{
		retval = g_salary_consistent_internal(
		             (BOX2DF*)DatumGetPointer(entry->key),
		             &query_gbox_index, strategy);
	}
	
	PG_RETURN_BOOL(retval);
}


Datum
g_salary_union(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	int		   *size = (int *) PG_GETARG_POINTER(1);
	int	numranges, i;
	
	numranges = entryvec->n;
	
	for ( i = 1; i < numranges; i++ )
	{
		
	}
	
	PG_RETURN_POINTER(PG_GETARG_POINTER(0));
}


Datum
g_salary_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *retval;
	
	if (entry->leafkey)
	{
		GBT_VARKEY *r = NULL;
		bytea	   *leaf = (bytea *) DatumGetPointer(PG_DETOAST_DATUM(entry->key));
		GBT_VARKEY_R u;

		u.lower = u.upper = leaf;
		r = gbt_var_key_copy(&u, FALSE);

		retval = palloc(sizeof(GISTENTRY));
		gistentryinit(*retval, PointerGetDatum(r),
					  entry->rel, entry->page,
					  entry->offset, TRUE);
	}
	else
		retval = entry;
				
	PG_RETURN_POINTER(retval);
}

Datum
g_salary_decompress(PG_FUNCTION_ARGS)
{
	// decompress 불필요
	PG_RETURN_POINTER(PG_GETARG_POINTER(0));
}


Datum
g_salary_penalty(PG_FUNCTION_ARGS)
{
	GISTENTRY  *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
	float	   *result = (float *) PG_GETARG_POINTER(2);
	
	
				
	PG_RETURN_POINTER(result);
}


Datum
g_salary_same(PG_FUNCTION_ARGS)
{
	Datum		d1 = PG_GETARG_DATUM(0);
	Datum		d2 = PG_GETARG_DATUM(1);
	bool	   *result = (bool *) PG_GETARG_POINTER(2);

	//TODO 비교 함수
	
	PG_RETURN_POINTER(result);
}


Datum
g_salary_picksplit(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
	OffsetNumber i,
				maxoff;
				
	maxoff = entryvec->n - 1;
				
	PG_RETURN_POINTER(v);
}
