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

//varibales
struct F_access_ {
	enum {inFrame, inReg} kind;
	union {
		int offset; //inFrame
		Temp_temp reg; //inReg
	} u;
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

Temp_temp F_FP(void) {
	return NULL;   //////
}

Temp_temp F_RV(void);

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
	return stm;    //////
}
