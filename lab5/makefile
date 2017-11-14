a.out: parse.o prabsyn.o y.tab.o lex.yy.o errormsg.o util.o table.o absyn.o symbol.o semant.o types.o env.o tree.o temp.o printtree.o frame.o translate.o
	gcc -g parse.o prabsyn.o y.tab.o lex.yy.o errormsg.o util.o table.o absyn.o symbol.o semant.o types.o env.o tree.o temp.o printtree.o frame.o translate.o

parse.o: parse.c errormsg.h util.h
	gcc -g -c parse.c

translate.o: translate.c translate.h
	gcc -g -c translate.c 

frame.o: x86frame.c frame.h
	gcc -g -c x86frame.c -o frame.o

temp.o: temp.c temp.h
	gcc -g -c temp.c

tree.o: tree.c tree.h 
	gcc -g -c tree.c

printtree.o: printtree.c printtree.h
	gcc -g -c printtree.c

prabsyn.o: prabsyn.c prabsyn.h
	gcc -g -c prabsyn.c
semant.o: semant.c semant.h
	gcc -g -c semant.c
env.o: env.c env.h
	gcc -g -c env.c
types.o: types.c types.h
	gcc -g -c types.c
y.tab.o: y.tab.c
	gcc -g -c y.tab.c

y.tab.c: tiger.y
	yacc -dv tiger.y

y.tab.h: y.tab.c

errormsg.o: errormsg.c errormsg.h util.h
	gcc -g -c errormsg.c

lex.yy.o: lex.yy.c symbol.h absyn.h y.tab.h errormsg.h util.h
	gcc -g -c lex.yy.c

lex.yy.c: tiger.lex
	lex tiger.lex

util.o: util.c util.h
	gcc -g -c util.c
table.o: table.c table.h
	gcc -g -c table.c
absyn.o: absyn.h absyn.c
	gcc -g -c absyn.c
symbol.o: symbol.c symbol.h
	gcc -g -c symbol.c

handin:
	tar -czf id.name.tar.gz  absyn.[ch] errormsg.[ch] makefile gradeMe.sh parse.[ch] prabsyn.[ch] refs-5 symbol.[ch] table.[ch] testcases tiger.lex tiger.y util.[ch] env.[ch] semant.[ch] translate.[ch] *.h *.c
clean: 
	rm -f a.out parse.o prabsyn.o y.tab.o lex.yy.o errormsg.o util.o table.o absyn.o symbol.o semant.o types.o env.o tree.o temp.o printtree.o frame.o translate.o y.tab.c y.tab.h lex.yy.c y.output *~
