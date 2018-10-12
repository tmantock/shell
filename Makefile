SMALLSHELL = main.c smallshell.c lexer.c

shell: $(SMALLSHELL)
	gcc -o smallsh $(SMALLSHELL) -std=gnu99
clean:
	test -f smallsh && rm smallsh
