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
#

CC=gcc
#------------------------------------------------------------------------------
OBJECT =  trash.o

CFLAGS = -g 
#CFLAGS += -DEXCEPT_ROOT 
CFLAGS += -Wall

all:    trash

trash:	${OBJECT}
	${CC} ${CFLAGS} -o trash $(OBJECT)  
clean:
	rm -f *.o

install:
	cp trash ~/.trash/trash

trash.o 	: 	Makefile trash.h
