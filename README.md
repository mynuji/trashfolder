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
gcc -g  -Wall -o trash trash.o

$ make install
cp trash ~/.trash/trash

Warning 2개 나오는 것은 무시한다. 

# 설치방법

 bash shell에서는 .profile 의 마지막에 아래 내용을 추가한다
 
 alias rm='~/.trash/trash'

# 사용방법

 - 삭제명령: 기존 rm 명령과 동일하다.
 - 파일복원: rm -u

 - 종료방법: Ctrl-X
 - 상하이동: 커서키 또는 J, K 키
 - 파일복원: Enter
 - 선택   : Space
 - 삭제   : Ctrl-D
 
# Original rm 사용 방법
 - OS에서 제공하는 기본적인 rm 명령어를 사용하기 위해서는 command line 의 맨 앞에 '\'를 시작하게 한다. 
 
   $ \rm *.tmp
 
 
# Screen Shot
![image](https://user-images.githubusercontent.com/20812684/122073134-2ab39c00-ce33-11eb-97a4-a7905479a437.png)

