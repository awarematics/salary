

#include "fmgr.h"


extern int	text_cmp(text *arg1, text *arg2, Oid collid);
extern Datum texteq(PG_FUNCTION_ARGS);
extern Datum textne(PG_FUNCTION_ARGS);
extern Datum text_lt(PG_FUNCTION_ARGS);
extern Datum text_le(PG_FUNCTION_ARGS);
extern Datum text_gt(PG_FUNCTION_ARGS);
extern Datum text_ge(PG_FUNCTION_ARGS);
extern Datum bttextcmp(PG_FUNCTION_ARGS);


extern GBT_VARKEY_R gbt_var_key_readable(const GBT_VARKEY *k);

extern GBT_VARKEY *gbt_var_key_copy(const GBT_VARKEY_R *u, bool force_node);

extern GISTENTRY *gbt_var_compress(GISTENTRY *entry, const gbtree_vinfo *tinfo);

extern bool gbt_var_consistent(GBT_VARKEY_R *key, const void *query,
				   StrategyNumber strategy, Oid collation, bool is_leaf,
				   const gbtree_vinfo *tinfo);

extern float *gbt_var_penalty(float *res, const GISTENTRY *o, const GISTENTRY *n,
				Oid collation, const gbtree_vinfo *tinfo);

extern GIST_SPLITVEC *gbt_var_picksplit(const GistEntryVector *entryvec, GIST_SPLITVEC *v,
				  Oid collation, const gbtree_vinfo *tinfo);