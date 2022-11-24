C_SRC += $(wildcard $(SRC_DIR)/src/*.c)
C_HDR += $(wildcard $(SRC_DIR)/inc/*.h)

./src/%.c.o: ./src/%.c ./inc/%.h
	$(CC) $(CFLAGS) -c $< -o $@

./src/%.c.o: ./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@