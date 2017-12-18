#include <stdio.h>
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
#include "liveness.h"
#include "table.h"

/*
Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail) {
	Live_moveList lm = (Live_moveList) checked_malloc(sizeof(*lm));
	lm->src = src;
	lm->dst = dst;
	lm->tail = tail;
	return lm;
}
*/

Live_move Live_Move(G_node src, G_node dst, AS_instr as) {
	Live_move p = (Live_move)checked_malloc(sizeof(*p));
	p->src = src;
	p->dst = dst;
	p->as = as;
	return p;
}

Live_moveList Live_MoveList(Live_move move, Live_moveList tail) {
	Live_moveList p = (Live_moveList)checked_malloc(sizeof(*p));
	p->head = move;
	p->tail = tail;
	return p;
}
bool Live_inMoveList(Live_move one, Live_moveList list) {
	for(; list; list = list->tail) {
		if(one == list->head) {
			return TRUE;
		}
	}
	return FALSE;
}
Live_moveList Live_removeOne(Live_move one, Live_moveList list) {
	Live_moveList now;
	Live_moveList prev = NULL;
	for(now = list; now; prev = now, now = now->tail) {
		if(now->head == one) {
			if(prev != NULL) {
				prev->tail = now->tail;
				free(now);
				return list;
			}
			else {
				list = now->tail;
				free(now);
				return list;
			}
		}
	}
	return list;
}


static bool *updatedLabel(G_table t, G_node flowNode) {
	return (bool *)G_look(t, flowNode);
}

static void enterLiveMap(G_table t, G_node flowNode, Temp_tempList temps) {
	G_enter(t, flowNode, temps);
}
static Temp_tempList lookupLiveMap(G_table t, G_node flowNode) {
	return (Temp_tempList)G_look(t, flowNode);
}

static G_table makeLiveMap(G_nodeList flowNodes) {
	G_table liveMap = G_empty();
	G_table updatedLabelMap = G_empty();
	G_nodeList reserveOrderFlowNodes = NULL;
	G_nodeList nl;
	bool globalUpdated;

	for(nl = flowNodes; nl; nl = nl->tail) {
		bool *p = (bool *)checked_malloc(sizeof(*p));
		*p = TRUE;
		G_enter(updatedLabelMap, nl->head, p);
		reserveOrderFlowNodes = G_NodeList(nl->head, reserveOrderFlowNodes);
	}

	do {
		globalUpdated = FALSE;
		for(nl = reserveOrderFlowNodes; nl; nl = nl->tail) {
			bool *p = updatedLabel(updatedLabelMap, nl->head);
			if(*p) {
				*p = FALSE;
				Temp_tempList out = lookupLiveMap(liveMap, nl->head);
				Temp_tempList def = FG_def(nl->head);
				Temp_tempList use = FG_use(nl->head);

				// in = use + (out - def)
				Temp_tempList in = use;
				Temp_tempList t;
				for(t = out; t; t = t->tail) {
					if(!Temp_inTempList(t->head, def) && !Temp_inTempList(t->head, use)) {
						in = Temp_TempList(t->head, in);
					}
				}

				G_nodeList prl;
				for(prl = G_pred(nl->head); prl; prl = prl->tail) {
					Temp_tempList prevOut = lookupLiveMap(liveMap, prl->head);
					Temp_tempList prevNewOut = prevOut;
					bool *prevP = updatedLabel(updatedLabelMap, prl->head);
					bool b = FALSE;
					Temp_tempList t;
					for(t = in; t; t = t->tail) {
						if(!Temp_inTempList(t->head, prevOut)) {
							b = TRUE;
							prevNewOut = Temp_TempList(t->head, prevNewOut);
						}
					}
					if(b) {
						*prevP = TRUE;
						globalUpdated = TRUE;
						enterLiveMap(liveMap, prl->head, prevNewOut);
					}
				}
			}
		}
	} while(globalUpdated);

	return liveMap;
}

/* =================== */

static TAB_table tempToNode = NULL;
static void setNodeOfTemp(Temp_temp t, G_node n) {
	TAB_enter(tempToNode, t, n);
}
static G_node getNodeByTemp(Temp_temp t) {
	return (G_node)TAB_look(tempToNode, t);
}

Temp_temp Live_gtemp(G_node n) {
	//your code here.
	return (Temp_temp)(G_nodeInfo(n));
}
Live_additionalInfo Live_getAdditionalInfo(G_table table, G_node n) {
	return (Live_additionalInfo)G_look(table, n);
}

void addEdge(bool **adjSet, G_node u, G_node v, G_table table) {
	Live_additionalInfo u_info = Live_getAdditionalInfo(table, u);
	Live_additionalInfo v_info = Live_getAdditionalInfo(table, v);
	if(!adjSet[u_info->index][v_info->index] && u != v) {
		adjSet[u_info->index][v_info->index] = TRUE;
		adjSet[v_info->index][u_info->index] = TRUE;
		if(!u_info->color) {
			u_info->adjList = G_NodeList(v, u_info->adjList);
			u_info->degree += 1;
		}
		if(!v_info->color) {
			v_info->adjList = G_NodeList(u, v_info->adjList);
			v_info->degree += 1;
		}
	}
}

struct Live_graph Live_liveness(G_graph flow) {
	//your code here.
	G_table additionalInfoTable = G_empty();
	
	G_nodeList flowNodes = G_nodes(flow);
	G_table liveMap = makeLiveMap(flowNodes);

	// make empty confilict graph
	tempToNode = TAB_empty();
	G_graph confilict = G_Graph();
	int precolorCnt = 0;
	Temp_tempList tl;
	for(tl = F_registers(); tl; tl = tl->tail) {
		++precolorCnt;
	}
	assert(precolorCnt == 8);    // in x86
	int nodeIndex = 0;
	for(tl = F_registers(); tl; tl = tl->tail) {
		// assert F_registers are different between each other
		G_node newNode= G_Node(confilict, tl->head);
		Live_additionalInfo p = calloc(1, sizeof(*p));
		p->index = nodeIndex;
		p->color = tl->head;
		p->degree = 10000;    // precolored nodes have infinite degree
		G_enter(additionalInfoTable, newNode, p);
		setNodeOfTemp(tl->head, newNode);
		++nodeIndex;
	}
	G_nodeList nl;
	for(nl = flowNodes; nl; nl = nl->tail) {
		Temp_tempList live = lookupLiveMap(liveMap, nl->head);
		Temp_tempList tl;
		for(tl = live; tl; tl = tl->tail) {
			if(getNodeByTemp(tl->head) == NULL) {
				G_node newNode= G_Node(confilict, tl->head);
				Live_additionalInfo p = calloc(1, sizeof(*p));
				p->index = nodeIndex;
				G_enter(additionalInfoTable, newNode, p);
				setNodeOfTemp(tl->head, newNode);
				++nodeIndex;
			}
		}
	}

	bool **adjSet = (bool **)calloc(nodeIndex, sizeof(bool *));
	int i;
	for(i = 0; i < nodeIndex; ++i) {
		adjSet[i] = (bool *)calloc(nodeIndex, sizeof(bool));
	}

	// edge for precolor
	for(i = 0; i < precolorCnt; ++i) {
		int j;
		for(j = 0; j < precolorCnt; ++j) {
			if(i != j) {
				adjSet[i][j] = TRUE;
			}
		}
	}

	Live_moveList worklistMoves = NULL;
	// Build()
	for(nl = flowNodes; nl; nl = nl->tail) {
		Temp_tempList live = lookupLiveMap(liveMap, nl->head);    // liveOut
		AS_instr as = FG_as(nl->head);
		Temp_tempList def = FG_def(nl->head);
		Temp_tempList use = FG_use(nl->head);
		bool isMove = FG_isMove(nl->head);
		if(isMove) {
			assert(def != NULL && def->tail == NULL && use != NULL && use->tail == NULL);
			G_node n_src = getNodeByTemp(use->head);
			Live_additionalInfo info_src = Live_getAdditionalInfo(additionalInfoTable, n_src);
			G_node n_dst = getNodeByTemp(def->head);
			Live_additionalInfo info_dst = Live_getAdditionalInfo(additionalInfoTable, n_dst);
			Live_move move = Live_Move(n_src, n_dst, as);
			info_src->moveList = Live_MoveList(move, info_src->moveList);
			info_dst->moveList = Live_MoveList(move, info_dst->moveList);
			worklistMoves = Live_MoveList(move, worklistMoves);
		}
		Temp_tempList d;
		for(d = def; d; d = d->tail) {
			G_node n_d = getNodeByTemp(d->head);
			Temp_tempList l;
			if(isMove) {
				for(l = live; l; l = l->tail) {
					if(l->head != use->head) {
						addEdge(adjSet, n_d, getNodeByTemp(l->head), additionalInfoTable);
					}
				}
			}
			else {
				for(l = live; l; l = l->tail) {
					addEdge(adjSet, n_d, getNodeByTemp(l->head), additionalInfoTable);
				}
				for(l = def; l; l = l->tail) {
					addEdge(adjSet, n_d, getNodeByTemp(l->head), additionalInfoTable);
				}
			}
		}
	}
	
	struct Live_graph lg;
	lg.graph = confilict;
	lg.moves = worklistMoves;
	lg.adjSet = adjSet;
	lg.table = additionalInfoTable;
	return lg;
}


