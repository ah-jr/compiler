etapa7: main.o lex.yy.o y.tab.o hash.o ast.o semantic.o tacs.o asm.o
	gcc main.o lex.yy.o y.tab.o hash.o ast.o semantic.o tacs.o asm.o -o compiler

main.o: main.c
	gcc -c main.c

semantic.o: semantic.c
	gcc -c semantic.c

hash.o: hash.c
	gcc -c hash.c

tacs.o: tacs.c
	gcc -c tacs.c

ast.o: ast.c
	gcc -c ast.c

asm.o: asm.c
	gcc -c asm.c

y.tab.o: y.tab.c
	gcc -c y.tab.c

lex.yy.o: lex.yy.c
	gcc -c lex.yy.c

y.tab.c: parser.y
	yacc -d parser.y

lex.yy.c: scanner.l
	flex scanner.l
	
clean:
	rm compiler lex.yy.c y.tab.c *.o