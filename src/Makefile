OUTPUT := x64
OUTPUTW64 := w64

INCSPATH := -I/usr/include/ -I/home/felix/workspace/libgmp/gmp-6.0.0/
LIBSPATH := 
#LIBS := -lgmpxx -lgmp -ljson  -I/usr/share/mingw-w64/include/
LIBS := -lgmpxx -lgmp -lcurl

OBJS := sha256.o ElGamal.o Random.o encrypt.o

WXX32 := i686-w64-mingw32-g++
WXX64 := x86_64-w64-mingw32-g++

all: build

build: $(OUTPUT) $(OBJS)
	$(CXX) -DNDEBUG -g $(OUTPUT)/*.o main.cpp -o $(OUTPUT)/agora-airgap $(INCSPATH) $(LIBSPATH) $(LIBS)

.PHONY: build clean all run gui xcompile OBJSW64

clean:
	rm -Rf $(OUTPUT)
	
%.o:  %.cpp
	$(CXX) -DNDEBUG -g -c  $^ $(INCSPATH) $(LIBSPATH) $(LIBS)
	mv $@ $(OUTPUT)
	
$(OUTPUT):
	mkdir -p $@
	
$(OUTPUTW64):
	mkdir -p $@

$(OUTPUT)/screen.png:
	cp screen.png $(OUTPUT)
	
	
$(OUTPUTW64)/screen.png:
	cp screen.png $(OUTPUTW64)
	
run:
	cd example && python -m SimpleHTTPServer & echo "$$!" > pid
	sleep 1
	$(OUTPUT)/agora-airgap download-audit example/ballot.json || true
	kill `cat pid`
	rm pid

gui: $(OUTPUT) $(OUTPUT)/screen.png $(OBJS)
# `wx-config --libs` `wx-config --cxxflags`
	$(CXX) -DNDEBUG -g $(OUTPUT)/*.o interface.cpp   `wx-config --cxxflags --libs` -o $(OUTPUT)/interface $(INCSPATH) $(LIBSPATH) $(LIBS)
	
	
OBJSW64:
	$(WXX64) -DNDEBUG -g -c  sha256.cpp $(INCSPATH) $(LIBSPATH) $(LIBS)
	$(WXX64) -DNDEBUG -g -c  ElGamal.cpp $(INCSPATH) $(LIBSPATH) $(LIBS)
	$(WXX64) -DNDEBUG -g -c  Random.cpp $(INCSPATH) $(LIBSPATH) $(LIBS)
	$(WXX64) -DNDEBUG -g -c  encrypt.cpp $(INCSPATH) $(LIBSPATH) $(LIBS)
	mv *.o $(OUTPUTW64)
	
xcompile: $(OUTPUTW64) $(OUTPUTW64)/screen.png OBJSW64
	$(WXX64) $(XCOMPILEFLAGS) -DNDEBUG -g $(OUTPUTW64)/*.o interface.cpp   `wx-config --cxxflags --libs` -o $(OUTPUTW64)/interface $(INCSPATH) $(LIBSPATH) $(LIBS)
	
	