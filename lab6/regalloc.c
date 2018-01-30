#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "table.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "liveness.h"
#include "color.h"
#include "regalloc.h"
#include "flowgraph.h"

static AS_instrList rewriteProgram(F_frame f, AS_instrList il, Temp_tempList spills, TAB_table tempAlias) {
	char as_buf[100];
	il = AS_InstrList(NULL, il);
	Temp_tempList tl;
	AS_instrList now = il->tail;
	AS_instrList prev = il;
	while(now) {
		AS_instr as = now->head;
		for(tl = AS_src(as); tl; tl = tl->tail) {    // replace alias
			Temp_temp t = TAB_look(tempAlias, tl->head);
			if(t != NULL) {
				tl->head = t;
			}
		}
		for(tl = AS_dst(as); tl; tl = tl->tail) {
			Temp_temp t = TAB_look(tempAlias, tl->head);
			if(t != NULL) {
				tl->head = t;
			}
		}
		if(as->kind == I_MOVE && as->u.MOVE.src->head == as->u.MOVE.dst->head) {    // if I_MOVE has the same src and dst, delete it
			prev->tail = now->tail;
			now = now->tail;
		}
		else {
			prev = now;
			now = now->tail;
		}
	}
	for(tl = spills; tl; tl = tl->tail) {
		F_access newAcc = F_allocLocal(f, TRUE);
		int offset = newAcc->u.offset;
		AS_instrList now = il->tail;
		AS_instrList prev = il;
		for(; now; prev = now, now = now->tail) {
			Temp_tempList src;
			for(src = AS_src(now->head); src; src = src->tail) {    // The AS_src list shouldn't have repeating elements
				if(src->head == tl->head) {    // so this condition shouldn't be true more than once in the for loop
					Temp_temp temp = Temp_newtemp();
					sprintf(as_buf, "movl %d(`s0), `d0", offset);
					AS_instr load = AS_Oper(String(as_buf), Temp_TempList(temp, NULL), Temp_TempList(F_FP(), NULL), NULL);
					src->head = temp;
					prev->tail = AS_InstrList(load, now);
					prev = prev->tail;
				}
			}
			Temp_tempList dst;
			for(dst = AS_dst(now->head); dst; dst = dst->tail) {
				if(dst->head == tl->head) {
					Temp_temp temp = Temp_newtemp();
					sprintf(as_buf, "movl `s1, %d(`s0)", offset);
					AS_instr store = AS_Oper(String(as_buf), NULL, Temp_TempList(F_FP(), Temp_TempList(temp, NULL)), NULL);
					dst->head = temp;
					prev = now;
					now->tail = AS_InstrList(store, now->tail);
					now = now->tail;
				}
			}
		}
	}
	il = il->tail;
	return il;
}

static AS_instrList deleteInstrs(AS_instrList il, Live_moveList coalescedMoves) {
	// for all AS_instr in coalescedMoves, set its src and dst to null
	// then for all AS_instr in il, if it is move and src and dst is null, delete it
	Live_moveList ml;
	for(ml = coalescedMoves; ml; ml = ml->tail) {
		AS_instr as = ml->head->as;
		assert(as->kind == I_MOVE);
		as->u.MOVE.assem = "";
	}
	il = AS_InstrList(NULL, il);
	AS_instrList now = il->tail;
	AS_instrList prev = il;
	while(now) {
		AS_instr as = now->head;
		string assem = AS_assem(as);
		if(strlen(assem) == 0) {    // not only for coalesed moves, but also for the last instr added in F_procEntryExit2
			prev->tail = now->tail;
			now = now->tail;
		}
		else {
			prev = now;
			now = now->tail;
		}
	}
	il = il->tail;
	return il;
}

struct RA_result RA_regAlloc(F_frame f, AS_instrList il) {
	//your code here
	
	struct RA_result ret;
	
	while(TRUE) {
		G_graph flow = FG_AssemFlowGraph(il, f);
		struct Live_graph liveness = Live_liveness(flow);
		struct COL_result colResult = COL_color(liveness.graph, F_initial(), F_registers(), liveness.moves, liveness.adjSet, liveness.table);
		if(colResult.spills != NULL) {
			il = rewriteProgram(f, il, colResult.spills, colResult.tempAlias);
			//AS_printInstrList(stdout, il, Temp_layerMap(F_tempMap, Temp_name()));
		}
		else {
			il = deleteInstrs(il, colResult.coalescedMoves);
			//AS_printInstrList(stdout, il, Temp_layerMap(F_tempMap, Temp_name()));
			ret.coloring = colResult.coloring;
			ret.il = il;
			break;
		}
	}
	return ret;
}
