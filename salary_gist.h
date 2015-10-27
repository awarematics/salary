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


typedef bytea GBT_VARKEY;

/* Better readable key */
typedef struct
{
	bytea	   *lower,
			   *upper;
} GBT_VARKEY_R;


/*
 * type description
 */
typedef struct
{

	/* Attribs */

	enum gbtree_type t;			/* data type */
	int32		eml;			/* cached pg_database_encoding_max_length (0:
								 * undefined) */
	bool		trnc;			/* truncate (=compress) key */

	/* Methods */

	bool		(*f_gt) (const void *, const void *, Oid);		/* greater than */
	bool		(*f_ge) (const void *, const void *, Oid);		/* greater equal */
	bool		(*f_eq) (const void *, const void *, Oid);		/* equal */
	bool		(*f_le) (const void *, const void *, Oid);		/* less equal */
	bool		(*f_lt) (const void *, const void *, Oid);		/* less than */
	int32		(*f_cmp) (const void *, const void *, Oid);		/* compare */
	GBT_VARKEY *(*f_l2n) (GBT_VARKEY *);		/* convert leaf to node */
} gbtree_vinfo;

typedef struct
{
	int			i;
	GBT_VARKEY *t;
} Vsrt;

typedef struct
{
	const gbtree_vinfo *tinfo;
	Oid			collation;
} gbt_vsrt_arg;





