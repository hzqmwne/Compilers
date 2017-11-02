/*Lab4: Your implementation of lab4*/

#ifndef __ENV_H_
#define __ENV_H_

#include "types.h"

typedef struct E_enventry_ *E_enventry;

struct E_enventry_ {
	enum {E_varEntry, E_funEntry} kind;
	int readonly; //for loop var
	union 
	{
		struct {Ty_ty ty;} var;
		struct {Ty_tyList formals; Ty_ty result;} fun;
	} u;
};

E_enventry E_VarEntry(Ty_ty ty);
E_enventry E_ROVarEntry(Ty_ty ty);
E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result);

S_table E_base_tenv(void);
S_table E_base_venv(void);

#endif
