
NAME      		= webserv

CC          	= clang++
CFLAGS      	= -Wall -Wextra -Werror -Wpedantic -std=c++11 -O0 -g
# -Wsign-conversion
# CFLAGS      	= -Wall -Wextra -Werror -Wuninitialized -O3

SRCEXT      	= cpp
SRCDIR      	= src
OBJEXT      	= o
HEADEREXT		= hpp
HEADERDIR		= include
BUILDDIR    	= obj

LIBS			=
INCLUDES		= -I$(HEADERDIR)
LINK			=

SRC 			= $(shell find $(SRCDIR) -name '*.$(SRCEXT)')
HEADERS 		= $(shell find $(HEADERDIR) -name '*.$(HEADEREXT)')
OBJ				= $(foreach src,$(SRC),$(BUILDDIR)/$(notdir $(src:.$(SRCEXT)=.$(OBJEXT))))

SILECE_MAKE 	= | grep -v -E ".*Leaving directory|.*Entering directory"
VPATH 			= $(shell find $(SRCDIR) -type d | tr '\n' ':' | sed -E 's/(.*):/\1/')
.SUFFIXES:

ifdef DEBUG
CFLAGS += -fsanitize=address
endif

all: $(NAME) www/cgi/input

$(NAME): $(BUILDDIR)/ $(OBJ) $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) $(LIBS) -o $(NAME) $(LINK)

$(BUILDDIR)/%.$(OBJEXT): %.$(SRCEXT) $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(BUILDDIR)/$(notdir $@)

www/cgi/input: www/cgi/input.c
	gcc -Wall -Wextra www/cgi/input.c -o www/cgi/input

# sources


clean:
ifneq ($(BUILDDIR),.)
	/bin/rm -rf $(BUILDDIR)/
endif

fclean: | clean
	/bin/rm -f $(NAME)
	/bin/rm -f www/cgi/input

re: | fclean all

$(BUILDDIR)/:
	mkdir -p $(BUILDDIR)

silent:
	@$(MAKE) > /dev/null

.PHONY: all clean fclean re silent
