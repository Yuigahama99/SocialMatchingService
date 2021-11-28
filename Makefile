GXX = g++
OBJECTS = serverC serverT serverS serverP clientA clientB
FLAGS = -std=c++11
JUNKS = *.out $(OBJECTS)

all: $(OBJECTS)

serverC: central.cpp
	$(GXX) $(FLAGS) $^ -o $@

serverT: serverT.cpp
	$(GXX) $(FLAGS) $^ -o $@

serverS: serverS.cpp
	$(GXX) $(FLAGS) $^ -o $@

serverP: serverP.cpp
	$(GXX) $(FLAGS) $^ -o $@

clientA: clientA.cpp
	$(GXX) $(FLAGS) $^ -o $@

clientB: clientB.cpp
	$(GXX) $(FLAGS) $^ -o $@

clean:
	rm -rf $(JUNKS)