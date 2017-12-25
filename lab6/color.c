#include <stdio.h>
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


static G_table additionalNodeTable = NULL;

static int K = 0;    // conut of machine registers

static G_nodeList simplifyWorklist = NULL;
static G_nodeList freezeWorklist = NULL;
static G_nodeList spillWorklist = NULL;

static G_nodeList spilledNodes = NULL;
static G_nodeList coalescedNodes = NULL;
static G_nodeList coloredNodes = NULL;

static G_nodeList selectStack = NULL;

static Live_moveList worklistMoves = NULL;    // = return value from liveness.c
static Live_moveList coalescedMoves = NULL;
static Live_moveList constrainedMoves = NULL;
static Live_moveList frozenMoves = NULL;
static Live_moveList activeMoves = NULL;

static bool **adjSet = NULL;    // the extra infomation for Temp node

/* ================================================= */

static Live_moveList nodeMoves(G_node n) {
	Live_moveList list = NULL;
	Live_additionalInfo info = Live_getAdditionalInfo(additionalNodeTable, n);
	Live_moveList ml;
	for(ml = info->moveList; ml; ml = ml->tail) {
		if(Live_inMoveList(ml->head, activeMoves) || Live_inMoveList(ml->head, worklistMoves)) {
			list = Live_MoveList(ml->head, list);
		}
	}
	return list;
}

static bool moveRelated(G_node n) {
	// return (nodeMoves(n) != NULL);
	Live_additionalInfo info = Live_getAdditionalInfo(additionalNodeTable, n);
	Live_moveList ml;
	for(ml = info->moveList; ml; ml = ml->tail) {
		if(Live_inMoveList(ml->head, activeMoves) || Live_inMoveList(ml->head, worklistMoves)) {
			return TRUE;
		}
	}
	return FALSE;
}

static void makeWorklist(G_nodeList nodes) {
	G_nodeList nl;
	for(nl = nodes; nl; nl = nl->tail) {
		Live_additionalInfo info = Live_getAdditionalInfo(additionalNodeTable, nl->head);
		if(info->color == NULL) {    // not precolored
			if(info->degree >= K) {
				spillWorklist = G_NodeList(nl->head, spillWorklist);
			}
			else if(moveRelated(nl->head)) {    // move related
				freezeWorklist = G_NodeList(nl->head, freezeWorklist);
			}
			else {
				simplifyWorklist = G_NodeList(nl->head, simplifyWorklist);
			}
		}
	}
}

static G_nodeList adjacent(G_node n) {
	G_nodeList list = NULL;
	Live_additionalInfo info = Live_getAdditionalInfo(additionalNodeTable, n);
	G_nodeList nl;
	for(nl = info->adjList; nl; nl = nl->tail) {
		if(!G_inNodeList(nl->head, selectStack) && !G_inNodeList(nl->head, coalescedNodes)) {
			list = G_NodeList(nl->head, list);
		}
	}
	return list;
}

/* =========== */

static void enableMoves(G_nodeList nodes) {
	G_nodeList nl;
	for(nl = nodes; nl; nl = nl->tail) {
		Live_moveList ml;
		for(ml = nodeMoves(nl->head); ml; ml = ml->tail) {
			if(Live_inMoveList(ml->head, activeMoves)) {
				activeMoves = Live_removeOne(ml->head, activeMoves);
				worklistMoves = Live_MoveList(ml->head, worklistMoves);
			}
		}
	}
}

static void decrementDegree(G_node m) {
	Live_additionalInfo info = Live_getAdditionalInfo(additionalNodeTable, m);
	int d = info->degree;
	info->degree = d - 1;
	if(d == K) {
		enableMoves(G_NodeList(m, adjacent(m)));
		spillWorklist = G_removeOne(m, spillWorklist);
		if(moveRelated(m)) {
			freezeWorklist = G_NodeList(m, freezeWorklist);
		}
		else {
			simplifyWorklist = G_NodeList(m, simplifyWorklist);
		}
	}
}

static void simplify() {
	while(simplifyWorklist != NULL) {
		G_node n = simplifyWorklist->head;
		simplifyWorklist = simplifyWorklist->tail;
		selectStack = G_NodeList(n, selectStack);
		G_nodeList nl;
		for(nl = adjacent(n); nl; nl = nl->tail) {
			decrementDegree(nl->head);
		}
	}
}

/* =========== */

static void addWorkList(G_node u) {
	Live_additionalInfo u_info = Live_getAdditionalInfo(additionalNodeTable, u);
	if(u_info->color == NULL && !(moveRelated(u)) && u_info->degree < K) {
		freezeWorklist = G_removeOne(u, freezeWorklist);
		simplifyWorklist = G_NodeList(u, simplifyWorklist);
	}
}

static bool george(G_node u, G_node v) {
	Live_additionalInfo u_info = Live_getAdditionalInfo(additionalNodeTable, u);
	G_nodeList nl;
	for(nl = adjacent(v); nl; nl = nl->tail) {
		G_node t = nl->head; 
		Live_additionalInfo t_info = Live_getAdditionalInfo(additionalNodeTable, t);
		if(!(t_info->degree < K || t_info->color != NULL || adjSet[t_info->index][u_info->index])) {
			return FALSE;
		}
	}
	return TRUE;
}

static bool briggs(G_node u, G_node v) {
	G_nodeList adju = adjacent(u);
	G_nodeList nodes = adju;
	G_nodeList nl;
	for(nl = adjacent(v); nl; nl = nl->tail) {
		if(!G_inNodeList(nl->head, adju)) {
			nodes = G_NodeList(nl->head, nodes);
		}
	}
	
	int k = 0;
	for(nl = nodes; nl; nl = nl->tail) {
		G_node n = nl->head;
		Live_additionalInfo n_info = Live_getAdditionalInfo(additionalNodeTable, n);
		if(n_info->degree >= K) {
			k = k + 1;
		}
	}
	return (k < K);
}

static G_node getAlias(G_node n) {
	if(G_inNodeList(n, coalescedNodes)) {
		Live_additionalInfo n_info = Live_getAdditionalInfo(additionalNodeTable, n);
		return getAlias(n_info->alias);
	}
	else {
		return n;
	}
}

static void combine(G_node u, G_node v) {
	Live_additionalInfo u_info = Live_getAdditionalInfo(additionalNodeTable, u);
	Live_additionalInfo v_info = Live_getAdditionalInfo(additionalNodeTable, v);
	if(G_inNodeList(v, freezeWorklist)) {
		freezeWorklist = G_removeOne(v, freezeWorklist);
	}
	else {
		spillWorklist = G_removeOne(v, spillWorklist);
	}
	coalescedNodes = G_NodeList(v, coalescedNodes);
	v_info->alias = u;
	Live_moveList new = u_info->moveList;
	Live_moveList ml;
	for(ml = v_info->moveList; ml; ml = ml->tail) {
		if(!Live_inMoveList(ml->head, u_info->moveList)) {
			new = Live_MoveList(ml->head, new);
		}
	}
	u_info->moveList = new;
	enableMoves(G_NodeList(v, NULL));
	G_nodeList nl;
	for(nl = adjacent(v); nl; nl = nl->tail) {
		G_node t = nl->head;
		addEdge(adjSet, t, u, additionalNodeTable);    // addEdge is defined in liveness.c
		decrementDegree(t);
	}
	if(u_info->degree >= K && G_inNodeList(u, freezeWorklist)) {
		freezeWorklist = G_removeOne(u, freezeWorklist);
		spillWorklist = G_NodeList(u, spillWorklist);
	}
}

static void coalesce() {
	while(worklistMoves != NULL) {
		Live_move m = worklistMoves->head;
		G_node x = m->dst;
		G_node y = m->src;
		x = getAlias(x);
		y = getAlias(y);
		Live_additionalInfo x_info = Live_getAdditionalInfo(additionalNodeTable, x);
		Live_additionalInfo y_info = Live_getAdditionalInfo(additionalNodeTable, y);
		G_node u;
		G_node v;
		Live_additionalInfo u_info;
		Live_additionalInfo v_info;
		if(y_info->color != NULL) {    // precolored
			u = y;
			u_info = y_info;
			v = x;
			v_info = x_info;
		}
		else {
			u = x;
			u_info = x_info;
			v = y;
			v_info = y_info;
		}
		worklistMoves = Live_removeOne(m, worklistMoves);
		if(u == v) {    // already coalesced
			coalescedMoves = Live_MoveList(m, coalescedMoves);
			addWorkList(u);
		}
		else if(v_info->color != NULL || adjSet[u_info->index][v_info->index]) {
			constrainedMoves = Live_MoveList(m, constrainedMoves);
			addWorkList(u);
			addWorkList(v);
		}
		else {
			if(((u_info->color != NULL) && george(u, v)) || ((u_info->color == NULL) && briggs(u, v))) {
				coalescedMoves = Live_MoveList(m, coalescedMoves);
				combine(u, v);
				addWorkList(u);
			}
			else {
				activeMoves = Live_MoveList(m, activeMoves);
			}
		}
	}
}

/* =========== */

static void freezeMoves(G_node u) {
	Live_moveList ml;
	for(ml = nodeMoves(u); ml; ml = ml->tail) {
		Live_move m = ml->head;
		G_node x = m->dst;
		G_node y = m->src;
		G_node aliasy = getAlias(y);
		G_node v;
		if(aliasy == getAlias(u)) {
			v = getAlias(x);
		}
		else {
			v = aliasy;
		}
		activeMoves = Live_removeOne(m ,activeMoves);
		frozenMoves = Live_MoveList(m, frozenMoves);
		Live_additionalInfo v_info = Live_getAdditionalInfo(additionalNodeTable, v);
		if(!moveRelated(v) && v_info->degree < K) {
			freezeWorklist = G_removeOne(v, freezeWorklist);
			simplifyWorklist = G_NodeList(v, simplifyWorklist);
		}
	}
}

static void freeze() {
	G_node u = freezeWorklist->head;
	freezeWorklist = G_removeOne(u, freezeWorklist);
	simplifyWorklist = G_NodeList(u, simplifyWorklist);
	freezeMoves(u);
}

/* =========== */

static void selectSpill() {
	G_node m = NULL;
	int maxDegree = -1;
	G_nodeList nl;
	for(nl = spillWorklist; nl; nl = nl->tail) {
		Live_additionalInfo info = Live_getAdditionalInfo(additionalNodeTable, nl->head);
		if(info->degree >= maxDegree) {
			m = nl->head;
			maxDegree = info->degree;
		}
	}
	spillWorklist = G_removeOne(m, spillWorklist);
	simplifyWorklist = G_NodeList(m, simplifyWorklist);
	freezeMoves(m);
}

static Temp_tempList copyTempList(Temp_tempList tl) {
	return (tl != NULL)? Temp_TempList(tl->head, copyTempList(tl->tail)) : NULL;
}
static void assignColors(Temp_tempList colors) {
	while(selectStack != NULL) {
		G_node n = selectStack->head;
		selectStack = G_removeOne(n, selectStack);
		Live_additionalInfo n_info = Live_getAdditionalInfo(additionalNodeTable, n);
		Temp_tempList okColors = copyTempList(colors);
		G_nodeList nl;
		for(nl = n_info->adjList; nl; nl = nl->tail) {
			G_node w = getAlias(nl->head);
			Live_additionalInfo w_info = Live_getAdditionalInfo(additionalNodeTable, w);
			if(w_info->color != NULL) {
				okColors = Temp_removeOne(w_info->color, okColors);
			}
		}
		if(okColors == NULL) {
			spilledNodes = G_NodeList(n, spilledNodes);
		}
		else {
			coloredNodes = G_NodeList(n, coloredNodes);
			n_info->color = okColors->head;
		}
	}
	G_nodeList nl;
	for(nl = coalescedNodes; nl; nl = nl->tail) {
		Live_additionalInfo info = Live_getAdditionalInfo(additionalNodeTable, nl->head);
		G_node alias = getAlias(nl->head);
		Live_additionalInfo aliasinfo = Live_getAdditionalInfo(additionalNodeTable, alias);
		coloredNodes = G_NodeList(nl->head, coloredNodes);
		info->color = aliasinfo->color;
	}
}

/* ================================================= */

/*
void printNodeList(G_nodeList nl, string s) {
	printf("%s ", s);
	G_nodeList il;
	for(il = nl; il; il = il->tail) {
		Live_additionalInfo info = Live_getAdditionalInfo(additionalNodeTable, il->head);
		printf("%d:%d ", Temp_int(Live_gtemp(il->head)), info->degree);
	}
	printf("\n");
}
void printFourNodeLists() {
	printNodeList(simplifyWorklist, "simplifyWorklist");
	printNodeList(freezeWorklist, "freezeWorklist");
	printNodeList(spillWorklist, "spillWorklist");
	printNodeList(selectStack, "selectStack");
	printNodeList(coalescedNodes, "coalescedNodes");
	printNodeList(spilledNodes, "spilledNodes");
	printf("\n");
}
*/

struct COL_result COL_color(G_graph ig, Temp_map initial, Temp_tempList regs, Live_moveList moves, bool **set, G_table table) {
	//your code here.
	
	simplifyWorklist = NULL;
	freezeWorklist = NULL;
	spillWorklist = NULL;

	coalescedNodes = NULL;
	coloredNodes = NULL;
	spilledNodes = NULL;

	selectStack = NULL;

	coalescedMoves = NULL;
	constrainedMoves = NULL;
	activeMoves = NULL;
	frozenMoves = NULL;
	worklistMoves = moves;

	additionalNodeTable = table;
	adjSet = set;
	K = 0;
	Temp_tempList tl;
	for(tl = regs; tl; tl = tl->tail) {
		++K;
	}
	assert(K == 8);    // in x86

	makeWorklist(G_nodes(ig));
	//printf("\n=====start======\n");
	//printFourNodeLists();
	do {
		if(simplifyWorklist != NULL) {
			simplify();
			//printf("after simplify\n");
			//printFourNodeLists();
		}
		else if(worklistMoves != NULL) {
			coalesce();
			//printf("after coalesce\n");
			//printFourNodeLists();
		}
		else if(freezeWorklist != NULL) {
			freeze();
			//printf("after freeze\n");
			//printFourNodeLists();
		}
		else if(spillWorklist != NULL) {
			selectSpill();
			//printf("after selectSpill\n");
			//printFourNodeLists();
		}
		//printf("%d\n", 5);
	} while(!(simplifyWorklist == NULL && worklistMoves == NULL && freezeWorklist == NULL && spillWorklist == NULL));
	assignColors(regs);
	//printNodeList(coloredNodes, "coloredNodes");
	//printNodeList(spilledNodes, "spilledNodes");

	struct COL_result ret;
	Temp_tempList spills = NULL;
	G_nodeList nl;
	for(nl = spilledNodes; nl; nl = nl->tail) {
		spills = Temp_TempList(Live_gtemp(nl->head), spills);
	}
	if(spills != NULL) {
		TAB_table tempAlias = TAB_empty();
		G_nodeList nl;
		for(nl = coalescedNodes; nl; nl = nl->tail) {
			TAB_enter(tempAlias, G_nodeInfo(nl->head), G_nodeInfo(getAlias(nl->head)));
		}
		ret.coloring = NULL;
		ret.spills = spills;
		ret.tempAlias = tempAlias;
	}
	else {
		Temp_map coloring = Temp_empty();
		G_nodeList nl;
		for(nl = coloredNodes; nl; nl = nl->tail) {
			Live_additionalInfo info = Live_getAdditionalInfo(additionalNodeTable, nl->head);
			Temp_enter(coloring, Live_gtemp(nl->head), Temp_look(initial, info->color));
			//printf("====debug==== %d %s\n", Temp_int(Live_gtemp(nl->head)), Temp_look(initial, info->color));
		}
		ret.coloring = coloring;
		ret.spills = NULL;
		ret.tempAlias = NULL;
	}
	ret.coalescedMoves = coalescedMoves;
	return ret;
}
