# Copyright 2021 Grama Nicolae
# The linux makefile.
# Project directories

# Executable name and path
LIBNAME = libso_stdio.so

# Libraries information (build, components)
# Compilation parameters
CC = gcc
CFLAGS =  -fPIC -Wall -Wextra -pedantic -g -O2 -std=c89
OBJS = src/so_stdio.o 

# Code Styling
CSFILES = src/*

# Build the program
build: $(OBJS)
	$(info Building executable...)
	@$(CC) -shared -o $(LIBNAME) $^ $(CFLAGS)
	rm $(OBJS)

# Create the object files
%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS)

# Automatic coding style, in my personal style
beauty:
	@cp code_styles/personal .clang-format
	@clang-format -i -style=file $(CSFILES)
	
# Automatic coding style, using the required coding style
beauty_req:
	@cp code_styles/linux .clang-format
	@clang-format -i -style=file $(CSFILES)
	@cp code_styles/personal .clang-format

# Remove object files and executables
clean:
	@rm -rf $(LIBNAME) $(OBJS) ./checker-lin/$(LIBNAME)

archive: clean beauty_req
	zip -FSr SO_STDIO.zip ./src GNUmakefile README.md  .git/
	@$(MAKE) -s beauty

check: clean build
	cp $(LIBNAME) checker-lin/
	@$(MAKE) -s -C checker-lin/ -f Makefile.checker