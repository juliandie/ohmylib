obj-c += $(wildcard ./lib/*.c)

./lib/%.o: ./lib/%.c ./includes/%.h
	$(CC) $(ccflags) -c $< -o $@

./lib/%.o: ./lib/%.c ./lib/%.h
	$(CC) $(ccflags) -c $< -o $@

./lib/%.o: ./lib/%.c
	$(CC) $(ccflags) -c $< -o $@