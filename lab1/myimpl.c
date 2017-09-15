#include "prog1.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int max(int a, int b);
int max3(int a, int b, int c);

int countList(A_expList expList);
int maxargsExp(A_exp exp);
int maxargsExpList(A_expList expList);
int maxargs(A_stm stm);

typedef struct table *Table_;
struct table {string id; int value; Table_ tail; };
Table_ Table(string id, int value, struct table *tail);
Table_ update(Table_ t, string key, int value);
int lookup(Table_ t, string key);
Table_ interpStm(A_stm s, Table_ t);

struct IntAndTable {int i; Table_ t; };
struct IntAndTable newIntAndTable(int i, Table_ t);
struct IntAndTable interpExp(A_exp e, Table_ t);

Table_ interpExpList(A_expList exps, Table_ t);

void interp(A_stm stm);

/* ======================================================================= */

int max(int a, int b) {
	return a>b? a:b;
}

int max3(int a, int b, int c) {
	return max(max(a,b),c);
}


int countList(A_expList expList) {
	switch(expList->kind) {
	case A_pairExpList:
		return 1 + countList(expList->u.pair.tail);
		break;
	case A_lastExpList:
		return 1;
		break;
	default:
		break;
	}
	return 0;
}

int maxargsExp(A_exp exp) {
	switch(exp->kind) {
	case A_idExp: A_numExp:
		return 0;
		break;
	case A_opExp:
		return max(maxargsExp(exp->u.op.left), maxargsExp(exp->u.op.right));
		break;
	case A_eseqExp:
		return max(maxargs(exp->u.eseq.stm), maxargsExp(exp->u.eseq.exp));
		break;
	default:
		break;
	}
	return 0;
}

int maxargsExpList(A_expList expList) {
	switch(expList->kind) {
	case A_pairExpList:
		return max3(countList(expList), maxargsExp(expList->u.pair.head), maxargsExpList(expList->u.pair.tail));
		break;
	case A_lastExpList:
		return maxargsExp(expList->u.last);
		break;
	default:
		break;
	}
	return 0;
}

int maxargs(A_stm stm)
{
	//TODO: put your code here.
	switch(stm->kind) {
	case A_compoundStm:
		return max(maxargs(stm->u.compound.stm1), maxargs(stm->u.compound.stm2));
		break;
	case A_assignStm:
		return maxargsExp(stm->u.assign.exp);
		break;
	case A_printStm:
		return maxargsExpList(stm->u.print.exps);
		break;
	default:
		break;
	}
	return 0;
}

/* ========================================================================= */

Table_ Table(string id, int value, struct table *tail) {
	Table_ t = malloc(sizeof(*t));
	t->id=id; t->value=value; t->tail=tail;
	return t;
}

Table_ update(Table_ t, string key, int value) {
	Table_ t2 = malloc(sizeof(*t));
	t2->id=key; t2->value=value; t2->tail=t;
	return t2;
}

int lookup(Table_ t, string key) {
	if(t==NULL) {
		return 0;
	}
	if(!strcmp(t->id, key)) {    // ???????????????????????????????
		return t->value;
	}
	return lookup(t->tail, key);
}

struct IntAndTable newIntAndTable(int i, Table_ t) {
	struct IntAndTable iat;
	iat.i = i;
	iat.t = t;
	return iat;
}

Table_ interpStm(A_stm s, Table_ t) {
	switch(s->kind) {
	case A_compoundStm:
		return interpStm(s->u.compound.stm2, interpStm(s->u.compound.stm1, t));
		break;
	case A_assignStm: {
		struct IntAndTable iat = interpExp(s->u.assign.exp, t);
		return update(iat.t, s->u.assign.id, iat.i);
		break;
	}
	case A_printStm:
		return interpExpList(s->u.print.exps, t);
		break;
	default:
		break;
	}
	return NULL;
}

struct IntAndTable interpExp(A_exp e, Table_ t) {  /////////////////////////////////
	switch(e->kind) {
	case A_idExp:
		return newIntAndTable(lookup(t, e->u.id), t);
		break;
	case A_numExp:
		return newIntAndTable(e->u.num, t);
		break;
	case A_opExp: {
		struct IntAndTable left = interpExp(e->u.op.left, t);
		struct IntAndTable right = interpExp(e->u.op.right, left.t);
		switch(e->u.op.oper) {
		case A_plus:
			return newIntAndTable(left.i + right.i, right.t);
			break;
		case A_minus:
			return newIntAndTable(left.i - right.i, right.t);
			break;
		case A_times:
			return newIntAndTable(left.i * right.i, right.t);
			break;
		case A_div:
			return newIntAndTable(left.i / right.i, right.t);
			break;
		default:
			break;
		}
		break;
	}
	case A_eseqExp:
		return interpExp(e->u.eseq.exp, interpStm(e->u.eseq.stm, t));
		break;
	default:
		break;
	}
	return newIntAndTable(0, NULL);
}

Table_ interpExpList(A_expList exps, Table_ t) {   ////////////////////////////
	switch(exps->kind) {
	case A_pairExpList: {
		struct IntAndTable head = interpExp(exps->u.pair.head, t);
		printf("%d ", head.i);
		return interpExpList(exps->u.pair.tail, head.t);
		break;
	}
	case A_lastExpList: {
		struct IntAndTable last = interpExp(exps->u.last, t);
		printf("%d \n", last.i);
		return last.t;
		break;
	}
	default:
		break;
	}
	return NULL;
}

void interp(A_stm stm)
{
	//TODO: put your code here.
	interpStm(stm, Table("", 0, NULL));
}
