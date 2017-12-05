#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "util.h"
#include "absyn.h"
#include "temp.h"
#include "frame.h"

/* Lab5: your code below */

typedef struct Tr_exp_ *Tr_exp;

typedef struct Tr_expList_ *Tr_expList;

typedef struct Tr_access_ *Tr_access;

typedef struct Tr_accessList_ *Tr_accessList;

typedef struct Tr_level_ *Tr_level;

typedef struct Tr_trExpList_ *Tr_trExpList;

struct Tr_accessList_ {
	Tr_access head;
	Tr_accessList tail;	
};

Tr_trExpList Tr_TrExpList(Tr_exp head, Tr_trExpList tail);

Tr_access Tr_Access(Tr_level level, F_access access);
Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail);

Tr_level Tr_outermost(void);

Tr_level Tr_newLevel(Tr_level parent, Temp_label name, U_boolList formals);

Tr_accessList Tr_formals(Tr_level level);

Tr_access Tr_allocLocal(Tr_level level, bool escape);

Tr_exp Tr_simpleVar(Tr_access acc, Tr_level lv);
Tr_exp Tr_fieldVar(Tr_exp host, int cnt);
Tr_exp Tr_subscriptVar(Tr_exp host, Tr_exp idx);
Tr_exp Tr_nop();
Tr_exp Tr_intExp(int v);
Tr_exp Tr_strExp(string str);
Tr_exp Tr_opExp(Tr_exp left, Tr_exp right, A_oper oper);
Tr_exp Tr_relOpExp(Tr_exp left, Tr_exp right, A_oper oper);
Tr_exp Tr_stringEq(Tr_exp left, Tr_exp right);
Tr_exp Tr_ifExp(Tr_exp condp, Tr_exp thenp, Tr_exp elsep, bool isVoid, Tr_level current);
Tr_exp Tr_whileExp(Tr_exp condp, Tr_exp bodyExp, Temp_label done);
Tr_exp Tr_forExp(Tr_access loopVar, Tr_exp lo, Tr_exp hi, Tr_exp bodyExp, Temp_label done, Tr_level current);
Tr_exp Tr_asnExp(Tr_exp leftExp, Tr_exp rightExp);
//Tr_exp Tr_callExp(A_expList args, Temp_label name, Temp_label);
Tr_exp Tr_callExp(Temp_label name, Tr_trExpList reserveOrderArgs, Tr_level current, Tr_level declare);
//Tr_exp Tr_insertExp(Tr_exp original, Tr_exp newExp);
//Tr_exp Tr_expList(Tr_exp original, Tr_exp newExp);
Tr_exp Tr_seqExp(Tr_trExpList reserveOrderTrExps);
//Tr_exp Tr_callInit(Temp_label name);
//Tr_exp Tr_addArg(Tr_exp call, Tr_exp arg);
//Tr_exp Tr_staticLink(Tr_exp call, Tr_level caller, Tr_level callee);
//Tr_exp Tr_recoverStack(Tr_exp call, int argCnt);
//Tr_exp Tr_recordExp(Tr_exp args);
Tr_exp Tr_recordExp(Tr_trExpList reserveOrderArgs, int argCnt, Tr_level current);
Tr_exp Tr_arrayExp(Tr_exp size, Tr_exp init);
Tr_exp Tr_breakExp(Temp_label target);
Tr_exp Tr_initExp(Tr_access var, Tr_exp init);
Tr_exp Tr_funcDec(Tr_exp body, Tr_level lv);

void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals);
F_fragList Tr_getResult(void);

#endif
