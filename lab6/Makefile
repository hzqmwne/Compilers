CC=gcc
CFLAGS=-g -c -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast 

tiger-compiler:  main.o y.tab.o lex.yy.o errormsg.o absyn.o prabsyn.o symbol.o translate.o semant.o parse.o escape.o frame.o table.o util.o types.o env.o tree.o temp.o printtree.o canon.o codegen.o assem.o graph.o flowgraph.o liveness.o color.o regalloc.o
	$(CC) -g $^ -o $@

frame.o: x86frame.c
	cc $(CFLAGS) $< -o $@

y.tab.o: y.tab.c
	cc $(CFLAGS) $<

y.tab.c: tiger.y
	yacc -dv tiger.y

y.tab.h: y.tab.c
	echo "y.tab.h was created at the same time as y.tab.c"

lex.yy.o: lex.yy.c
	cc $(CFLAGS) $<

lex.yy.c: tiger.lex
	lex tiger.lex

handin:
	tar -czf id.tar.gz *.h *.c *.lex *.y 

clean: 
	rm -f tiger-compiler *.o y.tab.c y.tab.h y.output lex.yy.c

