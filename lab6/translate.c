#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "table.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "tree.h"
#include "printtree.h"
#include "frame.h"
#include "translate.h"

//LAB5: you can modify anything you want.

struct Tr_access_ {
	Tr_level level;
	F_access access;
};

struct Tr_level_ {
	F_frame frame;
	Tr_level parent;
};

typedef struct patchList_ *patchList;
struct patchList_ 
{
	Temp_label *head; 
	patchList tail;
};

struct Cx 
{
	patchList trues; 
	patchList falses; 
	T_stm stm;
};

struct Tr_exp_ {
	enum {Tr_ex, Tr_nx, Tr_cx} kind;
	union {T_exp ex; T_stm nx; struct Cx cx; } u;
};

struct Tr_trExpList_ {
	Tr_exp head;
	Tr_trExpList tail;
};

static Tr_level outermost = NULL;
static F_fragList all_frags = NULL;

static Tr_exp Tr_Ex(T_exp ex) {
	Tr_exp p = checked_malloc(sizeof(*p));
	p->kind = Tr_ex;
	p->u.ex = ex;
	return p;
}

static Tr_exp Tr_Nx(T_stm nx) {
	Tr_exp p = checked_malloc(sizeof(*p));
	p->kind = Tr_nx;
	p->u.nx = nx;
	return p;
}

static Tr_exp Tr_Cx(patchList trues, patchList falses, T_stm stm) {
	Tr_exp p = checked_malloc(sizeof(*p));
	p->kind = Tr_cx;
	p->u.cx.trues = trues;
	p->u.cx.falses = falses;
	p->u.cx.stm = stm;
	return p;
}

static patchList PatchList(Temp_label *head, patchList tail)
{
	patchList list;

	list = (patchList)checked_malloc(sizeof(struct patchList_));
	list->head = head;
	list->tail = tail;
	return list;
}

static void doPatch(patchList tList, Temp_label label)
{
	for(; tList; tList = tList->tail)
		*(tList->head) = label;
}

/*
static patchList joinPatch(patchList first, patchList second)
{
	if(!first) return second;
	for(; first->tail; first = first->tail);
	first->tail = second;
	return first;
}
*/

static T_exp unEx(Tr_exp e) {
	switch(e->kind) {
		case Tr_ex: {
			return e->u.ex;
			break;
		}
		case Tr_cx: {
			Temp_temp r = Temp_newtemp();
			Temp_label t = Temp_newlabel(), f = Temp_newlabel();
			doPatch(e->u.cx.trues, t);
			doPatch(e->u.cx.falses, f);
			return T_Eseq(T_Move(T_Temp(r), T_Const(1)),
				T_Eseq(e->u.cx.stm,
				T_Eseq(T_Label(f),
				T_Eseq(T_Move(T_Temp(r), T_Const(0)),
				T_Eseq(T_Label(t),
				T_Temp(r))))));
			break;
		}
		case Tr_nx: {
			if(e->u.nx->kind == T_EXP) {
				return e->u.nx->u.EXP;
			}
			return T_Eseq(e->u.nx, T_Const(0));
			break;
		}
	}
	assert(0);
}

static T_stm unNx(Tr_exp e) {
	switch(e->kind) {
		case Tr_ex: {
			if(e->u.ex->kind == T_ESEQ && e->u.ex->u.ESEQ.exp->kind == T_CONST) {
				return e->u.ex->u.ESEQ.stm;
			}
			return T_Exp(e->u.ex);
			break;
		}
		case Tr_cx: {
			assert(e->u.cx.trues != NULL && e->u.cx.falses != NULL);
			return e->u.cx.stm;
			break;
		}
		case Tr_nx: {
			return e->u.nx;
			break;
		}
	}
	assert(0);
}

static struct Cx unCx(Tr_exp e)	{
	switch(e->kind) {
		case Tr_ex: {
			struct Cx cx;
			T_stm s = T_Cjump(T_ne, e->u.ex, T_Const(0), NULL, NULL);
			patchList trues = PatchList(&s->u.CJUMP.true, NULL);
			patchList falses = PatchList(&s->u.CJUMP.false, NULL);
			cx.trues = trues;
			cx.falses = falses;
			cx.stm = s;
			return cx;
			break;
		}
		case Tr_cx: {
			return e->u.cx;
			break;
		}
		case Tr_nx: {
			assert(0);
			break;
		}
	}
	assert(0);
}

// /* >>>>>
struct staticLinkCache_ {
	Tr_level current;
	Tr_level declare;
	T_exp r;
};
typedef struct staticLinkCache_ *staticLinkCache;

static TAB_table levelToStaticLinkTemp = NULL;

staticLinkCache StaticLinkCache(Tr_level current, Tr_level declare, T_exp r) {
	staticLinkCache p = checked_malloc(sizeof(*p));
	p->current = current;
	p->declare = declare;
	p->r = r;
	return p;
}
// <<<<< */

static Tr_exp Tr_staticLink(Tr_level current, Tr_level declare) {    // return value is FP(frame pointer) of each level
	assert(current != NULL);
	T_exp result = T_Temp(F_FP());
	// /* >>>>>
	if(current == declare) {
		return Tr_Ex(result);
	}
	if(levelToStaticLinkTemp == NULL) {
		levelToStaticLinkTemp = TAB_empty();
	}
	staticLinkCache cache = (staticLinkCache)TAB_look(levelToStaticLinkTemp, declare);
	if(cache != NULL && cache->current == current && cache->declare == declare) {
		return Tr_Ex(cache->r);
	}
	// <<<<< */
	Tr_level now;
	for(now = current; now != declare; now = now->parent) {
		assert(now != NULL);
		F_access sl = F_formals(now->frame)->head;    // must be inFrame(8) in x86
		assert(sl->kind == inFrame && sl->u.offset == 8);
		result = F_exp(sl, result);
	}
	// /* >>>>>
	Temp_temp t = Temp_newtemp();
	Temp_temp t2 = Temp_newtemp();
	result = T_Eseq(T_Seq(T_Move(T_Temp(t2), result), T_Move(T_Temp(t), T_Temp(t2))), T_Temp(t2));
	staticLinkCache newCache = StaticLinkCache(current, declare, T_Temp(t));
	TAB_enter(levelToStaticLinkTemp, declare, newCache);
	// <<<<< */
	return Tr_Ex(result);
}

Tr_trExpList Tr_TrExpList(Tr_exp head, Tr_trExpList tail) {
	Tr_trExpList p = checked_malloc(sizeof(*p));
	p->head = head;
	p->tail = tail;
	return p;
}

Tr_access Tr_Access(Tr_level level, F_access access) {
	Tr_access p = checked_malloc(sizeof(*p));
	p->level = level;
	p->access = access;
	return p;
}
Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail) {
	Tr_accessList p = checked_malloc(sizeof(*p));
	p->head = head;
	p->tail = tail;
	return p;
}

Tr_level Tr_outermost(void) {
	if(!outermost) {
		Tr_level p = checked_malloc(sizeof(*p));
		p->parent = NULL;
		p->frame = NULL;
		outermost = p;
	}
	return outermost;
}

Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals) {
	Tr_level p = checked_malloc(sizeof(*p));
	p->parent = parent;
	p->frame = F_newFrame(name, U_BoolList(TRUE, formals));
	return p;
}

static Tr_accessList f_accl2tr_accl(Tr_level level, F_accessList f_accl) {
	if(!f_accl) {
		return NULL;
	}
	Tr_access head = Tr_Access(level, f_accl->head);
	Tr_accessList tail = f_accl2tr_accl(level, f_accl->tail);
	return Tr_AccessList(head, tail);
}
Tr_accessList Tr_formals(Tr_level level) {
	F_accessList f_accl =  F_formals(level->frame);
	Tr_accessList tr_accl = f_accl2tr_accl(level, f_accl->tail);    // the first item in f_accl is static link !
	return tr_accl;
}

Tr_access Tr_allocLocal(Tr_level level, bool escape) {
	return Tr_Access(level, F_allocLocal(level->frame, escape));
}

/* ========================== */

Tr_exp Tr_simpleVar(Tr_access acc, Tr_level lv) {
	return Tr_Ex(F_exp(acc->access, unEx(Tr_staticLink(lv, acc->level))));
}

Tr_exp Tr_fieldVar(Tr_exp host, int cnt) {
	return Tr_Ex(T_Mem(T_Binop(T_plus, unEx(host), T_Const(cnt * F_wordSize))));
}

Tr_exp Tr_subscriptVar(Tr_exp host, Tr_exp idx) {
	return Tr_Ex(T_Mem(T_Binop(T_plus, unEx(host), T_Binop(T_mul, unEx(idx), T_Const(F_wordSize)))));
}

Tr_exp Tr_nop() {
	return Tr_Ex(T_Const(0));
}

Tr_exp Tr_intExp(int v) {
	return Tr_Ex(T_Const(v));
}

Tr_exp Tr_strExp(string str) {    // the str is tiger form, which first 4bytes is length
	Temp_label new_label = Temp_newlabel();
	F_frag new_frag = F_StringFrag(new_label, str);
	all_frags = F_FragList(new_frag, all_frags);
	return Tr_Ex(T_Name(new_label));
}

Tr_exp Tr_opExp(Tr_exp left, Tr_exp right, A_oper oper) {
	T_binOp binOp;
	switch(oper) {
		case A_plusOp: {
			binOp = T_plus;
			break;
		}
		case A_minusOp: {
			binOp = T_minus;
			break;
		}
		case A_timesOp: {
			binOp = T_mul;
			break;
		}
		case A_divideOp: {
			binOp = T_div;
			break;
		}
		default: {
			assert(0);
		}
	}
	return Tr_Ex(T_Binop(binOp, unEx(left), unEx(right)));
}

Tr_exp Tr_relOpExp(Tr_exp left, Tr_exp right, A_oper oper) {
	T_relOp relOp;
	switch(oper) {
		case A_eqOp: {
			relOp = T_eq;
			break;
		}
		case A_neqOp: {
			relOp = T_ne;
			break;
		}
		case A_ltOp: {
			relOp = T_lt;
			break;
		}
		case A_leOp: {
			relOp = T_le;
			break;
		}
		case A_gtOp: {
			relOp = T_gt;
			break;
		}
		case A_geOp: {
			relOp = T_ge;
			break;
		}
		default: {
			assert(0);
		}
	}
	T_stm s = T_Cjump(relOp, unEx(left), unEx(right), NULL, NULL);
	patchList trues = PatchList(&s->u.CJUMP.true, NULL);
	patchList falses = PatchList(&s->u.CJUMP.false, NULL);
	return Tr_Cx(trues, falses, s);
}

Tr_exp Tr_stringEq(Tr_exp left, Tr_exp right) {
	//Tr_access acc = Tr_allocLocal(current, TRUE);/////////????????
	//Tr_access acc = Tr_allocLocal(current, TRUE);/////////????????
	return Tr_Ex(F_externalCall("stringEqual", T_ExpList(unEx(left), T_ExpList(unEx(right), NULL))));
}

Tr_exp Tr_ifExp(Tr_exp condp, Tr_exp thenp, Tr_exp elsep, bool isVoid, Tr_level current) {
	Temp_label t = Temp_newlabel();
	Temp_label f = Temp_newlabel();
	Temp_label done = Temp_newlabel();
	T_exp r = NULL;/////?????
	if(!isVoid) {
		F_access acc = F_allocLocal(current->frame, FALSE);
		assert(acc->kind == inReg);
		r = F_exp(acc, NULL);
	}
	struct Cx cond = unCx(condp);
	doPatch(cond.trues, t);
	doPatch(cond.falses, f);
	T_stm seq = T_Seq(cond.stm, 
			T_Seq(T_Label(t), 
			T_Seq(isVoid? unNx(thenp):T_Move(r, unEx(thenp)), (elsep==NULL)?T_Label(f):
				T_Seq(T_Jump(T_Name(done), Temp_LabelList(done, NULL)), 
				T_Seq(T_Label(f), 
				T_Seq(isVoid? unNx(elsep):T_Move(r, unEx(elsep)), T_Label(done)
			))))));
	return isVoid ? Tr_Nx(seq) : Tr_Ex(T_Eseq(seq, r));
}

Tr_exp Tr_whileExp(Tr_exp condp, Tr_exp bodyExp, Temp_label done) {
	Temp_label test = Temp_newlabel();
	Temp_label t = Temp_newlabel();
	struct Cx cond = unCx(condp);
	doPatch(cond.trues, t);
	doPatch(cond.falses, done);
	return Tr_Nx(T_Seq(T_Label(test), 
		T_Seq(cond.stm, 
		T_Seq(T_Label(t), 
		T_Seq(unNx(bodyExp), 
		T_Seq(T_Jump(T_Name(test), Temp_LabelList(test, NULL)), 
			T_Label(done)))))));
}

Tr_exp Tr_forExp(Tr_access loopVar, Tr_exp lo, Tr_exp hi, Tr_exp bodyExp, Temp_label done, Tr_level current) {
	Temp_label start = Temp_newlabel();
	Temp_label t = Temp_newlabel();
	T_exp i = F_exp(loopVar->access, NULL);
	F_access acc = F_allocLocal(current->frame, FALSE);
	assert(acc->kind == inReg);
	T_exp limit = F_exp(acc, NULL);
	return Tr_Nx(T_Seq(T_Move(limit, unEx(hi)), 
		T_Seq(T_Move(i, unEx(lo)), 
		T_Seq(T_Cjump(T_le, i, limit, start, done), 
		T_Seq(T_Label(start), 
		T_Seq(unNx(bodyExp), 
		T_Seq(T_Cjump(T_lt, i, limit, t, done), 
		T_Seq(T_Label(t), 
		T_Seq(T_Move(i, T_Binop(T_plus, i, T_Const(1))), 
		T_Seq(T_Jump(T_Name(start), Temp_LabelList(start, NULL)), 
			T_Label(done)))))))))));
}

Tr_exp Tr_asnExp(Tr_exp leftExp, Tr_exp rightExp) {
	return Tr_Nx(T_Move(unEx(leftExp), unEx(rightExp)));
}

Tr_exp Tr_callExp(Temp_label name, Tr_trExpList reserveOrderArgs, Tr_level current, Tr_level declare) {
	T_expList final = NULL;
	Tr_trExpList tmp;
	for(tmp = reserveOrderArgs; tmp; tmp = tmp->tail) {
		//Tr_access acc = Tr_allocLocal(current, TRUE);/////////????????
		final = T_ExpList(unEx(tmp->head), final);
	}
	if(declare == Tr_outermost()) {
		return Tr_Ex(F_externalCall(Temp_labelstring(name), final));
	}
	Tr_exp sl = Tr_staticLink(current, declare);
	final = T_ExpList(unEx(sl), final);
	return Tr_Ex(T_Call(T_Name(name), final));
}

Tr_exp Tr_seqExp(Tr_trExpList reserveOrderTrExps) {
	T_exp eseqs = NULL;
	Tr_trExpList tmp;
	for(tmp = reserveOrderTrExps; tmp; tmp = tmp->tail) {
		eseqs = (eseqs == NULL)? unEx(tmp->head) : T_Eseq(unNx(tmp->head), eseqs);
	}
	return (eseqs == NULL)? Tr_nop() : Tr_Ex(eseqs);
}

Tr_exp Tr_recordExp(Tr_trExpList reserveOrderArgs, int argCnt, Tr_level current) {
	Tr_trExpList tmp;
	T_stm stms = NULL;
	int index = argCnt;
	F_access acc = F_allocLocal(current->frame, 0);
	assert(acc->kind == inReg);
	T_exp r = F_exp(acc, NULL);/////?????
	for(tmp = reserveOrderArgs; tmp; tmp = tmp->tail) {
		--index;
		T_stm mov = T_Move(T_Mem(T_Binop(T_plus, r, T_Const(index * F_wordSize))), unEx(tmp->head));
		stms = (stms == NULL)? mov : T_Seq(mov, stms);
	}
	assert(index == 0);
	//Tr_access acc = Tr_allocLocal(current, 1);/////////????????
	T_exp record = F_externalCall("allocRecord", T_ExpList(T_Const(argCnt * F_wordSize), NULL));
	T_stm init = T_Move(r, record);
	stms = (stms == NULL)? init : T_Seq(init, stms);
	return Tr_Ex(T_Eseq(stms, r));
}

Tr_exp Tr_arrayExp(Tr_exp size, Tr_exp init) {
	//Tr_access acc = Tr_allocLocal(current, 1);/////////????????
	//Tr_access acc = Tr_allocLocal(current, 1);/////////????????
	T_exp array = F_externalCall("initArray", T_ExpList(unEx(size), T_ExpList(unEx(init), NULL)));
	return Tr_Ex(array);
}

Tr_exp Tr_breakExp(Temp_label target) {
	return Tr_Nx(T_Jump(T_Name(target), Temp_LabelList(target, NULL)));
}

Tr_exp Tr_initExp(Tr_access var, Tr_exp init) {
	return Tr_Nx(T_Move(F_exp(var->access, T_Temp(F_FP())), unEx(init)));
}

Tr_exp Tr_funcDec(Tr_exp body, Tr_level lv) {
	return Tr_Nx(T_Move(T_Temp(F_RV()), unEx(body)));
}

void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals) {
	T_stm result = F_procEntryExit1(level->frame, unNx(body));
	F_frag new_frag = F_ProcFrag(result, level->frame);
	all_frags = F_FragList(new_frag, all_frags);
}

F_fragList Tr_getResult(void) {
	return all_frags;
}
