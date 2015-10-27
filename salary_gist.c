/* 
**  수정 테스트
*/

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

GBT_VARKEY_R
gbt_var_key_readable(const GBT_VARKEY *k)
{
	GBT_VARKEY_R r;

	r.lower = (bytea *) &(((char *) k)[VARHDRSZ]);
	if (VARSIZE(k) > (VARHDRSZ + (VARSIZE(r.lower))))
		r.upper = (bytea *) &(((char *) k)[VARHDRSZ + INTALIGN(VARSIZE(r.lower))]);
	else
		r.upper = r.lower;
	return r;
}


GBT_VARKEY *
gbt_var_key_copy(const GBT_VARKEY_R *u, bool force_node)
{
	GBT_VARKEY *r = NULL;
	int32		lowersize = VARSIZE(u->lower);
	int32		uppersize = VARSIZE(u->upper);

	if (u->lower == u->upper && !force_node)
	{							/* leaf key mode */
		r = (GBT_VARKEY *) palloc(lowersize + VARHDRSZ);
		memcpy(VARDATA(r), u->lower, lowersize);
		SET_VARSIZE(r, lowersize + VARHDRSZ);
	}
	else
	{							/* node key mode  */
		r = (GBT_VARKEY *) palloc0(INTALIGN(lowersize) + uppersize + VARHDRSZ);
		memcpy(VARDATA(r), u->lower, lowersize);
		memcpy(VARDATA(r) + INTALIGN(lowersize), u->upper, uppersize);
		SET_VARSIZE(r, INTALIGN(lowersize) + uppersize + VARHDRSZ);
	}
	return r;
}


GISTENTRY *
gbt_var_compress(GISTENTRY *entry, const gbtree_vinfo *tinfo)
{
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

	return (retval);
}


/*
 * The GiST consistent method
 *
 * Note: we currently assume that no datatypes that use this routine are
 * collation-aware; so we don't bother passing collation through.
 */
bool
gbt_var_consistent(GBT_VARKEY_R *key,
				   const void *query,
				   StrategyNumber strategy,
				   Oid collation,
				   bool is_leaf,
				   const gbtree_vinfo *tinfo)
{
	bool		retval = FALSE;

	switch (strategy)
	{
		case BTLessEqualStrategyNumber:
			if (is_leaf)
				retval = (*tinfo->f_ge) (query, key->lower, collation);
			else
				retval = (*tinfo->f_cmp) (query, key->lower, collation) >= 0
					|| gbt_var_node_pf_match(key, query, tinfo);
			break;
		case BTLessStrategyNumber:
			if (is_leaf)
				retval = (*tinfo->f_gt) (query, key->lower, collation);
			else
				retval = (*tinfo->f_cmp) (query, key->lower, collation) >= 0
					|| gbt_var_node_pf_match(key, query, tinfo);
			break;
		case BTEqualStrategyNumber:
			if (is_leaf)
				retval = (*tinfo->f_eq) (query, key->lower, collation);
			else
				retval =
					((*tinfo->f_cmp) (key->lower, query, collation) <= 0 &&
					 (*tinfo->f_cmp) (query, key->upper, collation) <= 0) ||
					gbt_var_node_pf_match(key, query, tinfo);
			break;
		case BTGreaterStrategyNumber:
			if (is_leaf)
				retval = (*tinfo->f_lt) (query, key->upper, collation);
			else
				retval = (*tinfo->f_cmp) (query, key->upper, collation) <= 0
					|| gbt_var_node_pf_match(key, query, tinfo);
			break;
		case BTGreaterEqualStrategyNumber:
			if (is_leaf)
				retval = (*tinfo->f_le) (query, key->upper, collation);
			else
				retval = (*tinfo->f_cmp) (query, key->upper, collation) <= 0
					|| gbt_var_node_pf_match(key, query, tinfo);
			break;
		case BtreeGistNotEqualStrategyNumber:
			retval = !((*tinfo->f_eq) (query, key->lower, collation) &&
					   (*tinfo->f_eq) (query, key->upper, collation));
			break;
		default:
			retval = FALSE;
	}

	return retval;
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
		//*res *= (FLT_MAX / (o->rel->rd_att->natts + 1));
	}

	return res;
}

GIST_SPLITVEC *
gbt_var_picksplit(const GistEntryVector *entryvec, GIST_SPLITVEC *v,
				  Oid collation, const gbtree_vinfo *tinfo)
{
	OffsetNumber i,
				maxoff = entryvec->n - 1;
	Vsrt	   *arr;
	int			svcntr = 0,
				nbytes;
	char	   *cur;
	GBT_VARKEY **sv = NULL;
	gbt_vsrt_arg varg;

	arr = (Vsrt *) palloc((maxoff + 1) * sizeof(Vsrt));
	nbytes = (maxoff + 2) * sizeof(OffsetNumber);
	v->spl_left = (OffsetNumber *) palloc(nbytes);
	v->spl_right = (OffsetNumber *) palloc(nbytes);
	v->spl_ldatum = PointerGetDatum(0);
	v->spl_rdatum = PointerGetDatum(0);
	v->spl_nleft = 0;
	v->spl_nright = 0;

	sv = palloc(sizeof(bytea *) * (maxoff + 1));

	/* Sort entries */

	for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
	{
		GBT_VARKEY_R ro;

		cur = (char *) DatumGetPointer(entryvec->vector[i].key);
		ro = gbt_var_key_readable((GBT_VARKEY *) cur);
		if (ro.lower == ro.upper)		/* leaf */
		{
			sv[svcntr] = gbt_var_leaf2node((GBT_VARKEY *) cur, tinfo);
			arr[i].t = sv[svcntr];
			if (sv[svcntr] != (GBT_VARKEY *) cur)
				svcntr++;
		}
		else
			arr[i].t = (GBT_VARKEY *) cur;
		arr[i].i = i;
	}

	/* sort */
	varg.tinfo = tinfo;
	varg.collation = collation;
	

	/* We do simply create two parts */

	for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
	{
		if (i <= (maxoff - FirstOffsetNumber + 1) / 2)
		{
			gbt_var_bin_union(&v->spl_ldatum, arr[i].t, collation, tinfo);
			v->spl_left[v->spl_nleft] = arr[i].i;
			v->spl_nleft++;
		}
		else
		{
			gbt_var_bin_union(&v->spl_rdatum, arr[i].t, collation, tinfo);
			v->spl_right[v->spl_nright] = arr[i].i;
			v->spl_nright++;
		}
	}

	/* Truncate (=compress) key */
	if (tinfo->trnc)
	{
		int32		ll = gbt_var_node_cp_len((GBT_VARKEY *) DatumGetPointer(v->spl_ldatum), tinfo);
		int32		lr = gbt_var_node_cp_len((GBT_VARKEY *) DatumGetPointer(v->spl_rdatum), tinfo);
		GBT_VARKEY *dl;
		GBT_VARKEY *dr;

		ll = Max(ll, lr);
		ll++;

		dl = gbt_var_node_truncate((GBT_VARKEY *) DatumGetPointer(v->spl_ldatum), ll, tinfo);
		dr = gbt_var_node_truncate((GBT_VARKEY *) DatumGetPointer(v->spl_rdatum), ll, tinfo);
		v->spl_ldatum = PointerGetDatum(dl);
		v->spl_rdatum = PointerGetDatum(dr);
	}

	return v;
}

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
