/*
 * contrib/btree_gist/btree_gist.h
 */

#include "fmgr.h"
#include "access/nbtree.h"

#define BtreeGistNotEqualStrategyNumber 6

/* indexed types */

enum gbtree_type
{
	gbt_t_var,
	gbt_t_int2,
	gbt_t_int4,
	gbt_t_int8,
	gbt_t_float4,
	gbt_t_float8,
	gbt_t_numeric,
	gbt_t_ts,
	gbt_t_cash,
	gbt_t_oid,
	gbt_t_time,
	gbt_t_date,
	gbt_t_intv,
	gbt_t_macad,
	gbt_t_text,
	gbt_t_bpchar,
	gbt_t_bytea,
	gbt_t_bit,
	gbt_t_inet
};



/*
 * Generic btree functions
 */

Datum		gbtreekey_in(PG_FUNCTION_ARGS);

Datum		gbtreekey_out(PG_FUNCTION_ARGS);


typedef char GBT_NUMKEY;

/* Better readable key */
typedef struct
{
	const GBT_NUMKEY *lower,
			   *upper;
} GBT_NUMKEY_R;


/* for sorting */
typedef struct
{
	int			i;
	GBT_NUMKEY *t;
} Nsrt;


/* type description */

typedef struct
{

	/* Attribs */

	enum gbtree_type t;			/* data type */
	int32		size;			/* size of type, 0 means variable */
	int32		indexsize;		/* size of datums stored in index */

	/* Methods */

	bool		(*f_gt) (const void *, const void *);	/* greater than */
	bool		(*f_ge) (const void *, const void *);	/* greater or equal */
	bool		(*f_eq) (const void *, const void *);	/* equal */
	bool		(*f_le) (const void *, const void *);	/* less or equal */
	bool		(*f_lt) (const void *, const void *);	/* less than */
	int			(*f_cmp) (const void *, const void *);	/* key compare function */
	//float8		(*f_dist) (const void *, const void *); /* key distance function */
} gbtree_ninfo;



/*
 * Note: The factor 0.49 in following macro avoids floating point overflows
 */
#define penalty_num(result,olower,oupper,nlower,nupper) do { \
  double	tmp = 0.0F; \
  (*(result))	= 0.0F; \
  if ( (nupper) > (oupper) ) \
	  tmp += ( ((double)nupper)*0.49F - ((double)oupper)*0.49F ); \
  if (	(olower) > (nlower)  ) \
	  tmp += ( ((double)olower)*0.49F - ((double)nlower)*0.49F ); \
  if (tmp > 0.0F) \
  { \
	(*(result)) += FLT_MIN; \
	(*(result)) += (float) ( ((double)(tmp)) / ( (double)(tmp) + ( ((double)(oupper))*0.49F - ((double)(olower))*0.49F ) ) ); \
	(*(result)) *= (FLT_MAX / (((GISTENTRY *) PG_GETARG_POINTER(0))->rel->rd_att->natts + 1)); \
  } \
} while (0);

