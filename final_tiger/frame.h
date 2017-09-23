#ifndef FRAME_H
#define FRAME_H

typedef struct F_frame_ *F_frame;
typedef struct F_frame_ *F_frame;
typedef struct F_access_ *F_access;

typedef struct F_accessList_ *F_accessList;
struct F_accessList_ {F_access head; F_accessList tail;};

Temp_map F_tempMap;

F_accessList F_AccessList(F_access head, F_accessList tail);

F_frame F_newFrame(Temp_label name, U_boolList formals);
Temp_label F_name(F_frame f);
F_accessList F_formals(F_frame f);
F_access F_allocLocal(F_frame f, bool escape);

void F_initialize(F_frame f, Temp_map tempMap);

void F_initRegList(F_frame f, Temp_map tempMap);

//the size of frame
int F_size(F_frame f);

//In the design of this compiler, special registers is also stored in a temporary.
Temp_temp F_FP();
Temp_temp F_SP();
Temp_temp F_RV();

//for edx (division)
Temp_temp F_DX(F_frame f);

Temp_tempList F_callersaves(F_frame f);
Temp_tempList F_calleesaves(F_frame f);
Temp_tempList F_specialregs(F_frame f);

extern const int F_wordSize;
T_exp F_Exp(F_access acc, T_exp framePtr);

T_exp F_externalCall(string s, T_expList args);
T_stm F_Funcpre();
T_stm F_Funcepi();
T_exp F_Access2Tree(F_access acc);

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
struct F_fragList_ {F_frag head; F_fragList tail;};
F_fragList F_FragList(F_frag head, F_fragList tail);

T_stm F_procEntryExit1(F_frame frame, T_stm stm);
AS_proc F_procEntryExit3(F_frame f, AS_instrList body);

#endif
