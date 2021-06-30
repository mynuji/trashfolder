# trashfolder

# 배경

1998년도인가에 만들었던 유닉스/리눅스용 trash folder 프로그램입니다. 

Command Line Interface의 터미널에서 rm명령어로 파일을 삭제했을 때, 복원할 수 있는 기능을 제공합니다. 

Trash Folder의 단점인 휴지통 비우기를 하지 않아서, 서버의 디스크를 모두 잡아먹는 경우를 없애기 위해서, 특정 일자(이틀)가 경과된 파일은 rm명령어를 사용했을 때, 먼저 삭제를 한 후에 실제 파일을 삭제하도록 하였습니다. 
서버의 다양한 사용자들을 통제(?)하기 위한 기능입니다. 

폴더를 삭제하고 난 후에 삭제한 폴더를 한꺼번에 복원하는 기능은 제공하지 않습니다.  하지만, 파일별로 복원하는 기능을 제공합니다.

이렇게 만든 사유는 삭제를 했을 때, 중요한 파일을 복원할 수 있으면 RISK를 대비할 수 있어서, 폴더를 일괄로 복원하는 기능은 만들지를 않았습니다. 


휴지통 기능을 사용할 계정에서 각각 아래의 설치를 해야한다.

# 컴파일

$ make

gcc -g  -Wall   -c -o trash.o trash.c

gcc -g  -Wall -o trash trash.o


컴파일 할 때, Warning이 2개 나타날 수 있는데, 이것은 무시해도 된다.

맥 OS에서 사용할 때는, Makefile 의 __OSX__ 를 설정해줘야 한다. 컴파일할 때, OSX OS에 대해 check하는 부분을 넣지 못하였다. darwin... 뭣이었나 기억이 가물가물...


# 설치


$ make install

mkdir -p ~/.trash

cp trash ~/.trash/trash

echo "alias rm='~/.trash/trash'" >> ~/.bash_profile

source ~/.bash_profile



 bash shell에서는 위의 make install 명령으로 자동으로 설정이 되지만, csh 등의 다른 쉘을 사용할 경우, alias 등록을 직접해줘야 한다.
 
 bash의 경우에는 .profile 또는 .bash_profile 의 맨 아래 내용에 alias를 추가한다
 
 alias rm='~/.trash/trash'
 
 
 csh의 경우에는 .cshrc 파일의 맨 아래 내용에 아래 내용을 추가한다.
 
 alias rm '~/.trash/trash'


OSX의 경우에는 설치되는 폴더가 .trash 가 아닌, .trash_folder를 사용한다.  그 사유는 OSX의 GUI에서 사용하는 휴지통 기능의 폴더가 .Trash로 설정되어 있어서 같이 사용하면 안된다.
make install 할 때, 스크립트 오류가 발생될 수 있는데, 설치는 직접 해당 폴더를 만들어주고 profile에 alias를 설정해준 후에 재로그인을 하면 된다.


# 사용방법

 - 삭제명령: 기존 rm 명령과 동일하다.
 - 파일복원: rm -u

 - 종료방법: Ctrl-X
 - 상하이동: 커서키 또는 J, K 키
 - 파일복원: Enter
 - 선택   : Space
 - 삭제   : Ctrl-D
 
# Original rm 사용 방법
 - OS에서 제공하는 기본적인 rm 명령어를 사용하기 위해서는 command line 의 맨 앞에 '\\'를 시작하면 됩니다. 이것은 alias 기능을 사용하지 않고, 원래의 명령어를 사용한다는 의미입니다. 
 
   $ \rm *.tmp
 
 
# Screen Shot
![image](https://user-images.githubusercontent.com/20812684/122073134-2ab39c00-ce33-11eb-97a4-a7905479a437.png)


# Trouble Shooting

 - make install 했을 때, 오류가 발생되는 경우가 있음.
   오류 발생하는 원인은 Makefile 을 제가 잘못 만들어서 발생하는 것입니다.
   
   홈디렉토리에 .trash 디렉토리를 만들고, 그 폴더 내에 실행화일을 넣어두면 됨.
   
   물론, 시스템 전체에 적용하기 위해서는 굳이 홈디렉토리에 설치하지 않고, /usr/local, /usr/bin 등의 폴더에 trash폴더를 만들어서 사용해도 됨.
   
   쉘의 rc파일, profile에 alias정보를 등록하여, 기존의 rm 명령어 대신에 trash 프로그램이 실행되도록 설정하면 됩니다. ^^
   
 - 맥북에서 설치할 때의 오류(정확히는 make install 했을 때 오류가 남)
   OSX의 GUI에서 사용되는 휴지통이 .Trash 라는 폴더를 사용하고 있어서, 중복 오류가 발생될 수 있고, 여기에 같이 사용할 경우, 다른 문제점이 발생될 수 있어서, 이 프로그램의 .trash폴더를 다른이름으로 변경해야 함. Makefile 에 OSX에 대한 부분 주석 처리된 부분을 제거할 필요있음.
 - makefile에서 OS define정보를 참조해서 자동으로 처리되게 변경해야 되는데, 자꾸 미루게 되네요.... ^^
 
