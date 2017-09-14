#include "util.h"
#include "slp.h"

// a = 5 + 3; b = (print(a, a-1), 10*a); print b;
A_stm prog(void) {
	// a = 5 + 3; 
	A_stm assStm1 = A_AssignStm("a", A_OpExp(A_NumExp(5), A_plus, A_NumExp(3)));

	// b = (print(a, a-1), 10*a);
	A_exp opExp1 = A_OpExp(A_IdExp("a"), A_minus, A_NumExp(1));
	A_stm prStm1 = A_PrintStm(A_PairExpList(A_IdExp("a"), A_LastExpList(opExp1)));
	A_exp opExp2 = A_OpExp(A_NumExp(10), A_times, A_IdExp("a"));
	A_stm assStm2 = A_AssignStm("b", A_EseqExp(prStm1, opExp2));

	// print b;
	A_stm prStm2 = A_PrintStm(A_LastExpList(A_IdExp("b")));

	// b = (print(a, a-1), 10*a); print b;
	A_stm comStm = A_CompoundStm(assStm2, prStm2);

	return A_CompoundStm(assStm1, comStm);
}

// a = 5 + 3; b = (print(a, a-1), 10*a); print b;
// a = 5 + b; b = (print(a, a, a-1), 10*a); print b;
A_stm prog_prog(void)
{
	// a = 5 + 3; b = (print(a, a-1), 10*a); print b;
    A_stm stm1 = prog();

	// a = 5 + b;
	A_stm assStm1 = A_AssignStm("a", A_OpExp(A_NumExp(5), A_plus, A_IdExp("b")));

	// print(a, a, a-1)
	A_exp exp1 = A_OpExp(A_IdExp("a"), A_minus, A_NumExp(1));
	A_expList explist = A_PairExpList(A_IdExp("a"),
							A_PairExpList(A_IdExp("a"), 
								A_LastExpList(exp1)));
	A_stm prStm1 = A_PrintStm(explist);
	// 10 * a
	A_exp exp2 = A_OpExp(A_NumExp(10), A_times, A_IdExp("a"));

	// b = (print(a, a, a-1), 10*a);
	A_stm assStm2 = A_AssignStm("b", A_EseqExp(A_PrintStm(explist), exp2));

	// print b;
	A_stm prStm2 = A_PrintStm(A_LastExpList(A_IdExp("b")));

     return A_CompoundStm(stm1, 
			 A_CompoundStm(assStm1, 
				 A_CompoundStm(assStm2, prStm2)));
}

// a = 5 + 3; b = (print(a, a-1), 10*a); print b;
// a = 5 + b; b = (print(a, a, a-1), 10*a); print b;
// a = (a = a+b, a);
A_stm right_prog(void)
{
    A_stm stm1 = prog_prog();
     return A_CompoundStm( stm1, A_AssignStm("a",
					A_EseqExp(A_AssignStm("a", 
							A_OpExp(A_IdExp("a"), A_plus, A_IdExp("b"))), A_IdExp("a"))));
 
}
