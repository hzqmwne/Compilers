a.out: parse.o prabsyn.o y.tab.o lex.yy.o errormsg.o util.o table.o absyn.o symbol.o
	gcc -g parse.o prabsyn.o y.tab.o lex.yy.o errormsg.o util.o table.o absyn.o symbol.o

parse.o: parse.c errormsg.h util.h
	gcc -g -c parse.c

prabsyn.o: prabsyn.c prabsyn.h
	gcc -g -c prabsyn.c

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
	@tar -czf lab3.tar.gz tiger.y tiger.lex
	@echo "Please rename lab3.tar.gz to <your student id>.tar.gz."
	@echo "For example, 5140379000.tar.gz."                           

grade:
	./gradeMe.sh

clean: 
	rm -f a.out parse.o prabsyn.o y.tab.o lex.yy.o errormsg.o util.o table.o absyn.o symbol.o y.tab.c y.tab.h lex.yy.c y.output *~
