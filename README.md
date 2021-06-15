# trashfolder

# 배경

1998년도인가에 만들었던 유닉스/리눅스용 trash folder 프로그램입니다. 

Command Line Interface의 터미널에서 rm명령어로 파일을 삭제했을 때, 복원할 수 있는 기능을 제공합니다. 

Trash Folder의 단점인 휴지통 비우기를 하지 않아서, 서버의 디스크를 모두 잡아먹는 경우를 없애기 위해서, 특정 일자(이틀)가 경과된 파일은 rm명령어를 사용했을 때, 먼저 삭제를 한 후에 실제 파일을 삭제하도록 하였습니다. 
서버의 다양한 사용자들을 통제(?)하기 위한 기능입니다. 

폴더를 삭제하고 난 후에 삭제한 폴더를 한꺼번에 복원하는 기능은 제공하지 않습니다.  하지만, 파일별로 복원하는 기능을 제공합니다.

이렇게 만든 사유는 삭제를 했을 때, 중요한 파일을 복원할 수 있으면 RISK를 대비할 수 있어서, 폴더를 일괄로 복원하는 기능은 만들지를 않았습니다. 


# 컴파일

$ make
gcc -g  -Wall   -c -o trash.o trash.c
trash.c: In function ‘delete_file’:
trash.c:1871:27: warning: ‘%s’ directive writing up to 255 bytes into a region of size 192 [-Wformat-overflow=]
       sprintf(buf, "%s/%s/%s", home, TRASH_FOLDER, t->trash);
                           ^~
trash.c:1871:7: note: ‘sprintf’ output 9 or more bytes (assuming 264) into a destination of size 200
       sprintf(buf, "%s/%s/%s", home, TRASH_FOLDER, t->trash);
       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gcc -g  -Wall -o trash trash.o


# 설치방법
