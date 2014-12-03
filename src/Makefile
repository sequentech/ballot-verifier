OUTPUT := x64

#INCSPATH := -Ilibjson
INCSPATH := 
LIBSPATH := 
#LIBS := -lgmpxx -lgmp -ljson
LIBS := -lgmpxx -lgmp -lcurl

OBJS := sha256.o ElGamal.o Random.o encrypt.o

all: build

build: $(OUTPUT) $(OBJS)
	$(CXX) -DNDEBUG -DDEBUG -g $(OUTPUT)/*.o main.cpp -o $(OUTPUT)/main $(INCSPATH) $(LIBSPATH) $(LIBS)

.PHONY: build clean all run

clean:
	rm -Rf $(OUTPUT)
	
%.o:  %.cpp
	$(CXX) -DNDEBUG -DDEBUG -g -c  $^ $(INCSPATH)
	mv $@ $(OUTPUT)
	
$(OUTPUT):
	mkdir -p $@
	
run:
	cd example && python -m SimpleHTTPServer & echo "$$!" > pid
	sleep 1
	$(OUTPUT)/main example/ballot.json || true
	kill `cat pid`
	rm pid