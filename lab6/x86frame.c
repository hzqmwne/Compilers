#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"
#include "tree.h"
#include "frame.h"

/*Lab5: Your implementation here.*/

struct F_frame_ {
	Temp_label name;

	F_accessList formals;    // the first formal is static link
	F_accessList locals;    // reserve order !

	//the number of arguments
	int argSize;

	//the number of local variables
	int length;

	//register lists for the frame
	Temp_tempList calleesaves;
	Temp_tempList callersaves;
	Temp_tempList specialregs;
};

static F_access InFrame(int offset) {
	F_access p = checked_malloc(sizeof(*p));
	p->kind = inFrame;
	p->u.offset = offset;
	return p;
}

static F_access InReg(Temp_temp reg) {
	F_access p = checked_malloc(sizeof(*p));
	p->kind = inReg;
	p->u.reg = reg;
	return p;
}

F_frag F_StringFrag(Temp_label label, string str) {
	F_frag p = checked_malloc(sizeof(*p));
	p->kind = F_stringFrag;
	p->u.stringg.label = label;
	p->u.stringg.str = str;
	return p;
}   
                                                      
F_frag F_ProcFrag(T_stm body, F_frame frame) {
	F_frag p = checked_malloc(sizeof(*p));
	p->kind = F_procFrag;
	p->u.proc.body = body;
	p->u.proc.frame = frame;
	return p;
}
                                                      
F_fragList F_FragList(F_frag head, F_fragList tail) {
	F_fragList p = checked_malloc(sizeof(*p));
	p->head = head;
	p->tail = tail;
	return p;
}

F_accessList F_AccessList(F_access head, F_accessList tail) {
	F_accessList p = checked_malloc(sizeof(*p));
	p->head = head;
	p->tail = tail;
	return p;
}

const int F_wordSize = 4;

F_frame F_newFrame(Temp_label name, U_boolList formals) {
	F_accessList accListHead = NULL;
	F_accessList accListTail = NULL;
	int offset = F_wordSize * 2;    // 8
	assert(offset == 8);
	int argCnt = 0;
	for(; formals; formals = formals->tail) {
		F_accessList p = checked_malloc(sizeof(*p));
		p->head = InFrame(offset);
		p->tail = NULL;
		if(accListTail == NULL) {
			accListHead = p;
			accListTail = p;
		}
		else {
			accListTail->tail = p;
			accListTail = p;
		}
		offset += F_wordSize;
		++argCnt;
	}
	F_frame f = checked_malloc(sizeof(*f));
	f->name = name;
	f->formals = accListHead;
	f->argSize = argCnt;
	f->length = 0;
	return f;
}

Temp_label F_name(F_frame f) {
	return f->name;
}

F_accessList F_formals(F_frame f) {
	return f->formals;
}

F_access F_allocLocal(F_frame f, bool escape) {
	F_access newAcc;
	if(escape) {
		f->length += 1;
		newAcc = InFrame(f->length * (-F_wordSize));
	}
	else {
		newAcc = InReg(Temp_newtemp());
	}
	f->locals = F_AccessList(newAcc, f->locals);
	return newAcc;
}

F_access F_staticLinkFormal(F_frame f) {
	return f->formals->head;     // assert the first formal is static link
}

Temp_temp F_SP(void) {    // %esp, stack pointer
	static Temp_temp sp = NULL;
	if (!sp) {
		sp = Temp_newtemp();
	}
	return sp;
}
Temp_temp F_FP(void) {    // %ebp, frame pointer
	static Temp_temp fp = NULL;
	if (!fp) {
		fp = Temp_newtemp();
	}
	return fp;
}
Temp_temp F_RV(void) {    // %eax, for return value and idivl
	static Temp_temp rv = NULL;
	if (!rv) {
		rv = Temp_newtemp();
	}
	return rv;
}
Temp_temp F_DX(void) {    // %edx, for idivl
	static Temp_temp dx = NULL;
	if (!dx) {
		dx = Temp_newtemp();
	}
	return dx;
}
Temp_temp F_CX(void) {    // %ecx
	static Temp_temp cx = NULL;
	if (!cx) {
		cx = Temp_newtemp();
	}
	return cx;
}
Temp_temp F_BX(void) {    // %ebx
	static Temp_temp bx = NULL;
	if (!bx) {
		bx = Temp_newtemp();
	}
	return bx;
}
Temp_temp F_DI(void) {    // %edi
	static Temp_temp di = NULL;
	if (!di) {
		di = Temp_newtemp();
	}
	return di;
}
Temp_temp F_SI(void) {    // %esi
	static Temp_temp si = NULL;
	if (!si) {
		si = Temp_newtemp();
	}
	return si;
}

Temp_map F_tempMap = NULL;
Temp_map F_initial() {
	Temp_map tempMap = Temp_empty();
	Temp_enter(tempMap, F_SP(), "%esp");
	Temp_enter(tempMap, F_FP(), "%ebp");
	Temp_enter(tempMap, F_RV(), "%eax");
	Temp_enter(tempMap, F_DX(), "%edx");
	Temp_enter(tempMap, F_CX(), "%ecx");
	Temp_enter(tempMap, F_BX(), "%ebx");
	Temp_enter(tempMap, F_DI(), "%edi");
	Temp_enter(tempMap, F_SI(), "%esi");
	return tempMap;
}

Temp_tempList F_callersaves() {
	static Temp_tempList callersaves = NULL;
	if(!callersaves) {
		callersaves = Temp_TempList(F_RV(), Temp_TempList(F_CX(), Temp_TempList(F_DX(), NULL)));
	}
	return callersaves;
}
Temp_tempList F_calleesaves() {
	static Temp_tempList calleesaves = NULL;
	if(!calleesaves) {
		calleesaves = Temp_TempList(F_BX(), Temp_TempList(F_SI(), Temp_TempList(F_DI(), NULL)));
	}
	return calleesaves;
}
Temp_tempList F_specialregs() {
	static Temp_tempList specialregs = NULL;
	if(!specialregs) {
		specialregs = Temp_TempList(F_SP(), Temp_TempList(F_FP(), NULL));
	}
	return specialregs;
}

Temp_tempList F_registers(void) {
	static Temp_tempList registers = NULL;
	if(!registers) {
		registers = Temp_TempList(F_RV(), 
				Temp_TempList(F_CX(), 
				Temp_TempList(F_DX(), 
				Temp_TempList(F_BX(), 
				Temp_TempList(F_SI(), 
				Temp_TempList(F_DI(), 
				Temp_TempList(F_SP(), 
				Temp_TempList(F_FP(), NULL))))))));
	}
	return registers;
}


T_exp F_exp(F_access acc, T_exp framePtr) {
	switch(acc->kind) {
		case inFrame: {
			return T_Mem(T_Binop(T_plus, framePtr, T_Const(acc->u.offset)));
			break;
		}
		case inReg: {
			return T_Temp(acc->u.reg);
			break;
		}
	}
	assert(0);
}

T_exp F_externalCall(string s, T_expList args) {
	return T_Call(T_Name(Temp_namedlabel(s)), args);
}

T_stm F_procEntryExit1(F_frame frame, T_stm stm) {
	T_stm result = T_Seq(T_Exp(T_Const(0)), NULL);
	T_stm after = T_Exp(T_Const(0));
	T_stm begintail = result;
	Temp_tempList calleesaves = F_calleesaves();
	for(Temp_tempList tl = calleesaves; tl; tl = tl->tail) {
		F_access new = F_allocLocal(frame, FALSE);
		T_stm m = T_Seq(T_Move(F_exp(new, NULL), T_Temp(tl->head)), NULL);
		begintail->u.SEQ.right = m;
		begintail = m;
		after = T_Seq(T_Move(T_Temp(tl->head), F_exp(new, NULL)), after);
	}
	begintail->u.SEQ.right = T_Seq(stm, after);
	return result;    //////
}

AS_instrList F_procEntryExit2(AS_instrList body) {    /////
	static Temp_tempList returnSink = NULL;
	if(!returnSink) {
		returnSink = Temp_TempList(F_SP(), Temp_TempList(F_FP(), F_calleesaves()));
	}
	return AS_splice(body, AS_InstrList(AS_Oper("", NULL, returnSink, NULL), NULL));
}

AS_proc F_procEntryExit3(F_frame frame, AS_instrList body) {    //////
	char prolog[100];
	char epilog[100];
	char buf[100];
	string procName = Temp_labelstring(F_name(frame));
	sprintf(prolog, ".text\n.globl %s\n.type %s, @function\n%s:\n", procName, procName, procName);
	sprintf(epilog, "\n");
	AS_instr a1 = AS_Oper("pushl `s0", Temp_TempList(F_SP(), NULL), Temp_TempList(F_FP(), Temp_TempList(F_SP(), NULL)), NULL);
	AS_instr a2 = AS_Oper("movl `s0, `d0", Temp_TempList(F_FP(), NULL), Temp_TempList(F_SP(), NULL), NULL);
	AS_instr a3 = NULL;
	if(frame->length > 0) {
		sprintf(buf, "subl $%d, `d0", frame->length * F_wordSize);
		a3 = AS_Oper(String(buf), Temp_TempList(F_SP(), NULL), Temp_TempList(F_SP(), NULL), NULL);
	}
	AS_instr a4 = AS_Oper("leave", NULL, NULL, NULL);
	AS_instr a5 = AS_Oper("ret", NULL, NULL, NULL);
	AS_instrList il = body;
	if(il == NULL) {
		il = AS_InstrList(AS_Oper("nop", NULL, NULL, NULL), NULL);
	}
	while(il->tail != NULL) {
		il = il->tail;
	}
	body = AS_InstrList(a1, AS_InstrList(a2, (a3 != NULL)? AS_InstrList(a3, body) : body));
	il->tail = AS_InstrList(a4, AS_InstrList(a5, NULL));
	return AS_Proc(String(prolog), body, String(epilog));
}
