#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "errormsg.h"
#include "tree.h"
#include "assem.h"
#include "frame.h"
#include "codegen.h"
#include "table.h"

static AS_instrList iList = NULL, last = NULL;

static void emit(AS_instr inst) {
	if(last != NULL) {
		last = last->tail = AS_InstrList(inst, NULL);
	}
	else {
		last = iList = AS_InstrList(inst, NULL);
	}
}

static Temp_tempList L(Temp_temp h, Temp_tempList t) {
	return Temp_TempList(h, t);
}

static char as_buf[100];

static Temp_temp munchExp(T_exp);
static Temp_tempList munchArgs(int index, T_expList args) {
	if(args == NULL) {
		return NULL;
	}
	Temp_tempList l = munchArgs(index + 1, args->tail);
	Temp_temp r = munchExp(args->head);
	emit(AS_Oper("pushl `s0", L(F_SP(), NULL), L(r, L(F_SP(), NULL)), NULL));
	return NULL;
}

static Temp_temp munchExp(T_exp e) {
	switch(e->kind) {
		case T_MEM: {
			T_exp mem = e->u.MEM;
			if(mem->kind == T_BINOP
				&& mem->u.BINOP.op == T_plus
				&& mem->u.BINOP.right->kind == T_CONST) {
				// MEM(BINOP(PLUS, e1, CONST(i)))
				Temp_temp r = Temp_newtemp();
				T_exp e1 = mem->u.BINOP.left;
				int i = mem->u.BINOP.right->u.CONST;
				sprintf(as_buf, "movl %d(`s0), `d0", i);
				string as = String(as_buf);    // munchExp(e1) in next line may change as_buf !!!!!
				emit(AS_Oper(as, L(r, NULL), L(munchExp(e1), NULL), NULL));
				return r;
			}
			else if(mem->kind == T_BINOP
				&& mem->u.BINOP.op == T_plus
				&& mem->u.BINOP.left->kind == T_CONST) {
				// MEM(BINOP(PLUS, CONST(i), e1))
				Temp_temp r = Temp_newtemp();
				T_exp e2 = mem->u.BINOP.right;
				int i = mem->u.BINOP.left->u.CONST;
				sprintf(as_buf, "movl %d(`s0), `d0", i);
				string as = String(as_buf);
				emit(AS_Oper(as, L(r, NULL), L(munchExp(e2), NULL), NULL));
				return r;
			}
			else if(mem->kind == T_CONST) {
				// MEM(CONST(i))
				int i = mem->u.CONST;
				Temp_temp r = Temp_newtemp();
				sprintf(as_buf, "movl %d, `d0", i);
				string as = String(as_buf);
				emit(AS_Oper(as, L(r, NULL), NULL, NULL));
				return r;
			}
			else {
				// MEM(e1)
				Temp_temp r = Temp_newtemp();
				T_exp e1 = mem;
				emit(AS_Oper("movl (`s0), `d0", L(r, NULL), L(munchExp(e1), NULL), NULL));
				return r;
			}
			break;
		}
		case T_BINOP: {
			T_exp e1 = e->u.BINOP.left;
			T_exp e2 = e->u.BINOP.right;
			switch(e->u.BINOP.op) {
				case T_plus: {
					// BINOP(PLUS, e1, e2)
					Temp_temp r = Temp_newtemp();
					Temp_temp left = munchExp(e1);
					Temp_temp right = munchExp(e2);
					if(r != left) {
						emit(AS_Move("movl `s0, `d0", L(r, NULL), L(left, NULL)));
					}
					emit(AS_Oper("addl `s1, `d0", L(r, NULL), L(r, L(right, NULL)), NULL));
					return r;
					break;
				}
				case T_minus: {
					// BINOP(MINUS, e1, e2)
					Temp_temp r = Temp_newtemp();
					Temp_temp left = munchExp(e1);
					Temp_temp right = munchExp(e2);
					if(r != left) {
						emit(AS_Move("movl `s0, `d0", L(r, NULL), L(left, NULL)));
					}
					emit(AS_Oper("subl `s1, `d0", L(r, NULL), L(r, L(right, NULL)), NULL));
					return r;
					break;
				}
				case T_mul: {
					// BINOP(MUL, e1, e2)
					Temp_temp r = Temp_newtemp();
					Temp_temp left = munchExp(e1);
					Temp_temp right = munchExp(e2);
					if(r != left) {
						emit(AS_Move("movl `s0, `d0", L(r, NULL), L(left, NULL)));
					}
					emit(AS_Oper("imull `s1, `d0", L(r, NULL), L(r, L(right, NULL)), NULL));
					return r;
					break;
				}
				case T_div: {
					// BINOP(DIV, e1, e2)
					Temp_temp eax = F_RV();
					Temp_temp edx = F_DX();
					Temp_temp left = munchExp(e1);
					Temp_temp right = munchExp(e2);
					if(eax != left) {
						emit(AS_Move("movl `s0, `d0", L(eax, NULL), L(left, NULL)));
					}
					emit(AS_Oper("cltd", L(eax, L(edx, NULL)), L(eax, NULL), NULL));
					emit(AS_Oper("idivl `s0", L(eax, L(edx, NULL)), L(right, L(eax, L(edx, NULL))), NULL));
					return eax;
					break;
				}
				default: {
					assert(0);
					break;
				}
			}
			assert(0);
			break;
		}
		case T_CONST: {
			// CONST(i)
			Temp_temp r = Temp_newtemp();
			int i = e->u.CONST;
			sprintf(as_buf, "movl $%d, `d0", i);
			string as = String(as_buf);
			emit(AS_Oper(as, L(r, NULL), NULL, NULL));
			return r;
			break;
		}
		case T_TEMP: {
			// TEMP(t)
			return e->u.TEMP;
			break;
		}
		case T_NAME: {
			// NAME(s), for string
			Temp_temp r = Temp_newtemp();
			sprintf(as_buf, "movl $.%s, `d0", Temp_labelstring(e->u.NAME));
			string as = String(as_buf);
			emit(AS_Oper(as, L(r, NULL), NULL, NULL));
			return r;
			break;
		}
		case T_CALL: {
			// CALL(e, args)
			if(e->u.CALL.fun->kind == T_NAME) {
				Temp_tempList l = munchArgs(0, e->u.CALL.args);
				assert(l == NULL);    // in x86, don't use register to transfer args
				sprintf(as_buf, "call %s", Temp_labelstring(e->u.CALL.fun->u.NAME));
				string as = String(as_buf);
				emit(AS_Oper(as, L(F_SP(), F_callersaves()), L(F_SP(), l), NULL));    // callersaves includes F_RV()
				int argCnt = 0;
				for(T_expList tmp = e->u.CALL.args; tmp != NULL; tmp = tmp->tail) {
					++argCnt;
				}
				sprintf(as_buf, "addl $%d, `d0", argCnt * F_wordSize);
				as = String(as_buf);
				emit(AS_Oper(as, L(F_SP(), NULL), L(F_SP(), NULL), NULL));
				return F_RV();
			}
			else {
				assert(0);     /////
			}
			break;
		}
		default: {
			assert(0);
		}
	}
	assert(0);
}

static void munchStm(T_stm s) {
	switch(s->kind) {
		case T_MOVE: {
			T_exp dst = s->u.MOVE.dst;
			T_exp src = s->u.MOVE.src;
			if(dst->kind == T_MEM) {    // "STORE"
				if(dst->u.MEM->kind == T_BINOP
					&& dst->u.MEM->u.BINOP.op == T_plus
					&& dst->u.MEM->u.BINOP.right->kind == T_CONST) {
					// MOVE(MEM(BINOP(PLUS, e1, CONST(i))), e2)
					T_exp e1 = dst->u.MEM->u.BINOP.left;
					T_exp e2 = src;
					int i = dst->u.MEM->u.BINOP.right->u.CONST;
					sprintf(as_buf, "movl `s1, %d(`s0)", i);
					string as = String(as_buf);
					emit(AS_Oper(as, NULL, L(munchExp(e1), L(munchExp(e2), NULL)), NULL));
				}
				else if(dst->u.MEM->kind == T_BINOP
					&& dst->u.MEM->u.BINOP.op == T_plus
					&& dst->u.MEM->u.BINOP.left->kind == T_CONST) {
					// MOVE(MEM(BINOP(PLUS, CONST(i), e1)), e2)
					T_exp e1 = dst->u.MEM->u.BINOP.right;
					T_exp e2 = src;
					int i = dst->u.MEM->u.BINOP.left->u.CONST;
					sprintf(as_buf, "movl `s1, %d(`s0)", i);
					string as = String(as_buf);
					emit(AS_Oper(as, NULL, L(munchExp(e1), L(munchExp(e2), NULL)), NULL));
				}
				else if(dst->u.MEM->kind == T_CONST) {
					// MOVE(MEM(CONST(i)), e2)
					T_exp e2 = src;
					int i = dst->u.MEM->u.CONST;
					sprintf(as_buf, "movl `s0, %d", i);
					string as = String(as_buf);
					emit(AS_Oper(as, NULL, L(munchExp(e2), NULL), NULL));
				}
				else {
					// MOVE(MEM(e1), e2)
					T_exp e1 = dst->u.MEM;
					T_exp e2 = src;
					emit(AS_Oper("movl `s1, (`s0)", NULL, L(munchExp(e1), L(munchExp(e2), NULL)), NULL));
				}
			}
			else if(dst->kind == T_TEMP) {
				T_exp e2 = src;
				Temp_temp t2 = munchExp(e2);
				if(dst->u.TEMP != t2) {
					emit(AS_Move("movl `s0, `d0", L(dst->u.TEMP, NULL), L(t2, NULL)));
				}
			}
			else {
				assert(0);
			}
			break;
		}
		case T_JUMP: {    /////
			sprintf(as_buf, "jmp %s", Temp_labelstring(s->u.JUMP.jumps->head));
			string as = String(as_buf);
			emit(AS_Oper(as, NULL, NULL, AS_Targets(s->u.JUMP.jumps)));
			break;
		}
		case T_CJUMP: {
			char *jins;
			switch(s->u.CJUMP.op) {
				case T_eq: {
					jins = "je";
					break;
				}
				case T_ne: {
					jins = "jne";
					break;
				}
				case T_lt: {
					jins = "jl";
					break;
				}
				case T_gt: {
					jins = "jg";
					break;
				}
				case T_le: {
					jins = "jle";
					break;
				}
				case T_ge: {
					jins = "jge";
					break;
				}
				default: {
					assert(0);    /////
					break;
				}
			}
			emit(AS_Oper("cmpl `s1, `s0", NULL, L(munchExp(s->u.CJUMP.left), L(munchExp(s->u.CJUMP.right), NULL)), NULL));
			sprintf(as_buf, "%s %s", jins, Temp_labelstring(s->u.CJUMP.true));
			string as = String(as_buf);
			emit(AS_Oper(as, NULL, NULL, AS_Targets(Temp_LabelList(s->u.CJUMP.true, Temp_LabelList(s->u.CJUMP.false, NULL)))));
			break;
		}
		case T_EXP: {
			T_exp e = s->u.EXP;
			if(e->kind == T_CALL) {
				munchExp(e);
			}
			else {
				;    /////
			}
			break;
		}
		case T_LABEL: {
			sprintf(as_buf, "%s", Temp_labelstring(s->u.LABEL));
			string as = String(as_buf);
			emit(AS_Label(as, s->u.LABEL));
			break;
		}
		default: {
			assert(0);
			break;
		}
	}
}

//Lab 6: put your code here
AS_instrList F_codegen(F_frame f, T_stmList stmList) {
	AS_instrList list;
	T_stmList sl;
	/*?????*/
	for(sl = stmList; sl; sl = sl->tail) {
		munchStm(sl->head);
	}
	list = iList;
	iList = last = NULL;
	return F_procEntryExit2(list);
}
