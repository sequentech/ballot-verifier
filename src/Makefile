OUTPUT := x64

#INCSPATH := -Ilibjson
INCSPATH := 
LIBSPATH := 
#LIBS := -lgmpxx -lgmp -ljson
LIBS := -lgmpxx -lgmp -lcurl

OBJS := sha256.o ElGamal.o Random.o encrypt.o

all: build

build: $(OUTPUT) $(OBJS)
	$(CXX) -DNDEBUG -g $(OUTPUT)/*.o main.cpp -o $(OUTPUT)/agora-airgap $(INCSPATH) $(LIBSPATH) $(LIBS)

.PHONY: build clean all run

clean:
	rm -Rf $(OUTPUT)
	
%.o:  %.cpp
	$(CXX) -DNDEBUG -g -c  $^ $(INCSPATH)
	mv $@ $(OUTPUT)
	
$(OUTPUT):
	mkdir -p $@
	
run:
	cd example && python -m SimpleHTTPServer & echo "$$!" > pid
	sleep 1
	$(OUTPUT)/agora-airgap download-audit example/ballot.json || true
	kill `cat pid`
	rm pid