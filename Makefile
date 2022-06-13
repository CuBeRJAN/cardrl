CC=g++
CPPFLAGS=-O2
DEPS = cpptree.h cardrl.h card.h pile.h os-specific.h player.h enemy.h
OBJ = cardrl.o card.o os-specific.o player.o enemy.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

cardrl: $(OBJ)
	$(CC) -o $@ $^ $(CPPFLAGS)

clean:
	rm *.o cardrl
