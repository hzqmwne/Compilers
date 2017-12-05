
/*Lab5: This header file is not complete. Please finish it with more definition.*/

#ifndef FRAME_H
#define FRAME_H

#include "tree.h"
#include "assem.h"

typedef struct F_frame_ *F_frame;

typedef struct F_access_ *F_access;

//varibales
struct F_access_ {
	enum {inFrame, inReg} kind;
	union {
		int offset; //inFrame
		Temp_temp reg; //inReg
	} u;
};


typedef struct F_accessList_ *F_accessList;

struct F_accessList_ {F_access head; F_accessList tail;};


/* declaration for fragments */
typedef struct F_frag_ *F_frag;
struct F_frag_ {enum {F_stringFrag, F_procFrag} kind;
			union {
				struct {Temp_label label; string str;} stringg;
				struct {T_stm body; F_frame frame;} proc;
			} u;
};

F_frag F_StringFrag(Temp_label label, string str);
F_frag F_ProcFrag(T_stm body, F_frame frame);

typedef struct F_fragList_ *F_fragList;
struct F_fragList_ 
{
	F_frag head; 
	F_fragList tail;
};

F_fragList F_FragList(F_frag head, F_fragList tail);

/* ============================================================================ */

F_accessList F_AccessList(F_access head, F_accessList tail);

extern Temp_map F_tempMap;
Temp_map F_initial();
Temp_tempList F_registers(void);
T_exp F_exp(F_access acc, T_exp framePtr);
F_access F_allocLocal(F_frame f, bool escape);
F_accessList F_formals(F_frame f);
Temp_label F_name(F_frame f);

extern const int F_wordSize;
Temp_temp F_SP(void);    // stack pointer
Temp_temp F_FP(void);    // frame pointer
Temp_temp F_RV(void);    // for return value and idivl
Temp_temp F_DX(void);    // for idivl
Temp_tempList F_callersaves();
Temp_tempList F_calleesaves();
Temp_tempList F_specialregs();

F_frame F_newFrame(Temp_label name, U_boolList formals);
T_exp F_externalCall(string s, T_expList args);

T_stm F_procEntryExit1(F_frame frame, T_stm stm);
AS_instrList F_procEntryExit2(AS_instrList body);
AS_proc F_procEntryExit3(F_frame frame, AS_instrList body);

F_access F_staticLinkFormal(F_frame f);

#endif
