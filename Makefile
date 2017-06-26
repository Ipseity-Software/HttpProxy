OBJDIR = obj
BINDIR = bin
FLAGS = 
OFLAGS = -c
DBGFLAGS = -g
INCLUDES = `pwd`
LIBS = -pthread
CC = gcc
RMFLAGS = -f
items = util main db

test = main_dbg.o db_dbg.o

define run-cc
$(CC) $(OFLAGS) -o $@ $(firstword $^) $(src) $(sub_flags) $(FLAGS) -I$(INCLUDES)
endef

define run-ld
$(CC) -o $(BINDIR)/$@ $(obj) $(sub_flags) $(FLAGS) $(LIBS)
endef

define do-final
strip $(BINDIR)/$@
endef

.PHONY : Release
Release: dirs $(BINDIR)/vx

.PHONY : all
all: dirs $(BINDIR)/test_vx $(BINDIR)/vx
.PHONY : dirs
dirs:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)
$(BINDIR)/test_vx: test_vx
$(BINDIR)/vx: vx

test_vx: obj = $(items:%=$(OBJDIR)/%_dbg.o)
test_vx: sub_flags := $(DBGFLAGS)
test_vx:
	@echo $(obj)
	$(MAKE) $(obj)
	$(run-ld)

vx: obj = $(items:%=$(OBJDIR)/%.o)
vx: sub_flags := -fPIC
vx:
	@echo $(obj)
	$(MAKE) $(obj)
	$(run-ld)
	$(do-final)

$(OBJDIR)/%_dbg.o: src = $(@:$(OBJDIR)/%_dbg.o=%.c)
$(OBJDIR)/%_dbg.o: $(src)
	$(run-cc)

$(OBJDIR)/%.o: src = $(@:$(OBJDIR)/%.o=%.c)
$(OBJDIR)/%.o: $(src)
	$(run-cc)

.PHONY : clean
clean:
	rm $(RMFLAGS) $(OBJDIR)/*.o
.PHONY : clean_all
clean_all: clean
	rm $(RMFLAGS) $(BINDIR)/*
