OUTPUT := x64
OUTPUTW64 := w64

INCSPATH := -I/usr/include/ 
WINCSPATH := -I/usr/share/mingw-w64/include/ -I/usr/local/include/
LIBSPATH := -L/usr/local/lib
LIBS :=  -Wl,-Bstatic -lgmpxx -lgmp -lstdc++ `curl-config --static-libs` -Wl,-Bdynamic `wx-config --cxxflags --libs`




OBJS := sha256.o ElGamal.o Random.o encrypt.o

WXX32 := i686-w64-mingw32-g++
WXX64 := x86_64-w64-mingw32-g++

all: build

build: $(OUTPUT) $(OBJS)
	$(CXX) -DNDEBUG -g $(OUTPUT)/*.o main.cpp -o $(OUTPUT)/agora-airgap $(INCSPATH) $(LIBSPATH) $(LIBS)

.PHONY: build clean all run gui xcompile OBJSW64

clean:
	rm -Rf $(OUTPUT)
	rm -Rf $(OUTPUTW64)
	rm -f *.o
	
%.o:  %.cpp
	$(CXX) -DNDEBUG -g -c  $^ $(INCSPATH)
	mv $@ $(OUTPUT)
	
$(OUTPUT):
	mkdir -p $@
	
$(OUTPUTW64):
	mkdir -p $@
	
run:
	cd example && python -m SimpleHTTPServer & echo "$$!" > pid
	sleep 1
	$(OUTPUT)/agora-airgap download-audit example/ballot.json || true
	kill `cat pid`
	rm pid

gui: $(OUTPUT) $(OBJS)
	$(CXX) -Wall -DNDEBUG -g $(OUTPUT)/*.o interface.cpp  -o $(OUTPUT)/agora-audit $(INCSPATH) $(LIBSPATH) $(LIBS)
	
OBJSW64:
	$(WXX64) -DNDEBUG -g -c  sha256.cpp $(WINCSPATH)
	$(WXX64) -DNDEBUG -g -c  ElGamal.cpp $(WINCSPATH)
	$(WXX64) -DNDEBUG -g -c  Random.cpp $(WINCSPATH)
	$(WXX64) -DNDEBUG -g -c  encrypt.cpp $(WINCSPATH)
	mv *.o $(OUTPUTW64)
	
xcompile: $(OUTPUTW64) OBJSW64
	$(WXX64) -DNDEBUG -g $(OUTPUTW64)/*.o interface.cpp    -o $(OUTPUTW64)/interface.exe $(WINCSPATH) $(LIBSPATH) -static -lgmpxx -lgmp -lstdc++ -DCURL_STATICLIB /usr/local/lib/libcurl.dll.a -L/usr/local/lib -lidn -lws2_32  `wx-config --static=yes --cxxflags --libs` 
	
	
