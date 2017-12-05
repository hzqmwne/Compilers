#include <stdio.h>
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "escape.h"
#include "table.h"

typedef struct Esc_escapeEntry_ *Esc_escapeEntry;
struct Esc_escapeEntry_ {
	int depth;
	bool *escape;
};
Esc_escapeEntry Esc_EscapeEntry(int depth, bool *escape) {
	Esc_escapeEntry p = (Esc_escapeEntry)checked_malloc(sizeof(*p));
	p->depth = depth;
	p->escape = escape;
	return p;
}

static void traverseExp(S_table env, int depth, A_exp e);
static void traverseDec(S_table env, int depth, A_dec d);
static void traverseVar(S_table env, int depth, A_var v);

static void traverseExp(S_table env, int depth, A_exp e) {
	switch(e->kind) {
		case A_varExp: {
			traverseVar(env, depth, e->u.var);
			break;
		}
		case A_nilExp: {
			break;
		}
		case A_intExp: {
			break;
		}
		case A_stringExp: {
			break;
		}
		case A_callExp: {
			for(A_expList el = e->u.call.args; el; el = el->tail) {
				traverseExp(env, depth, el->head);
			}
			break;
		}
		case A_opExp: {
			traverseExp(env, depth, e->u.op.left);
			traverseExp(env, depth, e->u.op.right);
			break;
		}
		case A_recordExp: {
			for(A_efieldList fl = e->u.record.fields; fl; fl = fl->tail) {
				traverseExp(env, depth, fl->head->exp);
			}
			break;
		}
		case A_seqExp: {
			for(A_expList el = e->u.seq; el; el = el->tail) {
				traverseExp(env, depth, el->head);
			}
			break;
		}
		case A_assignExp: {
			traverseVar(env, depth, e->u.assign.var);
			traverseExp(env, depth, e->u.assign.exp);
			break;
		}
		case A_ifExp: {
			traverseExp(env, depth, e->u.iff.test);
			traverseExp(env, depth, e->u.iff.then);
			if(e->u.iff.elsee != NULL) {
				traverseExp(env, depth, e->u.iff.elsee);
			}
			break;
		}
		case A_whileExp: {
			traverseExp(env, depth, e->u.whilee.test);
			traverseExp(env, depth, e->u.whilee.body);
			break;
		}
		case A_forExp: {
			traverseExp(env, depth, e->u.forr.lo);
			traverseExp(env, depth, e->u.forr.hi);
			S_beginScope(env);
			e->u.forr.escape = FALSE;
			S_enter(env, e->u.forr.var, Esc_EscapeEntry(depth, &(e->u.forr.escape)));
			traverseExp(env, depth, e->u.forr.body);
			S_endScope(env);
			break;
		}
		case A_breakExp: {
			break;
		}
		case A_letExp: {
			S_beginScope(env);
			for(A_decList d = e->u.let.decs; d; d = d->tail) {
				traverseDec(env, depth, d->head);
			}
			traverseExp(env, depth, e->u.let.body);
			S_endScope(env);
			break;
		}
		case A_arrayExp: {
			traverseExp(env, depth, e->u.array.size);
			traverseExp(env, depth, e->u.array.init);
			break;
		}
		default: {
			assert(0);
			break;
		}
	}
}

static void traverseDec(S_table env, int depth, A_dec d) {
	switch(d->kind) {
		case A_functionDec: {
			for(A_fundecList fl = d->u.function; fl; fl = fl->tail) {
				A_fundec f = fl->head;
				S_beginScope(env);
				for(A_fieldList l = f->params; l; l = l->tail) {
					l->head->escape = FALSE;
					S_enter(env, l->head->name, Esc_EscapeEntry(depth + 1, &(l->head->escape)));
				}
				traverseExp(env, depth + 1, f->body);
				S_endScope(env);
			}
			break;
		}
		case A_varDec: {
			traverseExp(env, depth, d->u.var.init);
			d->u.var.escape = FALSE;
			S_enter(env, d->u.var.var, Esc_EscapeEntry(depth, &(d->u.var.escape)));
			break;
		}
		case A_typeDec: {
			break;
		}
		default: {
			assert(0);
			break;
		}
	}
}

static void traverseVar(S_table env, int depth, A_var v) {
	switch(v->kind) {
		case A_simpleVar: {
			Esc_escapeEntry x = S_look(env, v->u.simple);
			if(x) {
				if(depth > x->depth) {
					*(x->escape) = TRUE;
				}
			}
			break;
		}
		case A_fieldVar: {
			traverseVar(env, depth, v->u.field.var);
			break;
		}
		case A_subscriptVar: {
			traverseVar(env, depth, v->u.subscript.var);
			traverseExp(env, depth, v->u.subscript.exp);
			break;
		}
		default: {
			assert(0);
		}
	}
}

void Esc_findEscape(A_exp exp) {
	//your code here	
	S_table escapeEnv = S_empty();
	traverseExp(escapeEnv, 1, exp);
}
