#!/usr/bin/make
# Makefile automatically generated using /home/crunch/bin/configure  
# 2017-02-21

# Build machine specs
export MACHINE  = x86_64-pc-linux-gnu
export ARCH     = x86_64

# C compiler information
export CC       = /usr/bin/cc
export CCVER    = 6.3.1
export STDC_VER = 201112
export CFLAGS   = -O3 -fno-strict-aliasing -Wall -Wextra -march=native -Iinclude

# Project information
export PREFIX   = /usr/local/bin
export PROJECT  = scheme
export VERSION  = 0.1.0

export DOCS     = docs
export MANPATH  = /usr/share/man/man1
# Build information
export SRC_DIR  = src
export INC_DIR  = include
export BUILD    = build
export HEADERS  = $(wildcard $(INC_DIR)/*.h)
export SOURCE   = $(wildcard $(SRC_DIR)/*.c)
export OBJECTS  = $(SOURCE:.c=.o)
export TARGET   = $(BUILD)/$(PROJECT)

.PHONY: all clean dist install uninstall upgrade

all: $(TARGET)

clean:
	@rm -rf $(OBJECTS)

dist: $(TARGET) clean 
	@echo "creating distribution archive $(PROJECT)-$(VERSION).tar.gz"
	if [ -d $(DOCS) ]; then \
		tar -czvf $(PROJECT)-$(VERSION).tar.gz configure Makefile $(DOCS) $(TARGET) $(SRC_DIR) $(IN_DIR); \
	else \
		tar -czvf $(PROJECT)-$(VERSION).tar.gz configure Makefile $(TARGET) $(SRC_DIR) $(INC_DIR); \
	fi 
install: $(TARGET)
	@echo "installing $(PROJECT) in $(PREFIX)"
	@cp -uv $(TARGET) $(PREFIX)/$(PROJECT)-$(VERSION)
	@chmod 0755 $(PREFIX)/$(PROJECT)-$(VERSION)
	@ln -sf $(PREFIX)/$(PROJECT)-$(VERSION) $(PREFIX)/$(PROJECT) 
	if [ -f $(DOCS)/$(PROJECT).1 ]; then \
		cp $(DOCS)/$(PROJECT).1 $(MANPATH); \
	fi;

uninstall:
	@echo "uninstalling $(PROJECT) in $(PREFIX)"
	if [ -f $(PREFIX)/$(PROJECT) ]; then \
		rm -rf $(PREFIX)/$(PROJECT); \
		rm -rf $(PREFIX)/$(PROJECT)-$(VERSION); \
	fi
	if [ -f $(MANPATH)/$(PROJECT).1 ]; then \
		rm -rf $(MANPATH)/$(PROJECT).1; \
	fi

upgrade:
	@cp -uv $(TARGET) $(PREFIX)/$(PROJECT)-$(VERSION)
	@cp -uv $(DOCS)/$(PROJECT).1 $(MANPATH)/$(PROJECT).1
	@ln -sf $(PREFIX)/$(PROJECT)-$(VERSION) $(PREFIX)/$(PROJECT) 

%.o: %.c
	@echo "cc $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS) $(HEADERS)
	@echo "building $(PROJECT)"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)
