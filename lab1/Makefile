CFLAGS=-std=c99 

a.out: main.o myimpl.o prog1.o slp.o util.o
	gcc $(CFLAGS) $^

myimpl.o: myimpl.c slp.h util.h prog1.h
	gcc $(CFLAGS) -c $<

main.o: main.c slp.h util.h prog1.h
	gcc $(CFLAGS) -c $<

prog1.o: prog1.c slp.h util.h
	gcc $(CFLAGS) -c $<

slp.o: slp.c slp.h util.h
	gcc $(CFLAGS) -c $<

util.o: util.c util.h
	gcc $(CFLAGS) -c $<

handin:
	@tar czf lab1_xxx.tar.gz myimpl.c
	@echo "Please rename lab1_xxx.tar.gz to lab1_<your student id>.tar.gz"
	@echo "For example, lab1_5140379000.tar.gz."

grade:
	./gradeMe.sh
	
clean: 
	rm -f a.out *.o
