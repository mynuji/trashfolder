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

OS = "__OSX__"
CFLAGS = -g 
#CFLAGS += -DEXCEPT_ROOT 
#CFLAGS += -D$(OS)
CFLAGS += -Wall


ifeq (OS, "__OSX__")
	INSTALL_DIR = ~/.trash_folder
else
	INSTALL_DIR = ~/.trash
endif

PROFILE1 = ~/.bash_profile
PROFILE2 = ~/.profile

ifeq ($(shell test -e $(PROFILE2) && echo -n yes),yes)
	PROFILE=$(PROFILE2)
endif
ifeq ($(shell test -e $(PROFILE1) && echo -n yes),yes)
	PROFILE=$(PROFILE1)
endif


all:    trash

trash:	${OBJECT}
	${CC} ${CFLAGS} -o trash $(OBJECT)  

md_install_dir:
	mkdir -p $(INSTALL_DIR)

clean:
	rm -f *.o

install: md_install_dir
	cp trash ~/$(INSTALL_DIR)/trash
	echo "alias rm='~/$(INSTALL_DIR)/trash'" >> $(PROFILE)
	cd
	. $(PROFILE)

trash.o 	: 	Makefile trash.h


