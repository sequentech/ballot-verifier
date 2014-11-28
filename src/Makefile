OUTPUT := x64

#INCSPATH := -Ilibjson
INCSPATH := 
LIBSPATH := 
#LIBS := -lgmpxx -lgmp -ljson
LIBS := -lgmpxx -lgmp 

OBJS := sha256.o ElGamal.o Random.o encrypt.o

all: build

build: $(OUTPUT) $(OBJS)
	$(CXX) -DNDEBUG $(OUTPUT)/*.o main.cpp -o $(OUTPUT)/main $(INCSPATH) $(LIBSPATH) $(LIBS)

.PHONY: build clean all run

clean:
	rm -Rf $(OUTPUT)
	
%.o:  %.cpp
	$(CXX) -DNDEBUG -c  $^ $(INCSPATH)
	mv $@ $(OUTPUT)
	
$(OUTPUT):
	mkdir -p $@
	
run:
	$(OUTPUT)/main