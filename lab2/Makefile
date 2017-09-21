lextest: driver.o lex.yy.o errormsg.o util.o
	gcc -g -o lextest driver.o lex.yy.o errormsg.o util.o

driver.o: driver.c tokens.h errormsg.h util.h
	gcc -g -c driver.c

errormsg.o: errormsg.c errormsg.h util.h
	gcc -g -c errormsg.c

lex.yy.o: lex.yy.c tokens.h errormsg.h util.h
	gcc -g -c lex.yy.c

lex.yy.c: tiger.lex
	lex tiger.lex

util.o: util.c util.h
	gcc -g -c util.c

clean: 
	rm -f lextest util.o driver.o lex.yy.o lex.yy.c errormsg.o

handin:
	@tar -czf xxx.tar.gz tiger.lex 
	@echo "Please rename xxx.tar.gz to <your student id>.tar.gz."
	@echo "For example, 5140379000.tar.gz."

grade:
	./gradeMe.sh
