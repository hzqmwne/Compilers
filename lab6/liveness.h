#ifndef LIVENESS_H
#define LIVENESS_H

/*
typedef struct Live_moveList_ *Live_moveList;
struct Live_moveList_ {
	G_node src, dst;
	Live_moveList tail;
};

Live_moveList Live_MoveList(G_node src, G_node dst, Live_moveList tail);
*/

typedef struct Live_move_ *Live_move;
struct Live_move_ {
	G_node src;
	G_node dst;
	AS_instr as;
};
Live_move Live_Move(G_node src, G_node dst, AS_instr as);

typedef struct Live_moveList_ *Live_moveList;
struct Live_moveList_ {
	Live_move head;
	Live_moveList tail;
};
Live_moveList Live_MoveList(Live_move move, Live_moveList tail);
bool Live_inMoveList(Live_move one, Live_moveList list);
Live_moveList Live_removeOne(Live_move one, Live_moveList list);
/*
Live_moveBiList Live_unionToFirst(Live_moveBiList first, Live_moveBiList second);   // first can't be used after this function!
Live_moveBiList Live_getIntersection(Live_moveBiList first, Live_moveBiList second);
*/

typedef struct Live_additionalInfo_ *Live_additionalInfo;
struct Live_additionalInfo_ {
	int index;
	G_nodeList adjList;
	int degree;
	G_node alias;
	Temp_temp color;
	Live_moveList moveList;
};
Live_additionalInfo Live_getAdditionalInfo(G_table table, G_node n);

struct Live_graph {
	G_graph graph;
	Live_moveList moves;
	bool **adjSet;
	G_table table;
};
Temp_temp Live_gtemp(G_node n);

void addEdge(bool **adjSet, G_node u, G_node v, G_table table);

struct Live_graph Live_liveness(G_graph flow);

#endif
