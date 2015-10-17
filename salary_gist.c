/* 
**  수정 테스트
*/

#include "postgres.h"

#include "access/gist.h"
#include "access/skey.h"

#include "salary_gist.h"


typedef struct int32key
{
	int32		lower;
	int32		upper;
} int32KEY;


GISTENTRY *
gbt_num_compress(GISTENTRY *retval, GISTENTRY *entry, const gbtree_ninfo *tinfo);
bool
gbt_num_consistent(const GBT_NUMKEY_R *key,
				   const void *query,
				   const StrategyNumber *strategy,
				   bool is_leaf,
				   const gbtree_ninfo *tinfo);
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

static const gbtree_ninfo tinfo =
{
	gbt_t_int4,
	sizeof(int32),
	8,							/* sizeof(gbtreekey8) */
	gbt_int4gt,
	gbt_int4ge,
	gbt_int4eq,
	gbt_int4le,
	gbt_int4lt,
	gbt_int4key_cmp
};



GISTENTRY *
gbt_num_compress(GISTENTRY *retval, GISTENTRY *entry, const gbtree_ninfo *tinfo)
{
	if (entry->leafkey)
	{
		union
		{
			int16		i2;
			int32		i4;
			int64		i8;
			float4		f4;
			float8		f8;
			DateADT		dt;
			TimeADT		tm;
			Timestamp	ts;
			Cash		ch;
		}			v;

		GBT_NUMKEY *r = (GBT_NUMKEY *) palloc0(tinfo->indexsize);
		void	   *leaf = NULL;

		switch (tinfo->t)
		{
			case gbt_t_int2:
				v.i2 = DatumGetInt16(entry->key);
				leaf = &v.i2;
				break;
			case gbt_t_int4:
				v.i4 = DatumGetInt32(entry->key);
				leaf = &v.i4;
				break;
			case gbt_t_int8:
				v.i8 = DatumGetInt64(entry->key);
				leaf = &v.i8;
				break;
			case gbt_t_oid:
				v.i4 = DatumGetObjectId(entry->key);
				leaf = &v.i4;
				break;
			case gbt_t_float4:
				v.f4 = DatumGetFloat4(entry->key);
				leaf = &v.f4;
				break;
			case gbt_t_float8:
				v.f8 = DatumGetFloat8(entry->key);
				leaf = &v.f8;
				break;
			case gbt_t_date:
				v.dt = DatumGetDateADT(entry->key);
				leaf = &v.dt;
				break;
			case gbt_t_time:
				v.tm = DatumGetTimeADT(entry->key);
				leaf = &v.tm;
				break;
			case gbt_t_ts:
				v.ts = DatumGetTimestamp(entry->key);
				leaf = &v.ts;
				break;
			case gbt_t_cash:
				v.ch = DatumGetCash(entry->key);
				leaf = &v.ch;
				break;
			default:
				leaf = DatumGetPointer(entry->key);
		}

		Assert(tinfo->indexsize >= 2 * tinfo->size);

		memcpy((void *) &r[0], leaf, tinfo->size);
		memcpy((void *) &r[tinfo->size], leaf, tinfo->size);
		retval = palloc(sizeof(GISTENTRY));
		gistentryinit(*retval, PointerGetDatum(r), entry->rel, entry->page,
					  entry->offset, FALSE);
	}
	else
		retval = entry;

	return retval;
}

/*
 * The GiST consistent method
 *
 * Note: we currently assume that no datatypes that use this routine are
 * collation-aware; so we don't bother passing collation through.
 */
bool
gbt_num_consistent(const GBT_NUMKEY_R *key,
				   const void *query,
				   const StrategyNumber *strategy,
				   bool is_leaf,
				   const gbtree_ninfo *tinfo)
{
	bool		retval;

	switch (*strategy)
	{
		case BTLessEqualStrategyNumber:
			retval = (*tinfo->f_ge) (query, key->lower);
			break;
		case BTLessStrategyNumber:
			if (is_leaf)
				retval = (*tinfo->f_gt) (query, key->lower);
			else
				retval = (*tinfo->f_ge) (query, key->lower);
			break;
		case BTEqualStrategyNumber:
			if (is_leaf)
				retval = (*tinfo->f_eq) (query, key->lower);
			else
				retval = ((*tinfo->f_le) (key->lower, query) && (*tinfo->f_le) (query, key->upper)) ? true : false;
			break;
		case BTGreaterStrategyNumber:
			if (is_leaf)
				retval = (*tinfo->f_lt) (query, key->upper);
			else
				retval = (*tinfo->f_le) (query, key->upper);
			break;
		case BTGreaterEqualStrategyNumber:
			retval = (*tinfo->f_le) (query, key->upper);
			break;
		case BtreeGistNotEqualStrategyNumber:
			retval = (!((*tinfo->f_eq) (query, key->lower) &&
						(*tinfo->f_eq) (query, key->upper))) ? true : false;
			break;
		default:
			retval = false;
	}

	return (retval);
}


Datum
g_salary_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	int32		query = PG_GETARG_INT32(1);
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	
	/* Oid		subtype = PG_GETARG_OID(3); */
	bool	   *recheck = (bool *) PG_GETARG_POINTER(4);
	int32KEY   *kkk = (int32KEY *) DatumGetPointer(entry->key);
	GBT_NUMKEY_R key;

	/* All cases served by this function are exact */
	*recheck = false;
	
	key.lower = (GBT_NUMKEY *) &kkk->lower;
	key.upper = (GBT_NUMKEY *) &kkk->upper;

	PG_RETURN_BOOL(
				   gbt_num_consistent(&key, (void *) &query, &strategy, GIST_LEAF(entry), &tinfo)
		);	
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
	GISTENTRY  *retval = NULL;

	PG_RETURN_POINTER(gbt_num_compress(retval, entry, &tinfo));
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
	int32KEY   *origentry = (int32KEY *) DatumGetPointer(((GISTENTRY *) PG_GETARG_POINTER(0))->key);
	int32KEY   *newentry = (int32KEY *) DatumGetPointer(((GISTENTRY *) PG_GETARG_POINTER(1))->key);
	float	   *result = (float *) PG_GETARG_POINTER(2);

	penalty_num(result, origentry->lower, origentry->upper, newentry->lower, newentry->upper);
	
				
	PG_RETURN_POINTER(result);
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
	
	GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
	
				
	PG_RETURN_POINTER(v);
}
