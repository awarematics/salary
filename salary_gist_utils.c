

#include "salary_gist.h"

/* text_cmp()
 * Internal comparison function for text strings.
 * Returns -1, 0 or 1
 */
static int
text_cmp(text *arg1, text *arg2, Oid collid)
{
	char	   *a1p,
			   *a2p;
	int			len1,
				len2;

	a1p = VARDATA_ANY(arg1);
	a2p = VARDATA_ANY(arg2);

	len1 = VARSIZE_ANY_EXHDR(arg1);
	len2 = VARSIZE_ANY_EXHDR(arg2);

	return varstr_cmp(a1p, len1, a2p, len2, collid);
}

/*
 * Comparison functions for text strings.
 *
 * Note: btree indexes need these routines not to leak memory; therefore,
 * be careful to free working copies of toasted datums.  Most places don't
 * need to be so careful.
 */

Datum
texteq(PG_FUNCTION_ARGS)
{
	Datum		arg1 = PG_GETARG_DATUM(0);
	Datum		arg2 = PG_GETARG_DATUM(1);
	bool		result;
	Size		len1,
				len2;

	/*
	 * Since we only care about equality or not-equality, we can avoid all the
	 * expense of strcoll() here, and just do bitwise comparison.  In fact, we
	 * don't even have to do a bitwise comparison if we can show the lengths
	 * of the strings are unequal; which might save us from having to detoast
	 * one or both values.
	 */
	len1 = toast_raw_datum_size(arg1);
	len2 = toast_raw_datum_size(arg2);
	if (len1 != len2)
		result = false;
	else
	{
		text	   *targ1 = DatumGetTextPP(arg1);
		text	   *targ2 = DatumGetTextPP(arg2);

		result = (memcmp(VARDATA_ANY(targ1), VARDATA_ANY(targ2),
						 len1 - VARHDRSZ) == 0);

		PG_FREE_IF_COPY(targ1, 0);
		PG_FREE_IF_COPY(targ2, 1);
	}

	PG_RETURN_BOOL(result);
}

Datum
textne(PG_FUNCTION_ARGS)
{
	Datum		arg1 = PG_GETARG_DATUM(0);
	Datum		arg2 = PG_GETARG_DATUM(1);
	bool		result;
	Size		len1,
				len2;

	/* See comment in texteq() */
	len1 = toast_raw_datum_size(arg1);
	len2 = toast_raw_datum_size(arg2);
	if (len1 != len2)
		result = true;
	else
	{
		text	   *targ1 = DatumGetTextPP(arg1);
		text	   *targ2 = DatumGetTextPP(arg2);

		result = (memcmp(VARDATA_ANY(targ1), VARDATA_ANY(targ2),
						 len1 - VARHDRSZ) != 0);

		PG_FREE_IF_COPY(targ1, 0);
		PG_FREE_IF_COPY(targ2, 1);
	}

	PG_RETURN_BOOL(result);
}

Datum
text_lt(PG_FUNCTION_ARGS)
{
	text	   *arg1 = PG_GETARG_TEXT_PP(0);
	text	   *arg2 = PG_GETARG_TEXT_PP(1);
	bool		result;

	result = (text_cmp(arg1, arg2, PG_GET_COLLATION()) < 0);

	PG_FREE_IF_COPY(arg1, 0);
	PG_FREE_IF_COPY(arg2, 1);

	PG_RETURN_BOOL(result);
}

Datum
text_le(PG_FUNCTION_ARGS)
{
	text	   *arg1 = PG_GETARG_TEXT_PP(0);
	text	   *arg2 = PG_GETARG_TEXT_PP(1);
	bool		result;

	result = (text_cmp(arg1, arg2, PG_GET_COLLATION()) <= 0);

	PG_FREE_IF_COPY(arg1, 0);
	PG_FREE_IF_COPY(arg2, 1);

	PG_RETURN_BOOL(result);
}

Datum
text_gt(PG_FUNCTION_ARGS)
{
	text	   *arg1 = PG_GETARG_TEXT_PP(0);
	text	   *arg2 = PG_GETARG_TEXT_PP(1);
	bool		result;

	result = (text_cmp(arg1, arg2, PG_GET_COLLATION()) > 0);

	PG_FREE_IF_COPY(arg1, 0);
	PG_FREE_IF_COPY(arg2, 1);

	PG_RETURN_BOOL(result);
}

Datum
text_ge(PG_FUNCTION_ARGS)
{
	text	   *arg1 = PG_GETARG_TEXT_PP(0);
	text	   *arg2 = PG_GETARG_TEXT_PP(1);
	bool		result;

	result = (text_cmp(arg1, arg2, PG_GET_COLLATION()) >= 0);

	PG_FREE_IF_COPY(arg1, 0);
	PG_FREE_IF_COPY(arg2, 1);

	PG_RETURN_BOOL(result);
}

Datum
bttextcmp(PG_FUNCTION_ARGS)
{
	text	   *arg1 = PG_GETARG_TEXT_PP(0);
	text	   *arg2 = PG_GETARG_TEXT_PP(1);
	int32		result;

	result = text_cmp(arg1, arg2, PG_GET_COLLATION());

	PG_FREE_IF_COPY(arg1, 0);
	PG_FREE_IF_COPY(arg2, 1);

	PG_RETURN_INT32(result);
}


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

text
gbt_var_MBRtoGeohash(const GBT_VARKEY_R *k)
{
	int precision = 12;		
	text minGeohash = NULL, maxGeohash = NULL;
	bool cmp = true;		
	
	do 
	{
		minGeohash = (text *) geohash_encode(k->xmin, k->ymin, precision);
		maxGeohash = (text *) geohash_encode(k->xmax k->ymax, precision);
	
		if (minGeohash = maxGeohash) 
		{
			cmp = true;	
		} else 
		{
			cmp = false;
		}
		
		precision--;
	} while (cmp);
		
	return minGeohash;
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

