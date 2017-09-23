
/*data structure and their constructors for type checking and further phases*/

#ifndef SEMANTIC_H
#define SEMANTIC_H

struct expty {
	Tr_exp exp;
	Ty_ty ty;
};

struct expty expTy(Tr_exp exp, Ty_ty ty);


/* type checking */
F_fragList SEM_transProg(A_exp exp);

bool isEqual(Ty_ty, Ty_ty); 

void traverseRec(S_table tenv, Ty_fieldList *tFields, A_fieldList fields);

void traverseParams(S_table venv, S_table tenv, Ty_tyList *formals, A_fieldList pams);

struct expty transVar(S_table venv, S_table tenv, A_var v, Tr_level lv);

struct expty transExp(S_table venv, S_table tenv, A_exp a, Tr_level level, Temp_label label);

Tr_exp transDec(S_table venv, S_table tenv, A_dec d, Tr_level level, Temp_label label);

Ty_ty transTy(S_table tenv, A_ty a);

#endif
