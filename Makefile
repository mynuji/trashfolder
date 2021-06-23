#
# trash Makefile
#
#
#  $Log: Makefile,v $
#  Revision 1.2  2012/06/19 14:33:59  igi
#  *** empty log message ***
#
#  Revision 1.1  2002/06/07 09:20:28  igi
#  Initial revision
#
#  myjunu@gmail.com
#

CC=gcc
#------------------------------------------------------------------------------
OBJECT =  trash.o

CFLAGS = -g 
#CFLAGS += -DEXCEPT_ROOT 
CFLAGS += -Wall


INSTALL_DIR = ~/.trash

PROFILE1 = ~/.bash_profile
PROFILE2 = ~/.profile

ifeq ($(shell test -e $(PROFILE1) && echo -n yes),yes)
	PROFILE=$(PROFILE1)
endif
ifeq ($(shell test -e $(PROFILE2) && echo -n yes),yes)
	PROFILE=$(PROFILE2)
endif


all:    trash

trash:	${OBJECT}
	${CC} ${CFLAGS} -o trash $(OBJECT)  

md_install_dir:
	mkdir -p $(INSTALL_DIR)

clean:
	rm -f *.o

install: md_install_dir
	cp trash ~/.trash/trash
	echo "alias rm='~/.trash/trash'" >> $(PROFILE)
	source $(PROFILE)

trash.o 	: 	Makefile trash.h


