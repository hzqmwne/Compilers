CC=gcc
CFLAGS=-g -c -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast

tiger-compiler:  main.o y.tab.o lex.yy.o errormsg.o absyn.o prabsyn.o symbol.o translate.o semant.o parse.o escape.o frame.o table.o util.o types.o env.o tree.o temp.o printtree.o canon.o codegen.o assem.o graph.o flowgraph.o liveness.o color.o regalloc.o
	$(CC) -g $^ -o $@

lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) $<

lex.yy.c:
	lex tiger.lex

clean:
	rm -f tiger-compiler lex.yy.c lex.yy.o
