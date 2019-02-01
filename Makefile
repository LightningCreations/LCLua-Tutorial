CC := gcc
CXX := g++
INCLUDE_PATH := include

CXXHEAD := $(foreach ipath,$(INCLUDE_PATH),$(wildcard $(ipath)/**/*.hpp) $(wildcard $(ipath)/**/*.inl))
CCHEAD := $(foreach ipath,$(INCLUDE_PATH),$(wildcard $(ipath)/**/*.h))



OBJECT_FILES = 

STANDALONE_OBJECT_FILES = $(OBJECT_FILES)

CC_DIALECT = -std=c11
CXX_DIALECT = -std=c++17

CC_FLAGS = -g -fvisibility-inlines-hidden -fvisibility=default -fpic $(CC_DIALECT) -w -fwrapv
COMPILE_FLAGS = -g -fvisibility-inlines-hidden -fvisibility=default -fpic $(CXX_DIALECT) -w -fpermissive -fwrapv
LINKER_FLAGS = -shared -flinker-output=dyn -pthread -fpic
STANDALONE_LINKER_FLAGS = -pthread -fpic
LIBS = -lssl -lcrypto -llc-cxx
OUTPUT = liblclua.so
STANDALONE = lclua
INCLUDE = $(foreach ipath,$(INCLUDE_PATH),-I$(ipath))
DEFINES = -DLCLIB_CXX_DEFINITION

LIBNAME = -Wl,-soname,liblclua.so

DIRS := out 

BASE_DIR := out

define gethead0
$(wordlist 2,$(words $(1)),$(1)) 
endef

define gethead
$(call gethead0,$(shell $(CXX) -MM $(CXX_DIALECT) $(INCLUDE) $(1))) 
endef

.PHONY: all FORCE install uninstall clean rebuild relink version
.DEFAULT: all
.PRECIOUS: Makefile $(DIRS)
.SECONDEXPANSION:
.SECONDARY: $(CXXHEAD:%.hpp=%.hpp.gch) $(CCHEAD:%.h=%.h.gch) $(LINFO_OBJ:out/%.o=src/%.cpp)

all: $(DIRS) $(OUTPUT) $(STANDALONE)

version:
	@echo $(CXX) version
	@$(CXX) --version
	@echo $(CC) verison
	@$(CC) --version

FORCE: ;

$(OUTPUT): $(OBJECT_FILES)
	$(CXX) $(LINKER_FLAGS) $(LIBNAME) -o $@ $^ $(LIBS)

$(STANDALONE): $(STANDALONE_OBJECT_FILES) 
	$(CXX) $(STANDALONE_LINKER_FLAGS) -o $@ $^ $(LIBS)
	
$(BASE_DIR):
	mkdir $(BASE_DIR)
	
$(BASE_DIR)/%/: $(BASE_DIR)
	mkdir -p $@

install: all
	install $(OUTPUT) /usr/lib/
	install --mode=755 -d -v /usr/include/lclib include/lclib
	cp -R include/lclib /usr/include
	chmod -R 755 /usr/include/lclib
	install --mode=755 -d -v /usr/include/detail include/lclib/detail
	cp -R include/detail /usr/include
	chmod -R 755 /usr/include/detail
	install --mode=755 $(STANDALONE) /usr/bin

uninstall:
	rm -rf /usr/include/lclib
	rm -rf /usr/lib/$(OUTPUT)

relink:
	rm -rf $(OUTPUT)
	$(MAKE) $(OUTPUT)


clean:
	rm -rf $(OBJECT_FILES)
	rm -rf $(LINFO_OBJ)
	rm -f $(OUTPUT)
	rm -f $(CXXHEAD:%.hpp=%.hpp.gch)
	rm -f $(CCHEAD:%.h=%.h.gch)

rebuild:
	$(MAKE) clean
	$(MAKE) $(OUTPUT)


out/%.o: src/%.cpp $$(D@) $(CXXHEAD:%.hpp=%.hpp.gch) $(CCHEAD:%.h=%.h.gch)
	$(CXX) $(COMPILE_FLAGS) $(DEFINES) -c $(INCLUDE) -o $@ $<

out/%.o: src/%.c $$(D@) $(CCHEAD:%.h=%.h.gch)
	$(CC) $(COMPILE_FLAGS) $(DEFINES) -c $(INCLUDE) -o $@ $<

$(LINFO_OBJ): $(LINFO_OBJ:out/%.o=src/%.cpp) out/ $(OBJECT_FILES)
	$(CXX) $(COMPILE_FLAGS) $(DEFINES) -c $(INCLUDE) -o $@ $<

%.hpp.gch: %.hpp
	$(CXX) $(COMPILE_FLAGS) $(DEFINES) -c $(INCLUDE) -o $@ $<

%.h.gch: %.h 
	$(CC) $(COMPILE_FLAGS) $(DEFINES) -c $(INCLUDE) -o $@ $<
