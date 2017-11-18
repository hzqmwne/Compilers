#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "helper.h"
#include "env.h"
#include "semant.h"
#include <stdlib.h>

/*Lab4: Your implementation of lab4*/


typedef void* Tr_exp;
struct expty 
{
	Tr_exp exp; 
	Ty_ty ty;
};

//In Lab4, the first argument exp should always be **NULL**.
struct expty expTy(Tr_exp exp, Ty_ty ty)
{
	struct expty e;

	e.exp = exp;
	e.ty = ty;

	return e;
}


/* ========================================================= */

static Ty_ty actual_ty(Ty_ty ty) {
	if(ty == NULL) {
		return NULL;
	}
	if(ty->kind == Ty_name) {
		return actual_ty(ty->u.name.ty);
	}
	return ty;
}

static int equal_ty(Ty_ty ty1, Ty_ty ty2) {    // check if two type equal (especially for Ty_nil)
	ty1 = actual_ty(ty1);
	ty2 = actual_ty(ty2);
	if(ty1->kind != Ty_nil && ty2->kind != Ty_nil) {
		return (ty1 == ty2);
	}
	if(ty1->kind == Ty_nil && (ty2->kind == Ty_record || ty2->kind == Ty_array || ty2->kind == Ty_nil)) {
		return 1;
	}
	if(ty2->kind == Ty_nil && (ty1->kind == Ty_record || ty1->kind == Ty_array)) {
		return 1;
	}
	return 0;
}

static Ty_tyList makeFormalTyList(S_table tenv, A_fieldList params) {
	if(params == NULL) {
		return NULL;
	}
	Ty_ty t = S_look(tenv, params->head->typ);
	if(t == NULL) {
		EM_error(params->head->pos, "params type field type undeclared");
	}
	Ty_tyList rest =  makeFormalTyList(tenv, params->tail);
	return Ty_TyList(t, rest);
}

static Ty_fieldList makeFormalFieldList(S_table tenv, A_fieldList record) {
	if(record == NULL) {
		return NULL;
	}
	Ty_ty t = S_look(tenv, record->head->typ);
	if(t == NULL) {
		EM_error(record->head->pos, "undefined type treelist");
	}
	Ty_field now = Ty_Field(record->head->name, t);
	Ty_fieldList rest =  makeFormalFieldList(tenv, record->tail);
	return Ty_FieldList(now, rest);
}

/* == */

struct VisitedItem {
	Ty_ty ty;
	int legal;
	struct VisitedItem *next;
};
static struct VisitedItem *visitedItemHead = NULL;
static struct VisitedItem *getVisited(Ty_ty ty) {
	struct VisitedItem *tmp;
	for(tmp = visitedItemHead; tmp; tmp = tmp->next) {
		if(tmp->ty == ty) {
			return tmp;
		}
	}
	return NULL;
}
static struct VisitedItem *addVisitedItem(Ty_ty ty, int legal) {
	struct VisitedItem *new = (struct VisitedItem *)malloc(sizeof(struct VisitedItem));
	new->ty = ty;
	new->legal = legal;
	new->next = visitedItemHead;
	visitedItemHead = new;
}
static void freeVisitedItem(struct VisitedItem *item) {
	if(item == NULL) {
		return;
	}
	freeVisitedItem(item->next);
	free(item);
}
static int isLegalType(Ty_ty ty) {
	if(ty->kind != Ty_name) {
		return 1;
	}    // now ty->kind must be Ty_name
	struct VisitedItem *i = getVisited(ty);
	if(i != NULL) {
		return i->legal;
	}    // now must not be visited
	struct VisitedItem *new = addVisitedItem(ty, 0);
	new->legal = isLegalType(ty->u.name.ty);
	return new->legal;
}
static int checkRecursiveType(S_table tenv, A_nametyList nl) {    // 1 for correct, 0 for error
	for(; nl; nl = nl->tail) {
		Ty_ty t = S_look(tenv, nl->head->name);
		//assert(t->kind == Ty_name);
		if(t->kind == Ty_name && !isLegalType(t)) {
			freeVisitedItem(visitedItemHead);
			visitedItemHead = NULL;
			return 0;
		}
	}
	freeVisitedItem(visitedItemHead);
	visitedItemHead = NULL;
	return 1;
}

/* ======= */

struct expty transVar(S_table venv, S_table tenv, A_var v) {
	switch(v->kind) {
		case A_simpleVar: {
			E_enventry x = S_look(venv, v->u.simple);
			if(x && x->kind == E_varEntry) {
				return expTy(NULL, actual_ty(x->u.var.ty));
			}
			else {
				EM_error(v->pos, "undefined variable %s", S_name(v->u.simple));
				return expTy(NULL, Ty_Void());
			}
			break;
		}
		case A_fieldVar: {
			struct expty e = transVar(venv, tenv, v->u.field.var);
			if(e.ty->kind != Ty_record) {
				EM_error(v->pos, "not a record type");
				return expTy(NULL, Ty_Void());
			}
			Ty_fieldList f;
			for(f = e.ty->u.record; f; f = f->tail) {
				if(v->u.field.sym == f->head->name) {
					return expTy(NULL, actual_ty(f->head->ty));
				}
			}
			EM_error(v->pos, "field %s doesn't exist", S_name(v->u.field.sym));
			return expTy(NULL, Ty_Void());
			break;
		}
		case A_subscriptVar: {
			struct expty e1 = transVar(venv, tenv, v->u.subscript.var);
			struct expty e2 = transExp(venv, tenv, v->u.subscript.exp);
			if(e1.ty->kind != Ty_array) {
				EM_error(v->pos, "array type required");
				return expTy(NULL, Ty_Void());
			}
			if(e2.ty->kind != Ty_int) {
				EM_error(v->pos, "subscriptvar exp should be int");
				return expTy(NULL, Ty_Int());
			}
			return expTy(NULL, actual_ty(e1.ty->u.array));
			break;
		}
	}
	assert(0);
	//return expTy(NULL, NULL);
}

struct expty transExp(S_table venv, S_table tenv, A_exp a) {
	switch(a->kind) {
		case A_varExp: {
			return transVar(venv, tenv, a->u.var);
			break;
		}
		case A_nilExp: {
			return expTy(NULL, Ty_Nil());
			break;
		}
		case A_intExp: {
			return expTy(NULL, Ty_Int());
			break;
		}
		case A_stringExp: {
			return expTy(NULL, Ty_String());
			break;
		}
		case A_callExp: {
			E_enventry x = S_look(venv, a->u.call.func);
			if(x) {    //  x->kind == E_funEntry should not be checked, because E_enventry.kind not assigned !
				Ty_tyList t;
				A_expList e;
				for(t = x->u.fun.formals, e = a->u.call.args; e || t; t = t->tail, e = e->tail) {
					if(e != NULL && t == NULL) {
						EM_error(a->pos, "too many params in function %s", S_name(a->u.call.func));
						break;
					}
					else if(e == NULL && t != NULL) {
						EM_error(a->pos, "too few params in function %s", S_name(a->u.call.func));
						break;
					}
					else if(e == NULL || t == NULL || !equal_ty(transExp(venv, tenv, e->head).ty, actual_ty(t->head))) {
						EM_error(a->pos, "para type mismatch");
						break;
					}
				}
				return expTy(NULL, actual_ty(x->u.fun.result));
			}
			else {
				EM_error(a->pos, "undefined function %s", S_name(a->u.call.func));
				return expTy(NULL, Ty_Int());
			}
			break;
		}
		case A_opExp: {
			A_oper oper = a->u.op.oper;
			struct expty left = transExp(venv, tenv, a->u.op.left);
			struct expty right = transExp(venv, tenv, a->u.op.right);
			switch(oper) {
				case A_plusOp: case A_minusOp: case A_timesOp: case A_divideOp: {
					if(left.ty->kind != Ty_int) {
						EM_error(a->u.op.left->pos, "integer required");
					}
					if(right.ty->kind != Ty_int) {
						EM_error(a->u.op.right->pos, "integer required");
					}
					return expTy(NULL, Ty_Int());
					break;
				}
				case A_eqOp: case A_neqOp: case A_ltOp: case A_leOp: case A_gtOp: case A_geOp: {
					/*
					if(left.ty->kind == Ty_nil && (right.ty->kind == Ty_record || right.ty->kind == Ty_array)
					|| right.ty->kind == Ty_nil && (left.ty->kind == Ty_record || left.ty->kind == Ty_array)
					) {
						return expTy(NULL, Ty_Int());
					}
					else if(left.ty->kind != right.ty->kind) {
						EM_error(a->pos, "same type required");
					}
					*/
					Ty_ty lt = actual_ty(left.ty);
					Ty_ty rt = actual_ty(right.ty);
					int lk = lt->kind;
					int rk = rt->kind;
					if((lk == Ty_nil && rk == Ty_nil) || !equal_ty(lt, rt)) {
						EM_error(a->pos, "same type required");
					}
					switch(lk) {
						case Ty_record: case Ty_int: case Ty_string: case Ty_array: {
							return expTy(NULL, Ty_Int());
							break;
						}
						default: {
							EM_error(a->pos, "error oper type");
						}
					}
					break;
				}
			}
			return expTy(NULL, Ty_Int());
			break;
		}
		case A_recordExp: {
			Ty_ty x = S_look(tenv, a->u.record.typ);
			if(x) {
				Ty_ty ty = actual_ty(x);
				if(ty->kind == Ty_record) {
					Ty_fieldList t;
					A_efieldList e;
					if(a->u.record.fields == NULL && ty->u.record == NULL) {
						return expTy(NULL, ty);
					}
					else {
						for(t = ty->u.record, e = a->u.record.fields; e || t; t = t->tail, e = e->tail) {
							if(e == NULL || t == NULL || e->head->name != t->head->name || !equal_ty(transExp(venv, tenv, e->head->exp).ty, actual_ty(t->head->ty))) {
								EM_error(a->pos, "args type error %s", S_name(a->u.call.func));
								break;
							}
						}
					}
					return expTy(NULL, ty);
				}
				else {
					EM_error(a->pos, "should be record type");
					return expTy(NULL, actual_ty(x));
				}
			}
			else {
				EM_error(a->pos, "undefined type %s", S_name(a->u.record.typ));
				return expTy(NULL, Ty_Void());
			}
			return expTy(NULL, actual_ty(x));
			break;
		}
		case A_seqExp: {
			struct expty exp = expTy(NULL, Ty_Void());
			A_expList el;
			for(el = a->u.seq; el; el = el->tail) {
				exp = transExp(venv, tenv, el->head);
			}
			return exp;
			break;
		}
		case A_assignExp: {
			struct expty e1 = transVar(venv, tenv, a->u.assign.var);
			struct expty e2 = transExp(venv, tenv, a->u.assign.exp);
			if(a->u.assign.var->kind == A_simpleVar) {
				E_enventry e = S_look(venv, a->u.assign.var->u.simple);
				if(e && e->readonly) {
					EM_error(a->pos, "loop variable can't be assigned");
				}
			}
			if(!equal_ty(e1.ty, e2.ty)) {
				EM_error(a->pos, "unmatched assign exp");
			}
			return expTy(NULL, Ty_Void());
			break;
		}
		case A_ifExp: {
			struct expty et = transExp(venv, tenv, a->u.iff.test);
			if(et.ty->kind != Ty_int) {
				EM_error(a->pos, "iff test should be int");
				return expTy(NULL, Ty_Void());
			}
			struct expty e1 = transExp(venv, tenv, a->u.iff.then);
			if(a->u.iff.elsee != NULL) {
				struct expty e2 = transExp(venv, tenv, a->u.iff.elsee);
				if(!equal_ty(e1.ty, e2.ty)) {
					EM_error(a->pos, "then exp and else exp type mismatch");
				}
				return expTy(NULL, e2.ty->kind == Ty_nil ? e1.ty : e2.ty);
			}
			else {
				if(e1.ty->kind != Ty_void) {
					EM_error(a->pos, "if-then exp's body must produce no value");
				}
				return expTy(NULL, e1.ty);
			}
			return expTy(NULL, Ty_Void());
			break;
		}
		case A_whileExp: {
			struct expty et = transExp(venv, tenv, a->u.whilee.test);
			struct expty eb = transExp(venv, tenv, a->u.whilee.body);
			if(et.ty->kind != Ty_int || eb.ty->kind != Ty_void) {
				EM_error(a->pos, "while body must produce no value");
			}
			return expTy(NULL, Ty_Void());
			break;
		}
		case A_forExp: {
			struct expty el = transExp(venv, tenv, a->u.forr.lo);
			struct expty eh = transExp(venv, tenv, a->u.forr.hi);
			if(el.ty->kind != Ty_int || eh.ty->kind != Ty_int) {
				EM_error(a->pos, "for exp's range type is not integer");
			}
			S_beginScope(venv);
			S_enter(venv, a->u.forr.var, E_ROVarEntry(Ty_Int()));
			struct expty eb = transExp(venv, tenv, a->u.forr.body);
			if(eb.ty->kind != Ty_void) {
				EM_error(a->pos, "for body shoule be nil type");
			}
			S_endScope(venv);
			return expTy(NULL, Ty_Void());
			break;
			// mustn't assign a->u.forr.var in a->u.forr.body, it is checked in A_assignExp
		}
		case A_breakExp: {
			return expTy(NULL, Ty_Void());
			break;
			// break must in for or while, but not checked here !!!
		}
		case A_letExp: {
			struct expty exp;
			A_decList d;
			S_beginScope(venv);
			S_beginScope(tenv);
			for(d = a->u.let.decs; d; d = d->tail) {
				transDec(venv, tenv, d->head);
			}
			exp = transExp(venv, tenv, a->u.let.body);
			S_endScope(tenv);
			S_endScope(venv);
			return exp;
			break;
		}
		case A_arrayExp: {
			Ty_ty xr = S_look(tenv, a->u.array.typ);
			Ty_ty x = actual_ty(xr);
			if(xr && x->kind == Ty_array) {
				Ty_ty t = actual_ty(x->u.array);
				struct expty es = transExp(venv, tenv, a->u.array.size);
				if(es.ty->kind != Ty_int) {
					EM_error(a->pos, "array exp index should be int");
					return expTy(NULL, Ty_Array(t));
				}
				struct expty eb = transExp(venv, tenv, a->u.array.init);
				if(!equal_ty(eb.ty, t)) {
					EM_error(a->pos, "type mismatch");
					return expTy(NULL, Ty_Array(t));
				}
				return expTy(NULL, x);
			}
			else {
				EM_error(a->pos, "array type error");
			}
			return expTy(NULL, Ty_Array(Ty_Nil()));
			break;
		}
	}
	assert(0);
	//return expTy(NULL, NULL);
}

void transDec(S_table venv, S_table tenv, A_dec d) {
	switch(d->kind) {
		case A_functionDec: {
			A_fundecList fl, fl1, fl2;
			for(fl1 = d->u.function; fl1; fl1 = fl1->tail) {
				if(fl1 != NULL) {
					for(fl2 = fl1->tail; fl2; fl2 = fl2->tail) {
						if(fl1->head->name == fl2->head->name) {
							EM_error(d->pos, "two functions have the same name");
							return;
						}
					}
				}
			}
			for(fl = d->u.function; fl; fl = fl->tail) {
				A_fundec f = fl->head;
				Ty_ty resultTy = (f->result == NULL) ? Ty_Void() : actual_ty(S_look(tenv, f->result));
				if(resultTy == NULL) {
					EM_error(d->pos, "function return type undeclared");
				}
				Ty_tyList formalTys = makeFormalTyList(tenv, f->params);
				S_enter(venv, f->name, E_FunEntry(formalTys, resultTy));
			}
			for(fl = d->u.function; fl; fl = fl->tail) {
				A_fundec f = fl->head;
				Ty_ty resultTy = (f->result == NULL) ? Ty_Void() : actual_ty(S_look(tenv, f->result));
				Ty_tyList formalTys = makeFormalTyList(tenv, f->params);
				S_beginScope(venv);
				{
					A_fieldList l;
					Ty_tyList t;
					for(l = f->params, t = formalTys; l; l = l->tail, t = t->tail) {
						S_enter(venv, l->head->name, E_VarEntry(t->head));
					}
				}
				struct expty e = transExp(venv, tenv, f->body);
				if(!equal_ty(e.ty, resultTy)) {
					if(resultTy->kind == Ty_void) {
						EM_error(d->pos, "procedure returns value");
					}
					else {
						EM_error(d->pos, "function return type error");
					}
				}
				S_endScope(venv);
			}
			break;
		}
		case A_varDec: {
			struct expty e = transExp(venv, tenv, d->u.var.init);
			Ty_ty t = NULL;
			if(d->u.var.typ != NULL) {
				t = actual_ty(S_look(tenv, d->u.var.typ));
			}
			if(t != NULL && t->kind != Ty_void) {
				if(!equal_ty(e.ty, t)) {
					EM_error(d->pos, "type mismatch");
					break;
				}
				else {
					S_enter(venv, d->u.var.var, E_VarEntry(t));    // t->kind == Ty_nil and e.ty->kind == Ty_array || Ty_record is allowed, so E_VarEntry must use t but e.ty !
				}
			}
			else {
				if(e.ty->kind == Ty_nil) {
					EM_error(d->pos, "init should not be nil without type specified");
					break;
				}
				else {
					S_enter(venv, d->u.var.var, E_VarEntry(e.ty));
				}
			}
			break;
		}
		case A_typeDec: {
			A_nametyList nl, nl1, nl2;
			for(nl1 = d->u.type; nl1; nl1 = nl1->tail) {
				if(nl1 != NULL) {
					for(nl2 = nl1->tail; nl2; nl2 = nl2->tail) {
						if(nl1->head->name == nl2->head->name) {
							EM_error(d->pos, "two types have the same name");
							return;
						}
					}
				}
			}
			for(nl = d->u.type; nl; nl = nl->tail) {
				S_enter(tenv, nl->head->name, Ty_Name(nl->head->name, NULL));
			}
			for(nl = d->u.type; nl; nl = nl->tail) {
				Ty_ty t = S_look(tenv, nl->head->name);
				assert(t->kind == Ty_name);
				t->u.name.ty = transTy(tenv, nl->head->ty);
			}
			int result = checkRecursiveType(tenv, d->u.type);
			if(result == 0) {
				EM_error(d->pos, "illegal type cycle");
			}
			break;
		}
	}
}

Ty_ty transTy (S_table tenv, A_ty t) {
	switch(t->kind) {
		case A_nameTy: {
			Ty_ty resultTy = S_look(tenv, t->u.name);
			if(resultTy == NULL) {
				EM_error(t->pos, "type undeclared");
				return Ty_Int();
			}
			return resultTy;
			break;
		}
		case A_recordTy: {
			A_fieldList f = t->u.record;
			Ty_fieldList tl = makeFormalFieldList(tenv, f);
			return Ty_Record(tl);
			break;
		}
		case A_arrayTy: {
			Ty_ty resultTy = S_look(tenv, t->u.array);
			if(resultTy == NULL) {
				EM_error(t->pos, "type array undeclared");
				return Ty_Int();
			}
			return Ty_Array(resultTy);
			break;
		}
	}
	return NULL;
}

void SEM_transProg(A_exp exp) {
	S_table tenv = E_base_tenv();
	S_table venv = E_base_venv();
	S_enter(tenv, S_Symbol(""), Ty_Void());
	S_enter(venv, S_Symbol("print"), E_FunEntry(Ty_TyList(Ty_String(), NULL), Ty_Void()));
	S_enter(venv, S_Symbol("printi"), E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_Void()));
	S_enter(venv, S_Symbol("flush"), E_FunEntry(NULL, Ty_Void()));
	S_enter(venv, S_Symbol("getchar"), E_FunEntry(NULL, Ty_String()));
	S_enter(venv, S_Symbol("ord"), E_FunEntry(Ty_TyList(Ty_String(), NULL), Ty_Int()));
	S_enter(venv, S_Symbol("chr"), E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_String()));
	S_enter(venv, S_Symbol("size"), E_FunEntry(Ty_TyList(Ty_String(), NULL), Ty_Int()));
	S_enter(venv, S_Symbol("substring"), E_FunEntry(Ty_TyList(Ty_String(), Ty_TyList(Ty_Int(), Ty_TyList(Ty_Int(), NULL))), Ty_String()));
	S_enter(venv, S_Symbol("concat"), E_FunEntry(Ty_TyList(Ty_String(), Ty_TyList(Ty_String(), NULL)), Ty_String()));
	S_enter(venv, S_Symbol("not"), E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_Int()));
	S_enter(venv, S_Symbol("exit"), E_FunEntry(Ty_TyList(Ty_Int(), NULL), Ty_Void()));
	transExp(venv, tenv, exp);
}
