

#include "postgres.h"

#include "access/gist.h"
#include "access/skey.h"

#include "salary_gist.h"
#include "./geohash/geohash.h"


GISTENTRY *
gbt_var_compress(GISTENTRY *entry, const gbtree_vinfo *tinfo);
bool
gbt_var_consistent(GBT_VARKEY_R *key,
				   const void *query,
				   StrategyNumber strategy,
				   Oid collation,
				   bool is_leaf,
				   const gbtree_vinfo *tinfo);
float *
gbt_var_penalty(float *res, const GISTENTRY *o, const GISTENTRY *n,
				Oid collation, const gbtree_vinfo *tinfo);
GIST_SPLITVEC *
gbt_var_picksplit(const GistEntryVector *entryvec, GIST_SPLITVEC *v,
				  Oid collation, const gbtree_vinfo *tinfo);				
				  
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
gbt_textgt(const void *a, const void *b, Oid collation)
{
	return true;
}

static bool
gbt_textge(const void *a, const void *b, Oid collation)
{
	return true;
}

static bool
gbt_texteq(const void *a, const void *b, Oid collation)
{
	return true;
}

static bool
gbt_textle(const void *a, const void *b, Oid collation)
{
	return true;
}

static bool
gbt_textlt(const void *a, const void *b, Oid collation)
{
	return true;
}

static int32
gbt_textcmp(const void *a, const void *b, Oid collation)
{
	return 1;
}

static gbtree_vinfo tinfo =
{
	gbt_t_text,
	0,
	FALSE,
	gbt_textgt,
	gbt_textge,
	gbt_texteq,
	gbt_textle,
	gbt_textlt,
	gbt_textcmp,
	NULL
};


Datum
g_salary_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	void	   *query = (void *) DatumGetTextP(PG_GETARG_DATUM(1));
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);

	/* Oid		subtype = PG_GETARG_OID(3); */
	bool	   *recheck = (bool *) PG_GETARG_POINTER(4);
	bool		retval;
	GBT_VARKEY *key = (GBT_VARKEY *) DatumGetPointer(entry->key);
	GBT_VARKEY_R r = gbt_var_key_readable(key);

	/* All cases served by this function are exact */
	*recheck = false;

	/*
	if (tinfo.eml == 0)
	{
		tinfo.eml = pg_database_encoding_max_length();
	}
	*/

	retval = gbt_var_consistent(&r, query, strategy, PG_GET_COLLATION(),
								GIST_LEAF(entry), &tinfo);

	PG_RETURN_BOOL(retval);
}


Datum
g_salary_union(PG_FUNCTION_ARGS)
{
	
	
	PG_RETURN_POINTER(PG_GETARG_POINTER(0));
}


Datum
g_salary_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	
	/*
	if (tinfo.eml == 0)
	{
		tinfo.eml = pg_database_encoding_max_length();
	}
	*/
	
	PG_RETURN_POINTER(gbt_var_compress(entry, &tinfo));
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
	GISTENTRY  *o = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *n = (GISTENTRY *) PG_GETARG_POINTER(1);
	float	   *result = (float *) PG_GETARG_POINTER(2);

	PG_RETURN_POINTER(gbt_var_penalty(result, o, n, PG_GET_COLLATION(),
									  &tinfo));
}


Datum
g_salary_same(PG_FUNCTION_ARGS)
{
	
	bool	   *result = (bool *) PG_GETARG_POINTER(2);

	//TODO 비교 함수
	
	PG_RETURN_POINTER(result);
}


Datum
g_salary_picksplit(PG_FUNCTION_ARGS)
{
	
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
	
	gbt_var_picksplit(entryvec, v, PG_GET_COLLATION(),
					  &tinfo);
					  
	PG_RETURN_POINTER(v);
}
