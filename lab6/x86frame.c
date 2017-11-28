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

//varibales
struct F_access_ {
	enum {inFrame, inReg} kind;
	union {
		int offset; //inFrame
		Temp_temp reg; //inReg
	} u;
};

F_frag F_StringFrag(Temp_label label, string str) {   
	    return NULL;                                      
}                                                     
                                                      
F_frag F_ProcFrag(T_stm body, F_frame frame) {        
	    return NULL;                                      
}                                                     
                                                      
F_fragList F_FragList(F_frag head, F_fragList tail) { 
	    return NULL;                                      
}                                                     

