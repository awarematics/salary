/* 
**  수정 테스트
*/

#include "postgres.h"

#include "access/gist.h"
#include "access/skey.h"

#include "salary_gist.h"
#include "./geohash/geohash.h"


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
				   float *
gbt_var_penalty(float *res, const GISTENTRY *o, const GISTENTRY *n,
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
	return DatumGetBool(DirectFunctionCall2Coll(text_gt,
												collation,
												PointerGetDatum(a),
												PointerGetDatum(b)));
}

static bool
gbt_textge(const void *a, const void *b, Oid collation)
{
	return DatumGetBool(DirectFunctionCall2Coll(text_ge,
												collation,
												PointerGetDatum(a),
												PointerGetDatum(b)));
}

static bool
gbt_texteq(const void *a, const void *b, Oid collation)
{
	return DatumGetBool(DirectFunctionCall2Coll(texteq,
												collation,
												PointerGetDatum(a),
												PointerGetDatum(b)));
}

static bool
gbt_textle(const void *a, const void *b, Oid collation)
{
	return DatumGetBool(DirectFunctionCall2Coll(text_le,
												collation,
												PointerGetDatum(a),
												PointerGetDatum(b)));
}

static bool
gbt_textlt(const void *a, const void *b, Oid collation)
{
	return DatumGetBool(DirectFunctionCall2Coll(text_lt,
												collation,
												PointerGetDatum(a),
												PointerGetDatum(b)));
}

static int32
gbt_textcmp(const void *a, const void *b, Oid collation)
{
	return DatumGetInt32(DirectFunctionCall2Coll(bttextcmp,
												 collation,
												 PointerGetDatum(a),
												 PointerGetDatum(b)));
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

float *
gbt_var_penalty(float *res, const GISTENTRY *o, const GISTENTRY *n,
				Oid collation, const gbtree_vinfo *tinfo)
{
	GBT_VARKEY *orge = (GBT_VARKEY *) DatumGetPointer(o->key);
	GBT_VARKEY *newe = (GBT_VARKEY *) DatumGetPointer(n->key);
	GBT_VARKEY_R ok,
				nk;

	*res = 0.0;

	nk = gbt_var_key_readable(newe);
	if (nk.lower == nk.upper)	/* leaf */
	{
		GBT_VARKEY *tmp;

		tmp = gbt_var_leaf2node(newe, tinfo);
		if (tmp != newe)
			nk = gbt_var_key_readable(tmp);
	}
	ok = gbt_var_key_readable(orge);

	if ((VARSIZE(ok.lower) - VARHDRSZ) == 0 && (VARSIZE(ok.upper) - VARHDRSZ) == 0)
		*res = 0.0;
	else if (!(((*tinfo->f_cmp) (nk.lower, ok.lower, collation) >= 0 ||
				gbt_bytea_pf_match(ok.lower, nk.lower, tinfo)) &&
			   ((*tinfo->f_cmp) (nk.upper, ok.upper, collation) <= 0 ||
				gbt_bytea_pf_match(ok.upper, nk.upper, tinfo))))
	{
		Datum		d = PointerGetDatum(0);
		double		dres;
		int32		ol,
					ul;

		gbt_var_bin_union(&d, orge, collation, tinfo);
		ol = gbt_var_node_cp_len((GBT_VARKEY *) DatumGetPointer(d), tinfo);
		gbt_var_bin_union(&d, newe, collation, tinfo);
		ul = gbt_var_node_cp_len((GBT_VARKEY *) DatumGetPointer(d), tinfo);

		if (ul < ol)
		{
			dres = (ol - ul);	/* reduction of common prefix len */
		}
		else
		{
			GBT_VARKEY_R uk = gbt_var_key_readable((GBT_VARKEY *) DatumGetPointer(d));
			unsigned char tmp[4];

			tmp[0] = (unsigned char) (((VARSIZE(ok.lower) - VARHDRSZ) <= ul) ? 0 : (VARDATA(ok.lower)[ul]));
			tmp[1] = (unsigned char) (((VARSIZE(uk.lower) - VARHDRSZ) <= ul) ? 0 : (VARDATA(uk.lower)[ul]));
			tmp[2] = (unsigned char) (((VARSIZE(ok.upper) - VARHDRSZ) <= ul) ? 0 : (VARDATA(ok.upper)[ul]));
			tmp[3] = (unsigned char) (((VARSIZE(uk.upper) - VARHDRSZ) <= ul) ? 0 : (VARDATA(uk.upper)[ul]));
			dres = Abs(tmp[0] - tmp[1]) + Abs(tmp[3] - tmp[2]);
			dres /= 256.0;
		}

		*res += FLT_MIN;
		*res += (float) (dres / ((double) (ol + 1)));
		*res *= (FLT_MAX / (o->rel->rd_att->natts + 1));
	}

	return res;
}

Datum
g_salary_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	void	   *query = (void *) DatumGetTextP(PG_GETARG_DATUM(1));	
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
	GISTENTRY  *o = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *n = (GISTENTRY *) PG_GETARG_POINTER(1);
	float	   *result = (float *) PG_GETARG_POINTER(2);

	gbt_var_penalty(result, o, n, PG_GET_COLLATION(),
									  &tinfo);	
				
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
