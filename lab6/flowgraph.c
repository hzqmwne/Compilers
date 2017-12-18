#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "absyn.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "flowgraph.h"
#include "errormsg.h"
#include "table.h"

AS_instr FG_as(G_node n) {
	return (AS_instr)(G_nodeInfo(n));
}

Temp_tempList FG_def(G_node n) {
	//your code here.
	AS_instr a = (AS_instr)(G_nodeInfo(n));
	if(a->kind == I_OPER) {
		return a->u.OPER.dst;
	}
	else if(a->kind == I_MOVE) {
		return a->u.MOVE.dst;
	}
	return NULL;
}

Temp_tempList FG_use(G_node n) {
	//your code here.
	AS_instr a = (AS_instr)(G_nodeInfo(n));
	if(a->kind == I_OPER) {
		return a->u.OPER.src;
	}
	else if(a->kind == I_MOVE) {
		return a->u.MOVE.src;
	}
	return NULL;
}

bool FG_isMove(G_node n) {
	//your code here.
	AS_instr a = (AS_instr)(G_nodeInfo(n));
	return a->kind == I_MOVE;
}

static TAB_table instrToNode = NULL;
static TAB_table labelToNode = NULL;
static G_node getNodeByASinstr(G_nodeList nl, AS_instr as) {
	return (G_node)TAB_look(instrToNode, as);
}
static G_node getNodeByTempLabel(G_nodeList nl, Temp_label l) {
	return (G_node)TAB_look(labelToNode, l);
}

G_graph FG_AssemFlowGraph(AS_instrList il, F_frame f) {
	//your code here.
	instrToNode = G_empty();
	labelToNode = G_empty();
	G_graph flowGraph = G_Graph();
	AS_instrList insli;
	for(insli = il; insli; insli = insli->tail) {
		AS_instr a = insli->head;
		G_node newNode = G_Node(flowGraph, insli->head);
		TAB_enter(instrToNode, a, newNode);
		if(a->kind == I_LABEL && a->u.LABEL.label != NULL) {
			TAB_enter(labelToNode, a->u.LABEL.label, newNode);
		}
	}

	G_nodeList nodes = G_nodes(flowGraph);
	for(insli = il; insli; insli = insli->tail) {
		AS_instr a = insli->head;
		if(a->kind == I_OPER && a->u.OPER.jumps != NULL) {
			Temp_labelList labels;
			for(labels = a->u.OPER.jumps->labels; labels; labels = labels->tail) {
				G_addEdge(getNodeByASinstr(nodes, a), getNodeByTempLabel(nodes, labels->head));
			}
		}
		else if(insli->tail != NULL) {
			G_addEdge(getNodeByASinstr(nodes, a), getNodeByASinstr(nodes, insli->tail->head));
		}
	}
	return flowGraph;
}
