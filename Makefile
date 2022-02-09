PREFIX=/usr/local
LIBDIR=$(PREFIX)/lib

CC=clang
CFLAGS=-O
FC=gfortran

NAME=vecLibFort
SOURCE=$(NAME).c
OBJECT=$(NAME).o
LIBRARY=lib$(NAME)
STATIC=$(LIBRARY).a
DYNAMIC=$(LIBRARY).dylib
PRELOAD=$(LIBRARY)I.dylib
INCLUDES=cloak.h static.h
DEPEND=$(INCLUDES) Makefile

all: static dynamic preload
static: $(STATIC)
dynamic: $(DYNAMIC)
preload: $(PRELOAD)

$(OBJECT): $(DEPEND)

$(STATIC): $(OBJECT)
	ar -cru $@ $^
	ranlib $@

$(DYNAMIC): $(OBJECT)
	$(CC) -shared -o $@ $^ \
		-Wl,-reexport_framework -Wl,Accelerate \
		-install_name $(LIBDIR)/$@

$(PRELOAD): $(SOURCE) $(DEPEND)
	$(CC) -shared $(CFLAGS) -DVECLIBFORT_INTERPOSE -o $@ -O $(SOURCE) \
		-Wl,-reexport_framework -Wl,Accelerate \
		-install_name $(LIBDIR)/$@

install: all
	mkdir -p $(LIBDIR)
	cp -f $(STATIC) $(LIBDIR)
	cp -f $(DYNAMIC) $(LIBDIR)
	cp -f $(PRELOAD) $(LIBDIR)

clean:
	rm -f $(OBJECT) $(STATIC) $(DYNAMIC) $(PRELOAD)

check: tester.f90 $(OBJECT)
	$(FC) -o tester -O $^ -framework Accelerate
	./tester

