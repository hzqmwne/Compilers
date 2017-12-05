#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "env.h"
#include "semant.h"
#include "helper.h"
#include "translate.h"
#include <stdlib.h>

/*Lab5: Your implementation of lab5.*/

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
		return Ty_Void();
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
	return new;
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

/* ============================================================================== */

struct expty transVar(S_table venv, S_table tenv, A_var v, Tr_level level, Temp_label label) {
	switch(v->kind) {
		case A_simpleVar: {
			E_enventry x = S_look(venv, v->u.simple);
			if(x && x->kind == E_varEntry) {
				Tr_exp exp = Tr_simpleVar(x->u.var.access, level);
				return expTy(exp, actual_ty(x->u.var.ty));
			}
			else {
				EM_error(v->pos, "undefined variable %s", S_name(v->u.simple));
				return expTy(Tr_nop(), Ty_Void());
			}
			break;
		}
		case A_fieldVar: {
			struct expty e = transVar(venv, tenv, v->u.field.var, level, label);
			if(e.ty->kind != Ty_record) {
				EM_error(v->pos, "not a record type");
				return expTy(Tr_nop(), Ty_Void());
			}
			int cnt = 0;
			Ty_fieldList f;
			for(f = e.ty->u.record; f; f = f->tail, ++cnt) {
				if(v->u.field.sym == f->head->name) {
					return expTy(Tr_fieldVar(e.exp, cnt), actual_ty(f->head->ty));
				}
			}
			EM_error(v->pos, "field %s doesn't exist", S_name(v->u.field.sym));
			return expTy(Tr_nop(), Ty_Void());
			break;
		}
		case A_subscriptVar: {
			struct expty e1 = transVar(venv, tenv, v->u.subscript.var, level, label);
			struct expty e2 = transExp(venv, tenv, v->u.subscript.exp, level, label);
			if(e1.ty->kind != Ty_array) {
				EM_error(v->pos, "array type required");
				return expTy(Tr_nop(), Ty_Void());
			}
			if(e2.ty->kind != Ty_int) {
				EM_error(v->pos, "subscriptvar exp should be int");
				return expTy(Tr_nop(), Ty_Int());
			}
			return expTy(Tr_subscriptVar(e1.exp, e2.exp), actual_ty(e1.ty->u.array));
			break;
		}
	}
	assert(0);
	//return expTy(NULL, NULL);
}

struct expty transExp(S_table venv, S_table tenv, A_exp a, Tr_level level, Temp_label label) {
	switch(a->kind) {
		case A_varExp: {
			return transVar(venv, tenv, a->u.var, level, label);
			break;
		}
		case A_nilExp: {
			return expTy(Tr_nop(), Ty_Nil());
			break;
		}
		case A_intExp: {
			return expTy(Tr_intExp(a->u.intt), Ty_Int());
			break;
		}
		case A_stringExp: {
			return expTy(Tr_strExp(a->u.stringg), Ty_String());
			break;
		}
		case A_callExp: {
			E_enventry x = S_look(venv, a->u.call.func);
			if(x) {    //  x->kind == E_funEntry should not be checked, because E_enventry.kind not assigned !
				Ty_tyList t;
				A_expList e;
				Tr_trExpList reserveOrderArgs = NULL;
				for(t = x->u.fun.formals, e = a->u.call.args; e || t; t = t->tail, e = e->tail) {
					if(e != NULL && t == NULL) {
						EM_error(a->pos, "too many params in function %s", S_name(a->u.call.func));
						return expTy(Tr_nop(), Ty_Int());
						break;
					}
					else if(e == NULL && t != NULL) {
						EM_error(a->pos, "too few params in function %s", S_name(a->u.call.func));
						return expTy(Tr_nop(), Ty_Int());
						break;
					}
					struct expty et = transExp(venv, tenv, e->head, level, label);
					if(e == NULL || t == NULL || !equal_ty(et.ty, actual_ty(t->head))) {
						EM_error(a->pos, "para type mismatch");
						return expTy(Tr_nop(), Ty_Int());
						break;
					}
					reserveOrderArgs = Tr_TrExpList(et.exp, reserveOrderArgs);
				}
				assert(x->u.fun.label != NULL);
				Tr_exp call =  Tr_callExp(x->u.fun.label, reserveOrderArgs, level, x->u.fun.level);
				return expTy(call, actual_ty(x->u.fun.result));
			}
			else {
				EM_error(a->pos, "undefined function %s", S_name(a->u.call.func));
				return expTy(Tr_nop(), Ty_Int());
			}
			break;
		}
		case A_opExp: {
			A_oper oper = a->u.op.oper;
			struct expty left = transExp(venv, tenv, a->u.op.left, level, label);
			struct expty right = transExp(venv, tenv, a->u.op.right, level, label);
			Ty_ty lt = actual_ty(left.ty);
			Ty_ty rt = actual_ty(right.ty);
			int lk = lt->kind;
			int rk = rt->kind;
			switch(oper) {
				case A_plusOp: case A_minusOp: case A_timesOp: case A_divideOp: {
					if(lk != Ty_int) {
						EM_error(a->u.op.left->pos, "integer required");
						return expTy(Tr_nop(), Ty_Int());
					}
					if(rk != Ty_int) {
						EM_error(a->u.op.right->pos, "integer required");
						return expTy(Tr_nop(), Ty_Int());
					}
					return expTy(Tr_opExp(left.exp, right.exp, oper), Ty_Int());
					break;
				}
				case A_eqOp: case A_neqOp: case A_ltOp: case A_leOp: case A_gtOp: case A_geOp: {
					if((lk == Ty_nil && rk == Ty_nil) || !equal_ty(lt, rt)) {
						EM_error(a->pos, "same type required");
						return expTy(Tr_nop(), Ty_Int());
					}
					int k = (lk == Ty_nil) ? rk : lk;
					assert(k != Ty_nil);
					switch(k) {
						case Ty_int: {
							return expTy(Tr_relOpExp(left.exp, right.exp, oper), Ty_Int());
							break;
						}
						case Ty_string: {
							if(oper != A_eqOp && oper != A_neqOp) {
								EM_error(a->pos, "string can only A_eq or A_neq");
								return expTy(Tr_nop(), Ty_Int());
							}
							return expTy(Tr_relOpExp(Tr_stringEq(left.exp, right.exp), Tr_intExp(TRUE), oper), Ty_Int());
							break;
						}
						case Ty_record: case Ty_array:  {
							if(oper != A_eqOp && oper != A_neqOp) {
								EM_error(a->pos, "record and array can only A_eq or A_neq");
								return expTy(Tr_nop(), Ty_Int());
							}
							return expTy(Tr_relOpExp(left.exp, right.exp, oper), Ty_Int());
							break;
						}
						default: {
							EM_error(a->pos, "error oper type");
							return expTy(Tr_nop(), Ty_Int());
						}
					}
					break;
				}
			}
			assert(0);
			break;
		}
		case A_recordExp: {
			Ty_ty x = S_look(tenv, a->u.record.typ);
			if(x) {
				Ty_ty ty = actual_ty(x);
				if(ty->kind == Ty_record) {
					Ty_fieldList t;
					A_efieldList e;
					Tr_trExpList reserveOrderArgs = NULL;
					int argCnt = 0;
					for(t = ty->u.record, e = a->u.record.fields; e || t; t = t->tail, e = e->tail) {
						if(e == NULL || t == NULL) {
							EM_error(a->pos, "args in record too more or too less");
							return expTy(Tr_nop(), ty);
						}
						struct expty et = transExp(venv, tenv, e->head->exp, level, label);
						if(e->head->name != t->head->name || !equal_ty(et.ty, t->head->ty)) {
							EM_error(a->pos, "args type error %s", S_name(a->u.call.func));
							break;
						}
						reserveOrderArgs = Tr_TrExpList(et.exp, reserveOrderArgs);
						++argCnt;
					}
					return expTy(Tr_recordExp(reserveOrderArgs, argCnt, level), ty);
				}
				else {
					EM_error(a->pos, "should be record type");
					return expTy(Tr_nop(), ty);
				}
			}
			else {
				EM_error(a->pos, "undefined type %s", S_name(a->u.record.typ));
				return expTy(Tr_nop(), Ty_Void());
			}
			assert(0);
			break;
		}
		case A_seqExp: {
			Ty_ty ty = Ty_Void();
			Tr_trExpList reserveOrderTrExps = NULL;
			A_expList el;
			for(el = a->u.seq; el; el = el->tail) {
				struct expty et = transExp(venv, tenv, el->head, level, label);
				ty = et.ty;
				reserveOrderTrExps = Tr_TrExpList(et.exp, reserveOrderTrExps);
			}
			return expTy(Tr_seqExp(reserveOrderTrExps), ty);
			break;
		}
		case A_assignExp: {
			struct expty e1 = transVar(venv, tenv, a->u.assign.var, level, label);
			struct expty e2 = transExp(venv, tenv, a->u.assign.exp, level, label);
			if(a->u.assign.var->kind == A_simpleVar) {
				E_enventry e = S_look(venv, a->u.assign.var->u.simple);
				if(e && e->readonly) {
					EM_error(a->pos, "loop variable can't be assigned");
					return expTy(Tr_nop(), Ty_Void());
				}
			}
			if(!equal_ty(e1.ty, e2.ty)) {
				EM_error(a->pos, "unmatched assign exp");
				return expTy(Tr_nop(), Ty_Void());
			}
			return expTy(Tr_asnExp(e1.exp, e2.exp), Ty_Void());
			break;
		}
		case A_ifExp: {
			struct expty et = transExp(venv, tenv, a->u.iff.test, level, label);
			if(et.ty->kind != Ty_int) {
				EM_error(a->pos, "iff test should be int");
				return expTy(Tr_nop(), Ty_Void());
			}
			struct expty e1 = transExp(venv, tenv, a->u.iff.then, level, label);
			if(a->u.iff.elsee != NULL) {
				struct expty e2 = transExp(venv, tenv, a->u.iff.elsee, level, label);
				if(!equal_ty(e1.ty, e2.ty)) {
					EM_error(a->pos, "then exp and else exp type mismatch");
					return expTy(Tr_nop(), Ty_Void());
				}
				Tr_exp result = Tr_ifExp(et.exp, e1.exp, e2.exp, e1.ty->kind == Ty_void, level);
				return expTy(result, e2.ty->kind == Ty_nil ? e1.ty : e2.ty);
			}
			else {
				if(e1.ty->kind != Ty_void) {
					EM_error(a->pos, "if-then exp's body must produce no value");
					return expTy(Tr_nop(), Ty_Void());
				}
				Tr_exp result = Tr_ifExp(et.exp, e1.exp, NULL, TRUE, level);
				return expTy(result, Ty_Void());
			}
			assert(0);
			break;
		}
		case A_whileExp: {
			Temp_label done = Temp_newlabel();
			struct expty et = transExp(venv, tenv, a->u.whilee.test, level, done);
			struct expty eb = transExp(venv, tenv, a->u.whilee.body, level, done);
			if(et.ty->kind != Ty_int || eb.ty->kind != Ty_void) {
				EM_error(a->pos, "while body must produce no value");
				return expTy(Tr_nop(), Ty_Void());
			}
			return expTy(Tr_whileExp(et.exp, eb.exp, done), Ty_Void());
			break;
		}
		case A_forExp: {
			Temp_label done = Temp_newlabel();
			struct expty el = transExp(venv, tenv, a->u.forr.lo, level, done);
			struct expty eh = transExp(venv, tenv, a->u.forr.hi, level, done);
			if(el.ty->kind != Ty_int || eh.ty->kind != Ty_int) {
				EM_error(a->pos, "for exp's range type is not integer");
				return expTy(Tr_nop(), Ty_Void());
			}
			S_beginScope(venv);
			Tr_access loopVar = Tr_allocLocal(level, FALSE);    // don't allow fundec in for loop ???
			S_enter(venv, a->u.forr.var, E_ROVarEntry(loopVar, Ty_Int()));
			struct expty eb = transExp(venv, tenv, a->u.forr.body, level, done);
			if(eb.ty->kind != Ty_void) {
				EM_error(a->pos, "for body shoule be nil type");
				return expTy(Tr_nop(), Ty_Void());
			}
			Tr_exp result = Tr_forExp(loopVar, el.exp, eh.exp, eb.exp, done, level);
			S_endScope(venv);
			return expTy(result, Ty_Void());
			break;
			// mustn't assign a->u.forr.var in a->u.forr.body, it is checked in A_assignExp
		}
		case A_breakExp: {
			if(label == NULL) {
				EM_error(a->pos, "break exp can only appear in while exp or for exp");
				return expTy(Tr_nop(), Ty_Void());
			}
			return expTy(Tr_breakExp(label), Ty_Void());
			break;
		}
		case A_letExp: {
			struct expty et;
			A_decList d;
			Tr_trExpList reserveOrderTrExps = NULL;
			S_beginScope(venv);
			S_beginScope(tenv);
			for(d = a->u.let.decs; d; d = d->tail) {
				Tr_exp exp = transDec(venv, tenv, d->head, level, label);
				reserveOrderTrExps = Tr_TrExpList(exp, reserveOrderTrExps);
			}
			et = transExp(venv, tenv, a->u.let.body, level, label);
			reserveOrderTrExps = Tr_TrExpList(et.exp, reserveOrderTrExps);
			S_endScope(tenv);
			S_endScope(venv);
			return expTy(Tr_seqExp(reserveOrderTrExps), et.ty);
			break;
		}
		case A_arrayExp: {
			Ty_ty xr = S_look(tenv, a->u.array.typ);    // real type
			Ty_ty x = actual_ty(xr);
			if(xr && x->kind == Ty_array) {
				Ty_ty t = actual_ty(x->u.array);
				struct expty es = transExp(venv, tenv, a->u.array.size, level, label);
				if(es.ty->kind != Ty_int) {
					EM_error(a->pos, "array exp index should be int");
					return expTy(Tr_nop(), Ty_Array(t));
				}
				struct expty eb = transExp(venv, tenv, a->u.array.init, level, label);
				if(!equal_ty(eb.ty, t)) {
					EM_error(a->pos, "type mismatch");
					return expTy(Tr_nop(), Ty_Array(t));
				}
				return expTy(Tr_arrayExp(es.exp, eb.exp), x);
			}
			else {
				EM_error(a->pos, "array type error");
				return expTy(Tr_nop(), Ty_Void());
			}
			assert(0);
			break;
		}
	}
	assert(0);
	//return expTy(NULL, NULL);
}

Tr_exp transDec(S_table venv, S_table tenv, A_dec d, Tr_level level, Temp_label label) {
	switch(d->kind) {
		case A_functionDec: {
			A_fundecList fl, fl1, fl2;
			for(fl1 = d->u.function; fl1; fl1 = fl1->tail) {
				if(fl1 != NULL) {
					for(fl2 = fl1->tail; fl2; fl2 = fl2->tail) {
						if(fl1->head->name == fl2->head->name) {
							EM_error(d->pos, "two functions have the same name");
							return Tr_nop();
						}
					}
				}
			}
			for(fl = d->u.function; fl; fl = fl->tail) {    // add all funcs, for recursive define
				A_fundec f = fl->head;
				Ty_ty resultTy = (f->result == NULL) ? Ty_Void() : actual_ty(S_look(tenv, f->result));
				if(resultTy == NULL) {
					EM_error(d->pos, "function return type undeclared");
					return Tr_nop();
				}
				Ty_tyList formalTys = makeFormalTyList(tenv, f->params);
				S_enter(venv, f->name, E_FunEntry(level, Temp_namedlabel(S_name(f->name)), formalTys, resultTy));    /// !!!!! warning !!!!! care functions with same name in different level !
			}
			for(fl = d->u.function; fl; fl = fl->tail) {
				A_fundec f = fl->head;
				Ty_ty resultTy = (f->result == NULL) ? Ty_Void() : actual_ty(S_look(tenv, f->result));
				Ty_tyList formalTys = makeFormalTyList(tenv, f->params);
				S_beginScope(venv);
				U_boolList reserveOrderFormalBools = NULL;
				{
					A_fieldList l;
					for(l = f->params; l; l = l->tail) {
						reserveOrderFormalBools = U_BoolList(l->head->escape, reserveOrderFormalBools);
					}
				}
				U_boolList formalBools = NULL;
				for(; reserveOrderFormalBools; reserveOrderFormalBools = reserveOrderFormalBools->tail) {
					formalBools = U_BoolList(reserveOrderFormalBools->head, formalBools);
				}
				Tr_level new_level = Tr_newLevel(level, Temp_namedlabel(S_name(f->name)), formalBools);     /////?????
				{
					A_fieldList l;
					Ty_tyList t;
					Tr_accessList formals = Tr_formals(new_level);
					for(l = f->params, t = formalTys; l; l = l->tail, t = t->tail, formals = formals->tail) {
						S_enter(venv, l->head->name, E_VarEntry(formals->head, t->head));
					}
				}
				struct expty e = transExp(venv, tenv, f->body, new_level, NULL);///// label should be NULL ? because label is only for break exp ?
				if(!equal_ty(e.ty, resultTy)) {
					if(resultTy->kind == Ty_void) {
						EM_error(d->pos, "procedure returns value");
					}
					else {
						EM_error(d->pos, "function return type error");
					}
					return Tr_nop();
				}
				Tr_exp final_exp = (resultTy->kind == Ty_void)? e.exp : Tr_funcDec(e.exp, new_level);
				Tr_procEntryExit(new_level, final_exp, Tr_formals(new_level));/////
				S_endScope(venv);
			}
			return Tr_nop();
			break;
		}
		case A_varDec: {
			struct expty e = transExp(venv, tenv, d->u.var.init, level, label);
			Ty_ty t = NULL;
			if(d->u.var.typ != NULL) {
				t = S_look(tenv, d->u.var.typ);
				if(t == NULL)  {
					EM_error(d->pos, "type undeclared ???");
					return Tr_nop();
				}
			}
			if(t != NULL && t->kind != Ty_void) {
				if(!equal_ty(e.ty, t)) {
					EM_error(d->pos, "type mismatch");
					//S_enter(venv, d->u.var.var, E_VarEntry(NULL, t));/////
					return Tr_nop();
				}
			}
			else {
				if(e.ty->kind == Ty_nil) {
					EM_error(d->pos, "init should not be nil without type specified");
					return Tr_nop();
				}
			}
			Tr_access new_acc = Tr_allocLocal(level, d->u.var.escape);
			S_enter(venv, d->u.var.var, E_VarEntry(new_acc, e.ty->kind == Ty_nil ? t : e.ty));
			return Tr_initExp(new_acc, e.exp);
			//return NULL;//Tr_Nx(T_Move(Tr_simpleVar(new_acc, level), unEx(e.exp)));
			break;
		}
		case A_typeDec: {
			A_nametyList nl, nl1, nl2;
			for(nl1 = d->u.type; nl1; nl1 = nl1->tail) {
				for(nl2 = nl1->tail; nl2; nl2 = nl2->tail) {
					if(nl1->head->name == nl2->head->name) {
						EM_error(d->pos, "two types have the same name");
						return Tr_nop();
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
			return Tr_nop();
			break;
		}
	}
	assert(0);
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

/* ========================================================================================= */

F_fragList SEM_transProg(A_exp exp){
	S_table tenv = E_base_tenv();
	S_table venv = E_base_venv();
	Tr_level newLevel = Tr_newLevel(Tr_outermost(), Temp_namedlabel("tigermain"), NULL);
	struct expty e = transExp(venv, tenv, exp, newLevel, NULL);
	Tr_procEntryExit(newLevel, Tr_funcDec(e.exp, newLevel), NULL);    // main function need tigermain's return value
	//TODO LAB5: do not forget to add the main frame
	return Tr_getResult();
}

