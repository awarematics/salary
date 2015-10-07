
#include "postgres.h"

#include "access/gist.h"
#include "access/skey.h"


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


Datum
g_salary_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	ArrayType  *query = PG_GETARG_ARRAYTYPE_P_COPY(1);
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);

	/* Oid		subtype = PG_GETARG_OID(3); */
	bool	   *recheck = (bool *) PG_GETARG_POINTER(4);
	bool		retval;
	
	PG_RETURN_BOOL(retval);
}


Datum
g_salary_union(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	int		   *size = (int *) PG_GETARG_POINTER(1);
	int32		i,
			   *ptr;
	ArrayType  *res;
	int			totlen = 0;
	
	PG_RETURN_POINTER(res);
}


Datum
g_salary_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *retval;
	ArrayType  *r;
	int			len;
	int		   *dr;
	int			i,
				min,
				cand;
				
	PG_RETURN_POINTER(entry);
}

Datum
g_salary_decompress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *retval;
	ArrayType  *r;
	int		   *dr,
				lenr;
	ArrayType  *in;
	int			lenin;
	int		   *din;
	int			i,
				j;

	in = DatumGetArrayTypeP(entry->key);
	
	PG_RETURN_POINTER(retval);
}


Datum
g_salary_penalty(PG_FUNCTION_ARGS)
{
	GISTENTRY  *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
	float	   *result = (float *) PG_GETARG_POINTER(2);
	ArrayType  *ud;
	float		tmp1,
				tmp2;
				
	PG_RETURN_POINTER(result);
}


Datum
g_salary_same(PG_FUNCTION_ARGS)
{
	ArrayType  *a = PG_GETARG_ARRAYTYPE_P(0);
	ArrayType  *b = PG_GETARG_ARRAYTYPE_P(1);
	bool	   *result = (bool *) PG_GETARG_POINTER(2);
	int32		n = ARRNELEMS(a);
	int32	   *da,
			   *db;
			   
	PG_RETURN_POINTER(result);
}


Datum
g_salary_picksplit(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
	OffsetNumber i,
				j;
				
	PG_RETURN_POINTER(v);
}
