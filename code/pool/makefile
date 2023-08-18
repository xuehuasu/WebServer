include ../build/Makefile

all: $(OBJECTS)

${OBJECTS}: ${OBJ_DIR}/%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)