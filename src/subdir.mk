C_SRC += $(wildcard src/*.c)

src/%.o: src/%.c inc/%.h
	$(CC) $(CFLAGS) -c $< -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@