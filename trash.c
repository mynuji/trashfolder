/*===========================================================================
 * 프로그램명  :  trash(rm) for the Unix                                     
 * 제목        : 삭제된 프로그램의 복구를 해주는 휴지통 프로그램             
 * 파일명.     : trash.c
 * TAB크기를 2 로 설정하면 소스가 깨끗하게 보임
 *---------------------------------------------------------------------------
 * 라이브러리  :                                                             
 * 작  성  자  : igi (E-mail:myjunu@gmail.com , facebook: myjunu@gmail.com)
 * 헤더  화일  : trash.h
 * 작  성  일  : 98. 2. 20 ( 최종수정일 : 2017.8.21)                         
 *---------------------------------------------------------------------------
 * 기      능  : 삭제명령으로 삭제시 해당 화일을 휴지통에 복사를 하고        
 *               복구 명령으로 복구시 휴지통의 화일을 원래의 위치로          
 *               복사한다. 휴지통의 디렉토리는  ~/.trash 가 되며, 이 디렉토  
 *               리에 삭제된 화일이 옮겨진다.                                
 *               ~/.trash/.erase_date는 최종 자동 삭제 날짜가 기록되어       
 *               있으며, 이 날짜를 키로 하여, 하루에 두번 자동삭제 기능이    
 *               작동하지 않도록 한다.                                       
 *                                                                           
 *               복구시켜주는 것은 기존의 rm 명령으로 삭제된 화일은 복구가   
 *               안되며, 이 프로그램으로 삭제한 프로그램에 한하여,           
 *               복구가 가능하다.                                            
 *---------------------------------------------------------------------------
 *  < UPDATE HISTORY >                                                       
 *    +--------+------------------------------------------------------+------
 *    |수 정 일|         수정내용  및  기타  참고사항                 |수정자
 *    +--------+------------------------------------------------------+------
 *    |98. 3. 5| 화일의 mode, 생성일자 출력 추가                      |igi
 *    |98. 3.25| 화일의 mode로 화일을 복구                            |igi
 *    |98. 5.13| '.'이 없는 화일명 삭제시 에러 수정                   |igi
 *    |02. 6. 7| 환경변수를 실행할 때 전역변수에 저장한 후 사용       |igi
 *    |17. 8.17| 윈도우 크기 변경에 따른 UI 가변적으로 변화도록 함    |igi
 *    |17. 8.18| 윈도우 크기 정의한 데이터형 변경(char --> int)       |igi
 *    |17. 8.18| OSX에서 사용되도록 trash폴더명 변경                  |igi
 *----+--------+------------------------------------------------------+------
 *  컴파일   :                                                               
 *                                                                           
 *     cc -o trash trash.c                                                   
 *                                                                           
 * 설      치  :                                                             
 *                                                                           
 * (0) trash.c 를 컴파일하고 alias를 이용하여 기존 rm명령을 trash로 변경     
 *                                                                           
 *     sh일 경우 .profile에, csh일 경우 .cshrc파일에 아래의 항목을 추가한다. 
 *                                                                           
 *     alias rm 'trash'                                                       
 *                                                                           
 * 사 용 법   :                                                              
 *                                                                           
 * (1) 삭제시에는  기존의 rm 명령과 동일하다.                                
 *                                                                           
 * (2) 화일 복구시에    rm -u 를 입력하면, 복구메뉴가 화면에 출력된다.       
 *                                                                           
 *     단, 복구시에 원래의 디렉토리로 복구가 되므로, 기존의 화일이 삭제될    
 *                                                                           
 *     수 있으므로(이것은 복구 불가) 주의가 요망된다.                        
 *                                                                           
 * 현재, AIX, Solaris, HPUX에서 테스트 완료                        
 *                                                                           
 *  $Log: trash.c,v $
 *  Revision 1.4  2012/06/19 14:32:30  igi
 *  utf8 로 한글 변였고,
 *  컴파경고 나오는 건에 대해 수정함.
 *
 *  Revision 1.3  2002/06/28 01:13:31  igi
 *  override 여부 확인 입력시 getch()로 된 것을 getdata()로 변경함.
 *  이는 getc(), getchar()가 정상적으로 작동되지 않아 문자열입력 받는
 *  함수를 새로 구현하게 되었다.
 *
 *  날짜 표현시 2000년 넘는 항목이 두자리로 표현될때 100이상의 값을
 *  갖고 출력되도록 한 것을 정상적인 두자리 표현식으로 변경함.
 *
 *  파일 복구시 퍼미션에 대한 완전한 복구 안되는 것 확인되었음.
 *  추후 변경대상
 *
 *  Revision 1.2  2002/06/07 09:41:04  igi
 *  화일 삭제 여부 뭍는 check()함수에 대한 버그 수정
 *  getchar()가 정상적으로 작동하지 않아, getch()를 이용하여 처리함
 *
 *  Revision 1.1  2002/06/07 09:18:56  igi
 *  Initial revision
 *
 *
 *===========================================================================*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <pwd.h>

#include "trash.h"




/* 실행시에 들어오는 인수의 flag를 저장하게 될 인수 */
int dflag, fflag, iflag, rflag, retval, stdin_ok;
extern int errno;

int crtcolumn = 80;
int crtline   = 24;


/*
 * 홈디렉토리의 위치를 저장할 home변수
 * 이 변수는 프로그램 처음 시작할때 값을 세팅해두도록 한다.
 */
char home[1024];


/*----------------------
 * 윈도우 사이즈가 변경되었을 때, 화면을 다시 그려주려고 했더니...
 * 어쩔수 없이, unerase()안에서 정의한 변수들을 밖으로 빼내어서(전역변수로)
 * 처리를 한다.
 *-------------------------*/
  trash_t head, *p, *t;








void getWindowSize()
{
	struct winsize w;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	crtline   = w.ws_row;
	crtcolumn = w.ws_col;

	gotoxy(crtcolumn, crtline);
//	printf("\ncolumn: %d, line: %d\n", crtcolumn, crtline);
}



void do_resize(int dummy)
{

	getWindowSize();
	cls();
  prn_head();
  prn_page(&head, p);

}

/*--------------------------------------------------------------------------
함수명 : int is_substring(char s[], char key[])
인수   : s 는  검색하게될 대상이 되는 문자열
         key는 검색의 키로 사용될 문자열로 '*', '?'등의 문자로 표현될수있다.
기능   : key로 사용된 문자열에 s가 포함되는지를 찾는 함수
반환   : 문자열(s)가 key에 포함된다면 1을, 아니면 0을 반환

사용예 :  is_substring("abcdefg", "abcdefg")  =>  1
          is_substring("abcdefg", "abcdeff")  =>  0
          is_substring("abcdefg", "abc*")     =>  1
          is_substring("abcdefg", "abc??fg")  =>  1
          is_substring("abcdefg", "a*c??fg")  =>  1
          is_substring("abcdefg", "*c??fg")   =>  1

***주의) 아직 완전한 검증을 받지 못함.. 테스트는 했는데.... 오랜 테스트
         시간없이 경우의 수만을 만들어서 만든 테스트라서 완전한 검증이되지
         못했다.
---------------------------------------------------------------------------*/
int is_substring(char s[], char key[])
{
  int i, j, wild;
  int temp_j=-1;

  /* 초기화 */
  i = j = wild = 0;

  /* 문자열을 다 소비할때까지 */
  while (i < strlen(s) ) {
    /* ?키일때 그냥 지나감  */
    if (key[j] == '?')  j++;
    else {
      if (key[j] == '*') { wild = 1; temp_j = ++j; }
      if (wild) {
        if (s[i] == key[j])  j++; 
        else if (j != temp_j) {j = temp_j; continue;}
      }
      else if (s[i] == key[j])  j++;
      else return 0;
    }
    i++;
  }
  /* 루틴을 마치고 나오면 i와 j는 각 문자열의 마지막 NULL을 가리키고 */
  /* 있어야지만, 정상적인 것이므로, 만일 이 자료가 서로 같지 않다면  */
  /* 오류로 반환                                                     */
  if (s[i] != key[j]) return 0;
  return 1;
}

/*--------------------------------------------------------------------------
함수명 : init_link(trash_t *head)
기능   : list의 head를 초기화 한다.
반환   : 없음.
사용예 : init_link(&head);
-------------------------------------------------------------------------*/
void init_link(trash_t *head)
{
  head->llink = head;
  head->rlink = head;
}

/*--------------------------------------------------------------------------
함수명 : add_link(trash_t *head, trash_t *node)
기능   : head의 list에 node를 추가한다
반환   : 없음.
사용예 : add_link(&head, &node);
-------------------------------------------------------------------------*/
void add_link(trash_t *head, trash_t *node)
{
  node->rlink = head;
  node->llink = head->llink;
  head->llink->rlink = node;
  head->llink = node;
}

/*--------------------------------------------------------------------------
함수명 : del_link(trash_t *head, trash_t *node)
기능   : head의 list에서 node를 삭제한다
반환   : 없음.
사용예 : del_link(&head, &node);
-------------------------------------------------------------------------*/
void del_link(trash_t *head, trash_t *node)
{
  if (node != head) {
    node->llink->rlink = node->rlink;
    node->rlink->llink = node->llink;
    free(node->f_name);
    free(node->path);
    free(node);
  }
}

/*--------------------------------------------------------------------------
함수명 : destroy_link(trash_t *head)
기능   : head의 모든 list를 삭제한다.
반환   : 없음.
사용예 : destroy_link(&head);
-------------------------------------------------------------------------*/
void destroy_link(trash_t *head)
{
  trash_t *t;

  for (t=head->rlink; t!=head; t=head->rlink) del_link(head, t);
}

/*-------------------------------------------------------------------------*/
int readline(int fd, char *ptr, int maxlen)
{
  int n, rc;
  char c;
  
  for (n = 1; n < maxlen; n++) {
    if ( (rc = read(fd, &c, 1)) == 1) {
      *ptr++ = c;
      if (c == '\n') break;
    }
    else if (rc == 0) {
      if (n == 1) return 0;
      else break;
    }
    else return -1;
  }
  *ptr = 0;
  return n;
}

/*--------------------------------------------------------------------------*/
void cursor_on()
{
  printf("%c[1z", 27);
  fflush(stdout);
}

/*--------------------------------------------------------------------------
함수명 : cls()
기능   : 화면 삭제하고 커서를 0,0 위치로 이동
반환   : 없음.
사용예 : cls()
-------------------------------------------------------------------------*/
void cls()
{
  printf("%c[2J%c[0;0H",27, 27);
  fflush(stdout);
}

/*--------------------------------------------------------------------------
함수명 : gotoxy(int x, int y)
기능   : 커서를 x, y 좌표로 이동
반환   : 없음.
사용예 : gotoxy(x, y)
-------------------------------------------------------------------------*/
void gotoxy(int x, int y)
{
  printf("%c[%d;%dH", 27, y, x);
  fflush(stdout);
}

/*------------------------------------------------------------------------
함수명 : reverse_scr()
기능   : 출력모드를 역상으로 정의
반환   : 없음.
사용예 : reverse_scr()
-------------------------------------------------------------------------*/
void reverse_scr()
{
  printf("%c[7m", 27);
   fflush(stdout);
}

/*------------------------------------------------------------------------
함수명 : beep()
기능   : beep 발생
반환   : 없음.
사용예 : beep()
-------------------------------------------------------------------------*/
void beep()
{
  printf("%c", 0x07);
  fflush(stdout);
}

/*------------------------------------------------------------------------
함수명 : norm_scr()
기능   : 출력모드를 일반모드로 전환
반환   : 없음.
사용예 : norm_scr()
-------------------------------------------------------------------------*/
void norm_scr()
{
  printf("%c[0m", 27);
  fflush(stdout);
}

/*------------------------------------------------------------------------
함수명 : save_scr()
기능   : 현재 커서의 위치를 저장
반환   : 없음.
사용예 : save_scr()
-------------------------------------------------------------------------*/
void save_scr()
{
  printf("%c[s", 27);
  fflush(stdout);
}

/*------------------------------------------------------------------------
함수명 : restore_scr()
기능   : 저장된 커서 위치로 커서 이동
반환   : 없음.
사용예 : restore_scr()
-------------------------------------------------------------------------*/
void restore_scr()
{
  printf("%c[u", 27);
  fflush(stdout);
}

/*------------------------------------------------------------------------
함수명 : bold_scr()
기능   : 굵은 글자 모드로 변경
반환   : 없음.
사용예 : bold_scr()
-------------------------------------------------------------------------*/
void bold_scr()
{
  printf("%c[1m", 27);
  fflush(stdout);
}

/*------------------------------------------------------------------------
함수명 : underline_scr()
기능   : 밑줄 긋기 모드로 변경 
반환   : 없음.
사용예 : underline_scr()
-------------------------------------------------------------------------*/
void underline_scr()
{
  printf("%c[4m", 27);
  fflush(stdout);
}

/*------------------------------------------------------------------------
함수명 : cls_eol()
기능   : 라인 삭제
반환   : 없음.
사용예 : cls_eol()
-------------------------------------------------------------------------*/
void cls_eol()
{
  printf("%c[K", 27);
  fflush(stdout);
} 

/*------------------------------------------------------------------------
함수명 : get_trash_file
기능   : 휴지통 디렉토리(.trash)에서 화일의 목록정보를 읽어온다.
         readdir()을 사용하여 휴지통 디렉토리에 있는 모든 화일의 정보를
         다 읽어오도록 한다. 하지만, 형식에 어긋나는 화일의 경우에는 
         그냥 스킵시키도록 한다.
인수   : head : 읽은 화일 정보를 저장할 linked list의 헤더 
반환   : 성공시에  읽은 화일의 갯수를, 그외는 -1을 반환
-------------------------------------------------------------------------*/
int get_trash_file(trash_t *head)
{
  char path[1024];
  DIR *dp;
  struct dirent *dirp;
  int i = 0;
  char *ptr;
  struct stat sb;

  /* 휴지통의 디렉토리를 설정한다. 자신의 홈디렉토리 아래에 존재 */
  sprintf(path, "%s/%s", home, TRASH_FOLDER);

  ptr = path + strlen(path);
  *ptr++ = '/';
  *ptr = 0;


  /* path 디렉토리(휴지통)을 연다. 열다가 에러발생하면 오류로 반환 */
  if ( (dp = opendir(path)) == NULL) return -1;

  /* 화일이 없을때 까지 계속 화일을 읽어서 리스트에 연결을 시킨다. */
  /* 여기서, 화일 정보에 오류가 있는 화일은 무시하고 넘어가도록 한다. */
  while ( (dirp=readdir(dp)) != NULL) {


    /* 읽은 화일명이  '.', '..' 이라면 무시하고 지나감 */
    if (strcmp(dirp->d_name, ".") == 0 ||
        strcmp(dirp->d_name, "..") == 0) continue;

    strcpy(ptr, dirp->d_name);
		
    /* 해당하는 화일명의 상태를 읽다가 에러 발생시 무시하고 다음 화일처리 */
    if (lstat(path, &sb) < 0) {
      prn_error(path, errno);
      continue;
    }

    /* 읽은 화일명이 디렉토리거나 symbolic link화일이라면 무시 */
    if (S_ISDIR(sb.st_mode) || S_ISLNK(sb.st_mode)) {
			continue;
		}
    /* 화일명에 해당하는 화일을 읽어서 head리스트에 읽어온다. */
    if (get_record(head, path, dirp->d_name) == 0) i++;
  } 
  /* 좀전에 연 디렉토리를 닫는다 */
  if (closedir(dp) < 0) return -1;

  /* 휴지통에서 읽은 화일의 갯수 */
  return i;
}

/*-------------------------------------------------------------------------
함수명 : str_chr_down(char *s, char ch)
기능   : strchr과 비슷한 함수이나, 차이점은 strchr은 처음부터 검색을 하는데
         str_chr_down은 뒤에서 부터 앞으로 검색을 해온다는 것이 큰 차이점이
         다. 
인수   :  s : 검색하게될 문자열
          ch : 키로 사용될 문자
사용예 :  t = str_chr_down("abcdefg/abcdef/ka", '/') => t="/ka"
--------------------------------------------------------------------------*/
char *str_chr_down(char *s, char ch)
{
  char *t;

  t = s + strlen(s) - 1;
  while (s != t) {
    if (*t == ch) return t;
    t--;
  }
  return NULL;
}

/*---------------------------------------------------------------------------
함수명 : get_file_info(char *path, trash_t *node)
기능   : path의 화일명을 읽어서 삭제전의 path, file명, size를 읽어서
         node에 저장하는 작업을 한다.
반환   : 성공시 0을 , 실패시 -1을 반환
----------------------------------------------------------------------------*/
int get_file_info(char *path, trash_t *node)
{
  int f;
  char buf[500];
//  char size[15];
//  char name[100];
  char *t;
  time_t f_time;
  struct tm *ltime;

  /* 화일을 오픈하고 */ 
  if ( (f=open(path, O_RDONLY)) == -1) return -1;
  /* 첫번째 줄(삭제되기전, 있었던 화일의 정보)를 읽음 */
  if (readline(f, buf, 500)  <= 0) {
    close(f);
    return -1;
  }

  /*문자열에서 '\n'을 없애줌 */
  if ( (t = str_chr(buf, '\n')) ) *t='\0';

  /* 화일명을 짤라낸다 */
  t = str_chr_down(buf, '/');
 
  /* path없이 화일명으로 구성되었다면 */ 
  if (t == NULL) {
    /* 이 경우는 없다고 본다. 무조건 화일명의 path명은 '/'는 꼭 하나*/
    /* 는 있기때문에 이 루틴이 실행될일은 없다.                     */
    node->f_name = (char *) xmalloc(strlen(buf)+1);
    strcpy(node->f_name, buf);
    node->path = (char *) xmalloc(2);
    strcpy(node->path, "/");
  }
  else {
    /* 화일명과 path명을 구분하여 구조체에 저장 한다. */
    *t++ = 0;
    node->f_name = (char *) xmalloc(strlen(t)+1);
    strcpy(node->f_name, t);
    node->path = (char *) xmalloc(strlen(buf)+1);
    strcpy(node->path, buf);
  }

  /* 두번째 줄(삭제되기전, 있었던 화일의 크기)를 읽음 */
  if (readline(f, buf, 15)  <= 0) {
    close(f);
    return -1;
  }
  /* 화일의 크기를 저장한다. */
  node->size = atol(buf);

  /* 세번째 줄 */
  if (readline(f, buf, 15)  <= 0) {
    close(f);
    return -1;
  }
  /* 화일의 날짜를 저장한다. */
  f_time = atol(buf);
  ltime = localtime(&f_time);
  node->o_time.year = ltime->tm_year+1900;
  node->o_time.month= ltime->tm_mon+1;
  node->o_time.day  = ltime->tm_mday;
  node->o_time.hour = ltime->tm_hour;
  node->o_time.min  = ltime->tm_min;
  node->o_time.sec  = ltime->tm_sec;

  /* 네번째 줄 */
  if (readline(f, buf, 15)  <= 0) {
    close(f);
    return -1;
  }
  /* 화일의 mode를 저장한다. */
  node->mode = atol(buf);
   
  /* 열은 화일 닫기 */
  close(f);

  /* 성공이니 0을 반환 */
  return 0;
}

/*-------------------------------------------------------------------------
함수명 : get_record(trash_t *head, char *path, struct stat sb, char *path)
기능   : 휴지통에서의 화일의 정보를 읽어와서 리스트(head)에 node를 추가
         시켜주는 함수, 
---------------------------------------------------------------------------*/
int get_record(struct trash_t *head, char *path, char *fname)
{
  struct tm *t_date, date;
  time_t t;
  struct trash_t *node;
  char buf[500];
  char *p;


  /* 화일명을 읽어서 해당 시간을 구한다 */
  /* 확실히 하려면 화일명의 '.'뒤로는 삭제를 하고 숫자로 바꿔야한다. */
  /* 화일명 자체가 시간으로 표시되었기에  아래 방법으로 삭제시간을 구한다. */
  strcpy(buf, fname);
  p = strchr(buf, '.');
  if (p == NULL) return -1;
  if (*p) *p=0;  
  t = atol(buf);
  if (!t) return -1;   
  t_date = localtime(&t);
  memcpy(&date, t_date, sizeof(date));

  /* 화일의 정보를 담기 위한 구조체 포인터하나를 할당받는다 */
  node = (struct trash_t *)xmalloc(sizeof(struct trash_t));
  memset(node, 0, sizeof(struct trash_t));

  /* 휴지통에 있는 현재의 화일명을 저장 */
  strcpy(node->trash, fname); 

  /* 해당화일의 화일명, path명, 화일 크기등을 저장 */
  if (get_file_info(path,  node) != 0) {
    free(node);
    return -1;
  }
  /* 삭제된 시간을 저장 */
  node->d_time.year = date.tm_year + 1900;
  node->d_time.month = date.tm_mon + 1;
  node->d_time.day = date.tm_mday;
  node->d_time.hour = date.tm_hour;
  node->d_time.min = date.tm_min + 1;
  node->d_time.sec = date.tm_sec;

  /* list에 node를 추가한다. */
  add_link(head, node);
  return 0; 
}

int chkWindowSize(int line, int column)
{
	if (line < 13) {
    gotoxy(1, 5);
    printf("[!] Window 크기(라인)가 너무 작아 화면에 표시를 할 수 없습니다.종료(Ctrl+X)\n");
		return -1;
	}
	if (column < 65) {
    gotoxy(1, 5);
    printf("[!] Window 크기(폭)가 너무 작아 화면에 표시를 할 수 없습니다.종료(Ctrl+X)");
		return -1;
	}
	return 0;
}

/*---------------------------------------------------------------------------
함수명 : prn_page(trash_t *head, trash_t *p)
기능   : p로 시작하는 리스트부터 화일의 정보를 화면에 한페이지 출력한다.
----------------------------------------------------------------------------*/
void prn_page(trash_t *head, trash_t *p) 
{
  trash_t *t;
  int i;

	if (chkWindowSize(crtline, crtcolumn) < 0) return;

  /* 한화면에 나타낼수 있는 화일의 갯수만큼 화면에 출력한다. */
  t = p;
  for (i=0; i < crtline-5; i++) {
    if (t == head) break;
    prn_record(i, t);
    t = t->rlink;
  }

  /* 출력되고 남은 공간을 blank로 출력한다. */
  while (i < crtline-5) {
    prn_blank(i);
    i++;
  }
  gotoxy(crtcolumn,crtline);
}

/*---------------------------------------------------------------------------
함수명: prn_record(char i, trash_t *t)
기능  : t node의 자료를 화면에 출력한다.
        화일명, 삭제일, 화일 크기 등을 출력한다. 이때 화일 크기는
        1k 이하는 999B 로 표현하고 1M 이하는 999K로 그외는 999M로 표현했다.
----------------------------------------------------------------------------*/
void prn_record(int i, trash_t *t)
{
  char buf[500];
  char buf2[500];
	char s[1024];

//  int  size;
  int re_x;
  int re_y;

	if (chkWindowSize(crtline, crtcolumn) < 0) return;

  /* 출력할 좌표를 정하고 */
  re_x = 0;
	re_y = i % (crtline-5);
/*  re_y = i % (MAX_ITEM); */
  /* 각 item을 출력한다. */

	sprintf(buf, "%%-%d.%ds", crtcolumn - (int)strlen(TITLE_FILLER)-9,
														crtcolumn - (int)strlen(TITLE_FILLER)-9);
	sprintf(buf2, buf, t->f_name);
 
  sprintf(s, " %c%s %02d/%02d/%02d %02d:%02d:%02d %6ld%c "
               "%c%c%c%c%c%c%c%c%c %02d/%02d/%02d %02d:%02d:%02d  ",
               (t->is_select)?'*':' ',
               buf2,
               (t->o_time.year-1900)%100, t->o_time.month, t->o_time.day,
               t->o_time.hour,  t->o_time.min, t->o_time.sec,

/*               t->size, */

               (t->size < 1000) ? t->size 
                                : 
               (t->size < 1000000L) ? t->size / 1000
                                : 
               (t->size < 1000000000L) ? t->size / 1000000L
                                    : t->size / 1000000000L,
               (t->size < 1000) ? 'B'
                                :
               (t->size < 1000000L) ? 'K' 
                                :
               (t->size < 1000000L) ? 'M' : 'G',

               (t->mode & S_IREAD) ? 'r' : '-',
               (t->mode & S_IWRITE) ? 'w' : '-',
               (t->mode & S_IEXEC) ? 'x' : '-',
               (t->mode & S_IRGRP) ? 'r' : '-',
               (t->mode & S_IWGRP) ? 'w' : '-',
               (t->mode & S_IXGRP) ? 'x' : '-',
               (t->mode & S_IROTH) ? 'r' : '-',
               (t->mode & S_IWOTH) ? 'w' : '-',
               (t->mode & S_IXOTH) ? 'x' : '-',

               (t->d_time.year-1900)%100, t->d_time.month, t->d_time.day,
               t->d_time.hour, t->d_time.min, t->d_time.sec);
 
  gotoxy( re_x * 39 + 2, re_y + 4);
  printf("%s", s);
}

/*---------------------------------------------------------------------------
함수명 : prn_blank(char i)
기능   : 해당 좌표에 공백을 인쇄한다.
----------------------------------------------------------------------------*/
void prn_blank(int i)
{
  int re_x, re_y;
	char buf[1024];

  /* 출력할 좌표 정의 */
  re_x = 0;
/*( i < (MAX_ITEM) ) ? 0 : 1;*/
/*  re_y = i % (MAX_ITEM ); */
  re_y = i % (crtline-5);
  gotoxy( re_x * 39 + 2, re_y + 4);

	sprintf(buf, "%%%d.%ds", crtcolumn-3, crtcolumn-3);
  printf(buf,"");
}

/*---------------------------------------------------------------------------
함수명 : cursor_off(struct temio *tbufsave)
기능   : 터미날의 echo기능과 buffering기능을 없애는 역할을 한다.
         이 함수는 예전에 유닉스에 처음 접했을때 책에서 옮긴 함수로
         그냥 여기서도 사용하게 되었다. 문제가 많은 함수중 하나이다. 
----------------------------------------------------------------------------*/
void cursor_off( struct termios *tbufsave)
{
  static struct termios tbuf;

  if (tcgetattr(0, &tbuf) < 0) {
    perror("ioctl 에러라나요..");
  }
  *tbufsave = tbuf;
  tbuf.c_iflag &= ~(BRKINT|IGNPAR|IXON|IXANY|IXOFF);
#ifdef XCASE
  tbuf.c_lflag &= ~(ISIG|ICANON|XCASE|ECHO|ECHOE|ECHOK|ECHONL);
#else
  tbuf.c_lflag &= ~(ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHONL);
#endif
  tbuf.c_cc[VINTR] = -1;
  tbuf.c_cc[VINTR] = -1;
  tbuf.c_cc[VQUIT] = -1;
  tbuf.c_cc[VERASE] = -1;
  tbuf.c_cc[VKILL] = -1;
  tbuf.c_cc[VMIN] = 1;
  tbuf.c_cc[VTIME] = 0;
  tbuf.c_cc[VEOL2] = -1;
/*  tbuf.c_cc[VSWTCH] = -1; */
  if (tcsetattr(0, TCSAFLUSH, &tbuf) < 0) {
    perror("ioctl2 에러랍니다.");
  }
  printf("%c[z", 27);
  fflush(stdout);
}


/*-------------------------------------------------------------------------
함수명 : restore(struct temio *tbufsave)
기능   : 터미날의 상태를 tbufsave로 정의한다.
         cursor_off()를 사용하기전의 상태로 되돌리기 위한 함수로 구현되었다.
--------------------------------------------------------------------------*/
void restore( struct termios *tbufsave)
{
  if (tcsetattr(0, TCSAFLUSH, tbufsave) == -1) perror("ioctl3");
  cursor_on();
}

/*-------------------------------------------------------------------------
함수명 : prn_head()
기능   : 단지 화면에 head 와 foot 영역에 문자열을 출력해 주는 함수 
--------------------------------------------------------------------------*/
void prn_head()
{
  char buf[100];

	if (chkWindowSize(crtline, crtcolumn) < 0) return;

  reverse_scr();
  gotoxy(1,1);
	
	sprintf(buf, "%%-%d.%ds", crtcolumn, crtcolumn);
	printf(buf, HEAD_FILLER);

/*  printf("%-80.80s",HEAD_FILLER); */

  gotoxy(1,3);
  norm_scr();
  underline_scr();


	sprintf(buf, "%%-%d.%ds", crtcolumn-(int)strlen(TITLE_FILLER), 
														crtcolumn-(int)strlen(TITLE_FILLER));
	printf(buf, TITLE_FILENAME_FILLER);

	sprintf(buf, "%%-%d.%ds", (int)strlen(TITLE_FILLER), (int)strlen(TITLE_FILLER));

  printf(buf, TITLE_FILLER);

/*  printf("%-80.80s",TITLE_FILLER); */

  norm_scr();
  reverse_scr();
  gotoxy(1,crtline);

	sprintf(buf, "%%-%d.%ds", crtcolumn, crtcolumn);
	printf(buf, FOOT_FILLER);

/*  printf("%-80.80s",FOOT_FILLER); */

  gotoxy(crtcolumn,crtline);
  norm_scr();
}

/*---------------------------------------------------------------------------
함수명 : prn_msg(char *s)
기능   : 두번째 줄에 에러메세지나 상태 메세지(s)를 출력한다.
---------------------------------------------------------------------------*/
void prn_msg( char *s)
{
	char buf[1024];

  gotoxy(1,2);
  sprintf(buf, "%%-%d.%ds", crtcolumn, crtcolumn);
  printf(buf, "");
  underline_scr();
  gotoxy(crtcolumn-strlen(s)+1, 2);
  printf("%s", s);
  gotoxy(crtcolumn,crtline);
  norm_scr();
}

/*--------------------------------------------------------------------------
함수명 : unerase()
기능   : 휴지통에 있는 화일을 읽어와서 선택한 화일을 삭제 저장 하는 메인함수
         너무나 오래간만에 화면 제어 프로그램을 짠것같다. 너무 오래된 탓인지,
         아니면, 라이브러리 구축 탓인지.. 웬지 짜기 싫음을 조금은 느끼게 
         되는게 현실이다. 조금만 바람 쉬고 와서 새로운 맘으로 계속 짜야겠다.
---------------------------------------------------------------------------*/

void unerase()
{
  struct termios tbufsave;
  unsigned int ch;
  int cnt;





  /* 리스트를 초기화 한다. */
  init_link(&head);
 
  /* head리스트에 휴지통에서 읽은 화일의 정보를 연결한다. */ 
  if (get_trash_file(&head) <= 0) {
    printf("\n");
    printf("휴지통에 저장된 화일이 없습니다.\n");
    return;
  }


	printf("[!] 삭제된 파일을 정렬하고 있습니다. 잠시만 기다려주세요.\n");
  sort(&head, 1);
  /* 화면을 삭제하고 */
  cls();

	getWindowSize();

  prn_head();

  cnt = 0;
  p = t = head.rlink;
  if (p == &head) return;


	signal(SIGWINCH, do_resize);

  /* 한 page를 화면에 출력한다. */
  prn_page(&head, p);
  /* 현재의 item에 역상으로 만들어준다. */
  prn_reverse_record(cnt, t);
  /* 터미널 모드를 입력된 문자를 출력하지 않고, 버퍼링도 하지 않게 변경 */
  cursor_off(&tbufsave); 
  do {
    /* 1byte를 입력 받음 */
    ch = getch();
    prn_msg("");

    /* 각 키에 해당하는 루틴으로 분기 */
    switch (ch) {
      case K_PGDN : if (pgdn_key(&head, &p, &t, &cnt) != 0)
                    prn_msg("더이상 자료가 없습니다");
                  break;
      case 'h'    :
      case 'H'    :
      case 'j'    :
      case 'J'    :
      case K_DOWN : if (down_key(&head, &p, &t, &cnt) != 0)
                    prn_msg("더이상 자료가 없습니다");
                  break;
      case K_PGUP : if (pgup_key(&head, &p, &t, &cnt) != 0)
                    prn_msg("더이상 자료가 없습니다");
                  break;
      case 'l'    :
      case 'L'    :
      case 'k'    :
      case 'K'    :
      case K_UP   : if (up_key(&head, &p, &t, &cnt) != 0) 
                    prn_msg("더이상 자료가 없습니다");
                  break;
      case K_SPACE: if (t != &head) space_key(t, cnt);
                  down_key(&head, &p, &t, &cnt);
                  break;
      case K_ENTER : if (restore_file(&head, &p, &t, &cnt) == 0) {
                    prn_msg("화일을 복구하였습니다.");
                  }
                  if (p != &head) {
                    prn_page(&head, p);
                    prn_reverse_record(cnt, t);
                  }
                  break;
      case KC_D   :
      case K_DEL  : if (delete_file(&head) <= 0) {
                      prn_msg("삭제할 화일을 선택하지 않았습니다.");
                  }
                  else {
                    cnt = 0;
                    p = t = head.rlink;
                    prn_page(&head, p);
                    prn_msg("휴지통을 비웠습니다.");
                    if (p != &head) prn_reverse_record(cnt, t);
                  }
                  break;
      case 'f'   :
      case 'F'   :
      case K_F5  : sort(&head, 0);
                   cnt = 0; 
                   p = t = head.rlink;
                   if (p != &head) {
                     prn_page(&head, p);
                     prn_reverse_record(cnt, t);
                   }
                   break;
      case 'd'   : 
      case 'D'   :
      case K_F6  : sort(&head, 1);
                   cnt = 0; 
                   p = t = head.rlink;
                   if (p != &head) {
                     prn_page(&head, p);
                     prn_reverse_record(cnt, t);
                   }
                   break;
      case KC_H  :
      case '1'   :
      case K_F1  : help();
                  prn_page(&head, p);
                  if (p != &head) prn_reverse_record(cnt, t);
                  break; 

    }
  } while (ch != KC_X);
  /* 터미널 모드를 원래대로 변경 */
  restore(&tbufsave);
  destroy_link(&head);
  cls();
}

/*--------------------------------------------------------------------------
함수명 : pgdn_key(trash_t *head, trash_t **p, trash_t **t)
기능   : item을 아래로 한칸 이동, 이동할수 있으면 이동하고 할수없으면
         이동하지 못하게 에러로 반환
---------------------------------------------------------------------------*/
int pgdn_key(trash_t *head, trash_t **p, trash_t **t, int *cnt)
{
  trash_t *r;
  int i, j;

  r = *p;
  for (i = 0; i< crtline-5 /*MAX_ITEM*/; i++) {
    if (r->rlink != head) r = r->rlink;
    else break;
  }

  if (i == crtline-5/*MAX_ITEM*/) {
    *p = r;
    
    for (i = 0; i < *cnt; i++) {
      if ( r->rlink != head ) r = r->rlink;
      else break;
    }
    *t = r;
    *cnt = i;
  }
  else {
    prn_record(*cnt, *t);
    *t = *p;
    for (j = 0; j < i; j++) 
      *t = (*t)->rlink;

    if (i == *cnt) {
      prn_reverse_record(*cnt, *t);
      return -1;
    }
    *cnt = i;
    prn_reverse_record(*cnt, *t);
    return -1;
  }
  prn_page(head, *p);
  /* 현재의 item을 역상으로 바꾼다. */
  prn_reverse_record(*cnt, *t);
  return 0;
   
}



/*--------------------------------------------------------------------------
함수명 : pgup_key(trash_t *head, trash_t **p, trash_t **t)
기능   : item을 아래로 한칸 이동, 이동할수 있으면 이동하고 할수없으면
         이동하지 못하게 에러로 반환
---------------------------------------------------------------------------*/
int pgup_key(trash_t *head, trash_t **p, trash_t **t, int *cnt)
{
  trash_t *r;
  int i;

  /* 첫번째 페이지라면 반환 */
  if ( (*p)->llink == head) {
    prn_record(*cnt, *t);
    *cnt = 0;
    *t = head->rlink;
    if (*p != head) prn_reverse_record(*cnt, *t);
    return -1;
  }

  r = *p;
  for (i = 0; i< crtline-5 /*MAX_ITEM*/; i++) {
    if (r->llink != head) r = r->llink;
    else break;
  }
  if (i == 0) return -1;
  *p = r;
  for (i=0; i< *cnt; i++) {
    if ((*p)->rlink != head) r = r->rlink;
    else break;
  }
  *t = r;
  *cnt = i;
   
  prn_page(head, *p);
  /* 현재의 item을 역상으로 바꾼다. */
  prn_reverse_record(*cnt, *t);
  return 0;
}
 

/*--------------------------------------------------------------------------
함수명 : down_key(trash_t *head, trash_t **p, trash_t **t)
기능   : item을 아래로 한칸 이동, 이동할수 있으면 이동하고 할수없으면
         이동하지 못하게 에러로 반환
---------------------------------------------------------------------------*/
int down_key(trash_t *head, trash_t **p, trash_t **t, int *cnt)
{
  /* 마지막 자료라면 반환 */
  if ( (*t)->rlink == head) return -1;
 
  /* 현재 화면내에서의 이동이라면 현재 위치만 이동 */ 
  if (*cnt < crtline-5 /*MAX_ITEM*/ - 1) {
    prn_record(*cnt, *t);
    *t = (*t)->rlink;
    (*cnt)++;
  }
  /* 화면 마지막에서 이동이라면 전체화면이 한칸 위로 스크롤 */
  else {
    *p = (*p)->rlink;
    *t = (*t)->rlink;
    prn_page(head, *p);
  }
  /* 현재의 item을 역상으로 바꾼다. */
  prn_reverse_record(*cnt, *t);
  return 0;
}
 

/*--------------------------------------------------------------------------
함수명 : up_key(trash_t *head, trash_t **p, trash_t **t)
기능   : item을 아래로 한칸 이동, 이동할수 있으면 이동하고 할수없으면
         이동하지 못하게 에러로 반환
---------------------------------------------------------------------------*/
int up_key(trash_t *head, trash_t **p, trash_t **t, int *cnt)
{
  /* 첫 자료라면 반환 */
  if ( (*t)->llink == head) return -1;
  
  /* 현재 화면내에서의 이동이라면 현재 위치만 이동 */ 
  if (*cnt > 0) {
    prn_record(*cnt, *t);
    *t = (*t)->llink;
    (*cnt)--;
  }
  /* 화면 처음에서 이동이라면 전체화면이 한칸 아래로 스크롤 */
  else {
    *p = (*p)->llink;
    *t = (*t)->llink;
    prn_page(head, *p);
  }
  /* 현재의 item을 역상으로 바꾼다. */
  prn_reverse_record(*cnt, *t);
  return 0;
}

/*-------------------------------------------------------------------------
함수명 : space_key(t, cnt)
기능   : 해당하는 t node의 is_select를 toggle시켜준다. 
         이 기능은 space를 눌렀을경우에 선택 유무를 표현하기 위해 사용한다. 
---------------------------------------------------------------------------*/
void space_key( struct trash_t *t, int cnt)
{
  t->is_select = !t->is_select;
  prn_reverse_record(cnt, t);
}

/*---------------------------------------------------------------------------
 함수명: int rmfile(char *f)
 기능  : f라는 화일을 삭제하는 루틴
         f가 디렉토리이고  dflag, rflag가 있다면 해당 디렉토리를 삭제한다.
---------------------------------------------------------------------------*/
int rmfile(char *f)
{
  struct stat sb;

  /* 화일의 상태를 읽어온다 */
  if (lstat(f, &sb) < 0) {
     if (!fflag || errno != ENOENT) prn_error(f, errno);
     return -1;
  }
  /* 디렉토리 이고  dflag가 set  되어있지 않다면 에러 출력 */
  if (S_ISDIR(sb.st_mode) && !dflag) {
    fprintf(stderr, "rm : %s : is a directory\n", f);
    retval = 1;
    return -1;
  }

  /* 화일의 모드값에 따른 루틴으로 기억한다. */
  if (!fflag && !check(f, f, &sb)) return 0;
  /* 디렉토리라면 rmdir을 아니라면 safe_erase를 호출한다. */
  if ((S_ISDIR(sb.st_mode) ? rmdir(f) : safe_erase(f) ) && 
      (!fflag || errno != ENOENT)) {
     prn_error(f, errno);
  } 
  return 0;
}

/*---------------------------------------------------------------------------
함수명 : rmtree(char *path)
기능   : path에 해당하는 디렉토리를 삭제한다. 
---------------------------------------------------------------------------*/
int rmtree(char *path)
{
  struct stat sb;
  char *ptr;
  DIR *dp=NULL;
  struct dirent *dirp;
//  char s[100];

  /* path의 상태를 읽어오고  에러라면 에러 출력 */ 
  if (lstat(path, &sb) < 0) {
    if (!fflag || errno != ENOENT) prn_error(path, errno);
    return -1;
  }
  /* 디렉토리가 아니라면, 화일 삭제로 분기 */
  if ( !S_ISDIR(sb.st_mode) ) {
    if ( !fflag && !check(path, path, &sb) ) return 0;
    if ( safe_erase(path) && ( !fflag || errno != ENOENT ) ) {
       prn_error(path, errno);
       return -1;
    } 
		return 0;
  }
  /*디렉토리라면 */
//  else {
    ptr = path + strlen(path);
    *ptr++ = '/';
    *ptr = 0;
     /* 디렉토리를 오픈 하고 */
    if ( (dp = opendir(path)) == NULL) return -1;
    /* 모든 화일을 다 읽는다. */
    while ( (dirp=readdir(dp)) != NULL) {
      /* '.', '..'라면 무시   */
      if (strcmp(dirp->d_name, ".") == 0 ||
          strcmp(dirp->d_name, "..") == 0) continue;
      strcpy(ptr, dirp->d_name);
      /* 해당 화일의 상태를 읽음 , 에러발생시 무시 */
      if (lstat(path, &sb) < 0) {
        prn_error(path, errno);
        continue;
      }
      /* 읽은 명이 디렉토리라면 rmtree를 화일이면 rmfile을 호출 */
      (S_ISDIR(sb.st_mode)) ? rmtree(path) : rmfile(path);
    } 
//  }
  /* 개방한 디렉토리 닫기 */
//	if (dp == NULL) return 0;
  if (closedir(dp) < 0) return -1;
  *ptr = 0;
  if ( !fflag && !check(path, path, &sb) ) return 0;
  /* 마지막으로 남은 마지막 path삭제 */
  if (rmdir(path) < 0) return -1;
  return 0;
}

/*--------------------------------------------------------------------------
함수명 : check(char *path, char *name, struct stat *sp)
기능   : iflag가 set 되어있다면, 삭제시 매번마다 삭제여부 입력받도록 하며,
         write acceptable이 아니면 삭제여부 입력받도록 한다
---------------------------------------------------------------------------*/
int check( char *path, char *name, struct stat *sp)
{
  char buf[10];
  struct termios tbufsave;

  /* iflag가 있다면 삭제 여부 출력 */
  if (iflag) fprintf(stderr, "remove %s?", path);
  else {
    /* 링크된 화일이거나.. 머시기 거나.. 등등 이라면... 리턴 */
    /* 이 알고리즘은 BSD에서 구현된 것을 그냥 사용했기에 특별한 주석 */
    /* 달기에 좀 어려운듯 하다.... :) 시간상 어렵다는 야그다 */
    if (S_ISLNK(sp->st_mode) || !stdin_ok || !access(name, W_OK)) return 1;
    fprintf(stderr, "override for %s? ", path);
  }
  /* 화면에 모든 문자를 출력하도록 버퍼를 비우고 */
  fflush(stderr);
 
  /* 삭제할 것인지에 대한 여부를 입력받고  */ 
  cursor_off(&tbufsave);
  getdata(NULL, buf, 1, ECHO_ON);
  restore(&tbufsave);

  /* y가 입력되면 1을, 다르다면 0을 반환한다. */
  return ( (buf[0] == 'y') || (buf[0] == 'Y') );
}


/*--------------------------------------------------------------------------
함수명 : checkdot(char **argv)
기능   : 
--------------------------------------------------------------------------*/
void checkdot(char **argv)
{
  //register char *p, **t, **save;
  char *p, **t, **save;
  int complained;  complained = 0;
  for (t = argv; *t;) {
    p = str_chr(*t, '/');
    if (p) ++p; else p = *t;

    if (ISDOT(p)) {
      if (!complained++) 
        fprintf(stderr, "rm: \".\" and \"..\" amy not be removed.\n");
      retval = 1;

    // 구문에 대한 경고 문구 때문에 조건을 풀어적는다.
//origin.     for (save = t; t[0] = t[1]; ++t);
     for (save = t; (t[0] = t[1]) != 0; ++t);
     t = save;
    }
    else ++t;
  }
}

/*-----------------------------------------------------------------------
함수명 : prn_error(char *name, int val)
기능   : 에러코드와 에러메세지를 출력한다.
반환   :  무조건 1을 반환, 의미 없다..
------------------------------------------------------------------------*/
void prn_error( char *name, int val)
{
  (void)fprintf(stderr, "rm: %s: %s.\n", name, strerror(val));
  retval = 1;
}

/*---------------------------------------------------------------------------
함수명 : usage()
기능   : 사용법을 출력하는 함수
반환   : 그냥 프로그램을 종료하면서, 1을 반환한다.
----------------------------------------------------------------------------*/
int usage()
{
  (void)fprintf(stderr, "\n");
  reverse_scr();
  (void)fprintf(stderr, "%s", HEAD_FILLER);
  norm_scr();
  (void)fprintf(stderr, "\n");
  (void)fprintf(stderr, "%s", LICENSE_FILLER);
  reverse_scr();
  (void)fprintf(stderr, "%s", HEAD_FILLER);
  norm_scr();
  (void)fprintf(stderr, "\n\n");
  (void)fprintf(stderr, "usage: rm [-dfiRru] File... \n");
  (void)fprintf(stderr, 
          " -u 옵션을 사용하시면, 삭제된 화일을 복구할수 있습니다\n");
  exit(1);
}

/*----------------------------------------------------------------------------
함수명 : buildup_trash()
기능   : 휴지통으로 사용하게 될 디렉토리가 없다면 새로 생성하는 함수
         휴지통으로 사용하게될 디렉토리는 각 사용자의 홈디렉토리의 .trash
         의 디렉토리가 해당된다. ( '~/.trash' )
반환   : 생성시 오류 발생 하면 -1을 아니면 0을 반환
----------------------------------------------------------------------------*/
int buildup_trash()
{
  struct stat st;
  char path[100];

  /* 휴지통의 path를 만든다 */
  sprintf(path, "%s/%s", home, TRASH_FOLDER);
 
  /* 만들어진 휴지통의 path의 상태를 구한다. */
  if (stat(path, &st) != 0) {
    /* 휴지통 디렉토리가 없다면, mkdir로 만듦 */
    if (mkdir(path, S_IRWXU) != 0) {
      /* 휴지통을 못만들었다면 에러 */
      printf("~/%s 디렉토리를 작성할수 없습니다.\n", TRASH_FOLDER);
      return -1;
    }
  }
  else {
    /*   $HOME/.trash 가 디렉토리가 아니라면  에러 */
    if ( !(st.st_mode & S_IFDIR) ) {
      printf("~/%s 가 디렉토리가 아닙니다.\n", TRASH_FOLDER);
      return -1;
    }
    /*  $HOME/.trash에 접근 할수 없다면 에러 */
    if ( !(st.st_mode & S_IEXEC) ) {
      printf("~/%s를 읽을수 없습니다.\n", TRASH_FOLDER);
      return -1;
    }
  }
  return 0;
} 

/*--------------------------------------------------------------------------
함수명 : make_trash_file
기능   : 휴지통으로 보내게될 화일명을 만드는 함수로, 원래의 화일을 삭제
         하기 전에 먼저 휴지통의 어떤 화일명으로 복사를 한후에 원래의 화일을
         삭제하도록 휴지통이 설계되었다. 이때 사용하게될 휴지통에서 사용하게
         되는 화일명을 만드는 작업이 이 함수에서 이루어진다. 
         화일명을 만드는 법칙은 현재의 시간(즉, 1970년 1월 1일 09:00를 
         기점으로 하여 현재까지 몇초가 걸렸는지)을 화일명으로 하였고,
         중복성을 없애기 위해-동시에 여러화일이 삭제되거나 했을경우 -
         구분 코드로 알파벳코드를 붙이도록 하였다.
         처음 생성된 화일은 999999999.A 라는 화일명으로 생성되고 같은시간에
         생성된 화일명은 9999999999.B로 된다. 동시에 26개 이상이 넘을경우
         에는 999999999.Z 다음에는 999999999.AA 로 구분 필드가 하나더 늘어
         나게 되며, 위의 경우와 마찬가지로 마지막 알파벳이 Z가 될때까지 
         변경되게 된다. 단지, 생성되는 시간이 틀리다면, '*.A'로만으로 화일이
         생성된다.
반환   : 조합된 화일명이 반환된다.
-----------------------------------------------------------------------------*/
char *make_trash_file()
{
  static char f_name[100];
  char fullpath[1024];
  char *t=NULL;
  int fp;

  /* 현재의 시간으로 화일명을 만든다 */
  sprintf(f_name, "%ld.A", time(0));

  /* fullpath로 만듦 */
  sprintf(fullpath, "%s/%s/%s", home, TRASH_FOLDER, f_name);
  t = str_chr(f_name, 'A');

  /* 해당하는 화일이 기존에 존재하지 않을때까지 화일명을 재조합한다. */
  while ((fp = open(fullpath, O_CREAT|O_EXCL|O_WRONLY, 0600)) == -1) {
    /* 필드가 'Z'까지 왔다면 */
    if (*t == 'Z') {
      *t++ = 'A';
      *t   = 'A';
      *(t+1) = '\0';
    }
    else (*t)++;
    sprintf(fullpath, "%s/%s/%s", home, TRASH_FOLDER, f_name);
  }
  /* 화일명 조합이 되었다면 화일을 닫는다.                              */
  /* 이 화일은 아무런 데이타가 없는 빈화일로 된다. 나중에 이 화일명으로 */
  /* 실제의 데이타를 복사하게 된다.                                     */
  close(fp);
  return f_name;
}

/*---------------------------------------------------------------------------
함수명 : path_of_the_old_file(char *path)
기능   : 현재의 작업 경로를 path에 추가시키는 함수
반환   : 재조합된 경로를 반환
----------------------------------------------------------------------------*/
char *path_of_the_old_file(char *path)
{
  static char buf[1024];

  /* 현재의 path 로 재구성 */
  sprintf(buf, "%s/%s", gnu_getcwd(), path);
  return buf;
}

/*--------------------------------------------------------------------------
함수명:  fcopy(char *src, char *dest)
기능  :  화일을 복사하는 함수로, 원래의 화일을 휴지통으로 복사하는 함수
         복사할때 dest로 정의된 화일명으로 복사하기 전에, 상태정보(원래
         화일의 위치, 크기 등)을 먼저 등록하고, 그후에 화일을 이어서 복사
         한다.
반환  :  정상이면 0을 아니면 -1을 반환
--------------------------------------------------------------------------*/
int fcopy(char *src, char *dest)
{
  int f;
  FILE *t;
  char ch[1024];
  struct stat sb;
  char *cwd; 
  int rt;
  struct passwd *pw;

  /* src의 상태를 확인한다. */
  /* 만일 존재하지 않다면 에러 */
  if (stat(src, &sb) != 0) return -1;

  /* 읽을 화일을 열고 */
  if ( (f=open(src, O_RDONLY)) == -1) return -1;
  /* 저장할 화일을 쓰기 모드로 열고 */
  if ( (t=fopen(dest, "w")) == NULL) return -1;

  /* 현재의 작업 디렉토리를 얻고 */ 
  if ( (cwd = gnu_getcwd()) == NULL) return -1;

  /* src가 절대path로 되어있지 않다면 절대경로로 만듦 */
  if (src[0] == '/') fprintf(t, "%s\n", src);
  else  fprintf(t, "%s/%s\n", cwd, src);

  /* 화일의 크기를 등록 */
  fprintf(t, "%lu\n", (unsigned long) sb.st_size);
  fprintf(t, "%lu\n", (unsigned long) sb.st_atime);
  fprintf(t, "%d\n", sb.st_mode);

  /* 화일을 복사한다. */
  while ( (rt = read(f, ch, sizeof(ch))) > 0) {
    fwrite(ch, rt, 1, t);
  }
  close(f);
  if (getenv("USER"))
    sprintf(ch, "%s", getenv("USER"));
  else if (getenv("LOGNAME"))
    sprintf(ch, "%s", getenv("LOGNAME"));
  else { fclose(t); return 0; }
  pw = getpwnam(ch);
  if (pw != NULL) {
    fchown(fileno(t), pw->pw_uid, pw->pw_gid);
  }
  fclose(t);
  /* 복사 끄읕~~~ */
  return 0;
}

/*---------------------------------------------------------------------------
함수명  : fmove(char *src, char *dest)
기능    : 화일을 복사하고 source화일을 삭제한다. 
반환    : 성공시에 0을 에러시에 -1을 반환 
----------------------------------------------------------------------------*/
int fmove( char *src, char *dest)
{
  if (fcopy(src, dest) != 0) return -1;
  unlink(src);
  return 0;
}

/*-------------------------------------------------------------------------
함수명 : xmalloc(size_t size)
기능   : size만큼 메모리를 할당받는 함수 
         이 함수는 GNU REF. GUIDE에 나오는 함수를 그대로 옮겼다.
         물론 주석 하나하나까지도 그냥 옮겼다.
         누구의 말로는 내가 다는 주석 스타일과 여기에 주석을 단 사람의 
         스타일이 비슷하다나...  그래서, 주석을 읽으면서, '무슨 암호
         코드 읽는 그런 암담한 생각이 드는것이 아닌, 정말 시를 사랑하고,
         음유할 줄아는 시(C)인인 듯한 느낌을 받는다' 라나... :)  
---------------------------------------------------------------------------*/
void *xmalloc(size_t size)
{
  //register void *value = (void *)malloc(size);
  void *value = (void *)malloc(size);
  /*
     위라인은 size만큼의 메모리를 할당하려고 malloc을 사용하는 데 그 
     malloc의 리턴값을 *value로 받았네요.
  */
  if (value == 0) {
  /* value가 0이면, 즉 메모리 할당 실패 */
    printf("virtual memory exhausted\n");
    cursor_on();
    exit(1);
  }
  return value;
  /* 할당에 성공하면 블록의 번지가 저장된 값을 리턴 */
}

/*--------------------------------------------------------------------------
함수명 : gnu_getcwd()
기능   : 현재의 디렉토리를 구하는 함수
         GNU REF. GUIDE에서 가져온 함수, 주석은 내가 달았다.
반환   : path명을 반환한다.
---------------------------------------------------------------------------*/
char *gnu_getcwd()
{
  size_t size = 100;
  char *cwd;
 
  /* 메모리를 할당받고 */ 
  cwd = (char *)xmalloc(size);
  while (1) {
    /* 현재의 path명을 구한다. */
    if (getcwd(cwd, size) != NULL) return cwd;
    /* 만일 size가 작다면, 재할당 받아서 다시 시도한다 */
    /* 현재 cwd에 할당된 메모리를 해지시키고 */
    free(cwd);
    /* 크기를 현재의 두배로 변경하고 */ 
    size *= 2;
    /* 두배로 늘린 크기로서 메모리를 다시 할당받는다 */
    cwd = (char *) xmalloc(size);
  }
}

/*-------------------------------------------------------------------------
함수명 : safe_erase(char *src)
기능   : src화일을 휴지통으로 보내고 기존의 화일을 삭제하는 함수
반환   : 성공시 0을, 에러시 -1을 반환
---------------------------------------------------------------------------*/
int safe_erase(char *src)
{
//  char *cwd;
  char buf2[1024];
  char *t;
	struct stat sb;

  /* 사용자 ID가 root라면... 화일을 휴지통에 저장하지 않고 바로 */
  /* 삭제한다.  이것은 일반적(?)으로 root의 홈디렉토리를  시스템 */
  /* 의 파티션에 넣어두는 경우가 많아서... 휴지통 기능을 사용   */
  /* 했을 경우 시스템의 파티션영역이 넘치기 때문에 무조건 삭제  */
  /* 하도록 하였다. 하지만, root의 홈디렉토리를 다른 파티션으로 */
  /* 했다면 아래 4줄은 삭제함이 좋을 듯 싶다..                  */
  /* root이면 더더욱이 휴지통을 사용할수 있도록 해야 될텐데...  */
  /* 실수를 했을 경우 더 큰 재난(?)을 초래할수 있기에...        */
  /* root의 그 막강한 권한과 함께 따라오는 것은 엄청난 재난도   */
  /* 같이 가지고 올수 있는 권한을 가지고 있다는 것을 알고 있어  */
  /* 겠다.. 우리네... 인생에서도 말이다..                       */
#ifdef EXCEPT_ROOT
  if (getenv("USER") && strcmp(getenv("USER"), "root") == 0) {
    unlink(src);
    return 0;
  }
#endif
  /* 여기까지가 root일경우 휴지통 기능을 사용하지 않도록 하였다 */

  if (lstat(src, &sb) < 0) {
    prn_error(src, errno);
    return -1;
  }
  /* 읽은 화일명이 디렉토리거나 symbolic link화일이라면 무시 */
  if (S_ISLNK(sb.st_mode)) {
		unlink(src);
		return 0; 
	}

  t = str_chr_down(src, '.');
  if (t) {
    /* *.wrk *.perr *.err *.tmp *.ck 의 화일은 완전히 삭제 복구불가 */
    if (strcmp(t, ".wrk") == 0 || strcmp(t, ".perr") == 0 ||
        strcmp(t, ".err") == 0 || strcmp(t, ".tmp") == 0  ||
        strcmp(t, ".ck") == 0 ) {
         unlink(src);
         return 0;
    }
  }
  
  /* 휴지통 디렉토리가 없다면 새로 생성한다. 생성오류라면 에러 */
  if (buildup_trash() < 0) return -1;

  /* 휴지통에 새로 생성할 화일명의 fullpath를 구한다. */
  sprintf(buf2, "%s/%s/%s", home,
                            TRASH_FOLDER, 
                            make_trash_file());

  /* 화일을 move 시킨다. 어디로냐... 당연히... 휴지통으로, 당근이지.. */
  if (fmove(src, buf2) != 0) return -1;
  return 0;
}

/*--------------------------------------------------------------------------
함수명:  str_chr(char *s, char ch)
기능  :  strchr(), rindex와 같은 함수로, strchr을 사용하니 프로그램에서
         계속 에러가 발생하여 새로 만들게 되었다.
반환  :  ch문자를 s에서 찾았다면 해당 포인터를 반환, 아니라면 NULL을 반환
---------------------------------------------------------------------------*/
char *str_chr(char *s, char ch)
{
  while (*s) {
    if (*s == ch) return s;
    s++;
  }
  return NULL;
}

/*---------------------------------------------------------------------------
함수명 : prn_reverse_record(int cnt, trash_t *t)
기능   : 
----------------------------------------------------------------------------*/
void prn_reverse_record( int cnt, trash_t *t)
{
  char buf[1024];
  char s[1024];

	if (chkWindowSize(crtline, crtcolumn) < 0) return;

  reverse_scr();
  prn_record(cnt, t);
  norm_scr();
  gotoxy(1, crtline-1);
  sprintf(s, "%s/%s", t->path, t->f_name);
  sprintf(buf, "%%-%d.%ds", crtcolumn, crtcolumn);
  printf(buf, s);
  gotoxy(crtcolumn,crtline);
  fflush(stdout);
}

int message_box( char buf[][100], char option)
{

 struct menu_t  item[4];
  int x1=20, y1= 7;

  int cur_item = 0;
  int  i;
  unsigned int ch;

  beep();
  strcpy(item[0].name, "  Yes "); item[0].enable = 0;
  strcpy(item[1].name, "  No  "); item[1].enable = 0;
  strcpy(item[2].name, "  All "); item[2].enable = 0;
  strcpy(item[3].name, "CANCEL"); item[3].enable = 0;

  msg_box_draw(x1, y1);
  gotoxy(x1+3, y1+2);
  reverse_scr();
  printf("%-49.49s", buf[0]);
  norm_scr();
  gotoxy(x1+3, y1+3);
  printf("%s", buf[1]);
  gotoxy(x1+3, y1+4);
  printf("%s", buf[2]);

  if (option & ITEM_YES) item[0].enable = 1;
  if (option & ITEM_NO)  item[1].enable = 1;
  if (option & ITEM_ALL)  item[2].enable = 1;
  if (option & ITEM_CANCEL)  item[3].enable = 1;

  for (i = 0; i < 4; i++) {
    if (item[i].enable) { cur_item = i; break; }
  }
  
  for (i = 0; i < 4; i++) {
    if (item[i].enable) prn_menu(x1, y1, item[i], i);
  }

  prn_reverse_menu(x1, y1,item[cur_item], cur_item);

  do {
    ch = getch();
    prn_menu(x1, y1, item[cur_item], cur_item);
    switch (ch) {
      case 'h'    :
      case 'H'    :
      case K_LEFT : if (cur_item > 0) {
                     for (i=cur_item-1; i >= 0; i--) {
                       if (item[i].enable) { cur_item = i; break; }
                     }
                  }
                  break;
      case 'l'    :
      case 'L'    :
      case K_RIGHT: if (cur_item < 3) {
                    for (i=cur_item+1; i < 4; i++) {
                      if (item[i].enable) { cur_item= i; break; }
                    }
                  }
                  break;
      case K_ENTER: if (cur_item == 0) return(ITEM_YES);
                  else if (cur_item == 1)  return(ITEM_NO);
                  else if (cur_item == 2)  return ITEM_ALL;
                  else  return ITEM_CANCEL;
                  break;
      case K_ESC :  return ITEM_CANCEL; 
    }
    prn_reverse_menu(x1, y1, item[cur_item], cur_item);
  } while (1);
}

void prn_menu( int x, int y, struct menu_t item, int cur_item)
{
  gotoxy(x+5 + cur_item * 10, y + 7);
  printf("%s", item.name);
  gotoxy(crtcolumn,crtline);
  fflush(stdout);
}

void prn_reverse_menu( int x, int y, struct menu_t item, int cur_item)
{
  reverse_scr();
  prn_menu(x, y, item, cur_item);
  norm_scr();
}

void msg_box_draw(int x, int y)
{
  gotoxy(x, y);    printf("                                                ");
  gotoxy(x, y+ 1); printf(" ┌-─-───────────────────────────────────────┐ ");
  gotoxy(x, y+ 2); printf(" │                                          │ ");
  gotoxy(x, y+ 3); printf(" │                                          │ ");
  gotoxy(x, y+ 4); printf(" │                                          │ ");
  gotoxy(x, y+ 5); printf(" │                                          │ ");
  gotoxy(x, y+ 6); printf(" │                                          │ ");
  gotoxy(x, y+ 7); printf(" │                                          │ ");
  gotoxy(x, y+ 8); printf(" │                                          │ ");
  gotoxy(x, y+ 9); printf(" │%c[7m이동: 좌(H), 우(L)  선택: ENTER  취소:ESC %c[0m│ ", 27, 27);
  gotoxy(x, y+10); printf(" └──────────────────────────────────────────┘ ");
  gotoxy(x, y+11); printf("                                                ");
  gotoxy(crtcolumn,crtline);
  fflush(stdout);
}

int is_file_exist( trash_t *t)
{
  struct stat sb;
  char buf[200];

  sprintf(buf, "%s/%s", t->path, t->f_name);
  if (stat(buf, &sb) != 0) return 0;
  return 1;
}

int file_rebuild( char *src, trash_t *trash)
{
  char buf[1024];
  int f;
  int t;
  int rt;
  int mode;

  sprintf(buf, "%s/%s/%s", home, TRASH_FOLDER, src);
  if ( (f=open(buf, O_RDONLY)) == -1) return -1;

  readline(f, buf, 500);
  readline(f, buf, 500);
  readline(f, buf, 500);
  readline(f, buf, 500);
  mode   = (atol(buf) & S_IREAD )?  0400 : 0000;
  mode  |= (atol(buf) & S_IWRITE)?  0200 : 0000;
  mode  |= (atol(buf) & S_IREAD )?  0100 : 0000;
  mode  |= (atol(buf) & S_IRGRP )?  0040 : 0000;
  mode  |= (atol(buf) & S_IWGRP )?  0020 : 0000;
  mode  |= (atol(buf) & S_IXGRP )?  0010 : 0000;
  mode  |= (atol(buf) & S_IROTH )?  0004 : 0000;
  mode  |= (atol(buf) & S_IWOTH )?  0002 : 0000;
  mode  |= (atol(buf) & S_IXOTH )?  0001 : 0000;

  sprintf(buf, "%s/%s", trash->path, trash->f_name);
  if ( (t=open(buf, O_WRONLY | O_CREAT, mode)) == -1) {
    close(f);
    return -1;
  }
/*  readline(f, buf, 500);*/
  while (1) {
    if ( (rt = read(f, buf, sizeof(buf))) < 1) break;
    if (write(t, buf, rt) < 1) break;
  }
  close(f);
  close(t);
  return 0;
}

int restore_file(trash_t *head, trash_t **p, trash_t **t, int *cnt)
{
//  trash_t *r;
//  char buf[500];
  char msg[4][100];

  if (head == *t) return -1;

  strcpy(msg[0], "  화일을 복구합니다.");
  sprintf(msg[1], "%s/%s", (*t)->path, (*t)->f_name);
  strcpy(msg[2], " 복구하시겠습니까?");
  strcpy(msg[3], "");
  switch (message_box(msg, ITEM_YES|ITEM_NO|ITEM_CANCEL)) {
    case ITEM_NO : 
    case ITEM_CANCEL : return -1;
  }

  if (is_file_exist(*t)) {
    strcpy(msg[0], "  화일이 이미 존재합니다.");
    sprintf(msg[1], "%s/%s", (*t)->path, (*t)->f_name);
    strcpy(msg[2], " 덮어쓰시겠습니까?");
    strcpy(msg[3], "");
    switch (message_box(msg, ITEM_YES|ITEM_NO|ITEM_CANCEL)) {
      case ITEM_NO : 
      case ITEM_CANCEL : return -1;
    }
  }
  else if (dir_rebuild((*t)->path) != 0) return -1;
  if (file_rebuild((*t)->trash, *t) != 0) return -1;

  return 0; 
} 

int delete_file(trash_t *head)
{
  trash_t *t;
  char buf[200];
  int i;

  i = 0;
  t = head->rlink;

  while (t != head) {
    if (t->is_select) {
      sprintf(buf, "%s/%s/%s", home, TRASH_FOLDER, t->trash);
      unlink(buf);
      del_link(head, t);
      i++;
    }
    t = t->rlink;
  }
  return i;
}
 
void help()
{
  int x; int y;
//  char ch;
 
  x = crtcolumn/2-20; y = crtline/2-11;
  gotoxy(x, y+  1); printf("                                                ");
  gotoxy(x, y+  2); printf(" ┌─────────────────────────────────────────────┐ ");
  gotoxy(x, y+  3); printf(" │%c[7m   휴지통  프로그램 V 1.02(98.5 ~ 17.8)판    %c[0m│ ", 27, 27);
  gotoxy(x, y+  4); printf(" │                                             │ ");
  gotoxy(x, y+  5); printf(" │이 프로그램은 삭제된 화일을 원래의 위치로    │ ");
  gotoxy(x, y+  6); printf(" │복구해줍니다.                                │ ");
  gotoxy(x, y+  7); printf(" │                                             │ ");
  gotoxy(x, y+  8); printf(" │@ KEY 정의                                   │ "); 
  gotoxy(x, y+  9); printf(" │  이동키         : 상(K),하(J),좌(H),우(L)   │ "); 
  gotoxy(x, y+ 10); printf(" │  화면 단위이동   : ^F, ^B                   │ ");
  gotoxy(x, y+ 11); printf(" │  항목 선택/취소  : space 로 toggle됨        │ "); 
  gotoxy(x, y+ 12); printf(" │  현재 항목 복구  : Enter                    │ ");
  gotoxy(x, y+ 13); printf(" │  선택한 항목 삭제: DEL, ^D                  │ ");
  gotoxy(x, y+ 14); printf(" │  화일명순   : F5, 'f'  -> 화일명순 소트     │ ");
  gotoxy(x, y+ 15); printf(" │  삭제일순   : F6, 'd'  -> 삭제일순 소트     │ ");
  gotoxy(x, y+ 16); printf(" │                                             │ ");
  gotoxy(x, y+ 17); printf(" │%c[7m               Korea Telecom                 %c[0m│ ", 27, 27);
  gotoxy(x, y+ 18); printf(" └─────────────────────────────────────────────┘ ");
  gotoxy(x, y+ 19); printf("                                                ");
  gotoxy(crtcolumn,crtline);
  fflush(stdout);
  getch();
}


int dir_rebuild(char *path)
{
  char *t, *p;
  char *buf;
  struct stat sb;

  buf = (char *) xmalloc(strlen(path)+1);
  strcpy(buf, path);
 
  p = buf;
  if ( *p == '/' ) p++;
  while ( (t = str_chr(p, '/')) != NULL ) {
    *t++ = 0;
    if (stat(buf, &sb) != 0) {
      if (mkdir(p, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        free(buf);
        return -1;
      }
    }
    *(t-1) = '/';
    p = t;
  }

  if (*p) {
    if (stat(buf, &sb) != 0) {
      if (mkdir(buf, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        free(buf);
        return -1;
      }
    }
  }

  return 0;
} 

int main(int argc, char **argv)
{
  extern int optind;
  int ch;
  char *f;
//  unsigned int gch=0;

  if (!getenv("HOME") || !getenv("USER")) {
    printf("HOME, USER 환경변수가 정의되어 있지 않습니다.\n");
    exit(1);
  }
  strcpy(home, (char *)getenv("HOME"));

  erase_old_file(1);
  rflag = 0;
  while ( (ch = getopt(argc, argv, "dfiRru")) != EOF)
    switch(ch) {
      case 'd' : dflag = 1; break;
      case 'f' : fflag = 1; iflag = 0; break;
      case 'i' : fflag = 0; iflag = 1; break;
      case 'R' : 
      case 'r' : rflag = 1; break;
      case 'u' : unerase(); exit(0);
      case '?' :
      default  : usage();
  }
  argc -= optind;
  argv += optind;
  if (argc < 1) usage();

  checkdot(argv);
  if (!*argv) exit(retval);
  stdin_ok = isatty(STDIN_FILENO);

  // 구문에 대한 경고 문구 때문에, 풀어적음
//  if (rflag)  while ( f = *argv++ ) rmtree(f);
//  else        while ( f = *argv++ ) rmfile(f);  

  if (rflag)  while ( (f = *argv++) != NULL ) rmtree(f);
  else        while ( (f = *argv++) != NULL ) rmfile(f);  

  exit(retval);
}

/*-------------------------------------------------------------------------
함수명 : swap_link(trash_t *x, trash_t *y)
기능   : 두개의 노드의 위치를 바꾸는 역할한다.
인수   : 교환할 두개의 노드를 입력받는다.
--------------------------------------------------------------------------*/
void swap_link( trash_t **x,  trash_t **y)
{
  trash_t *t;

  t = (*y)->rlink; 

  (*y)->llink->rlink = (*y)->rlink;
  (*y)->rlink->llink = (*y)->llink;
 
  (*y)->rlink = (*x);
  (*y)->llink = (*x)->llink;
  (*x)->llink->rlink = (*y);
  (*x)->llink = (*y);

  (*x)->llink->rlink = (*x)->rlink;
  (*x)->rlink->llink = (*x)->llink;

  (*x)->rlink = t;
  (*x)->llink = t->llink;
  t->llink->rlink = (*x);
  t->llink = (*x);

  t = *y;
  *y =  *x;
  *x = t; 
}

/*--------------------------------------------------------------------------
함수명: trash_compare
기능  : 문자열을 비교하여 두 형식의 같은지 다른지를 구한다.
반환  : 같으면 0을 다르면 -, + 값을 반환한다.
인수  : type이 0이면 화일명끼리 비교를 하며,
               그외값이면 날짜로 비교를 한다.
---------------------------------------------------------------------------*/
int trash_compare(trash_t *s1, trash_t *s2, char type)
{
  char buf1[20], buf2[20];

  if ( type == 0) {
    return (strcasecmp(s1->f_name, s2->f_name));
  }
  sprintf(buf1, "%04d%02d%02d%02d%02d%02d", s1->d_time.year, 
                                            s1->d_time.month, 
                                            s1->d_time.day,
                                            s1->d_time.hour, 
                                            s1->d_time.min,   
                                            s1->d_time.sec);
  sprintf(buf2, "%04d%02d%02d%02d%02d%02d", s2->d_time.year, 
                                            s2->d_time.month, 
                                            s2->d_time.day,
                                            s2->d_time.hour, 
                                            s2->d_time.min,   
                                            s2->d_time.sec);
  return(strcmp(buf1, buf2));
}

void sort(trash_t *head, char type)
{
  trash_t *t1, *t2;

  
  t1 = head->rlink;
  t2 = t1->rlink;

  for (t1 = head->rlink; t1->rlink != head; t1 = t1->rlink) {
    for (t2 = t1->rlink; t2 != head; t2=t2->rlink) {
      if (type == 0) {
        if (trash_compare(t1, t2, type) > 0) {
          swap_link(&t1, &t2);
        }
      }
      else {
        if (trash_compare(t1, t2, type) < 0) {
          swap_link(&t1, &t2);
        }
      }
    }
  }
}

         
     
/*---------------------------------------------------------------------------
함수명 : _read(int fd, char *buf, char *cnt, int timeout, char flag)
기능   : 키보드로부터 NONBLOCKING 되게 읽어온다.
---------------------------------------------------------------------------*/
int _read( int fd, char *buf, char cnt, int timeout, char flag)
{
  struct termios tattr;
  int rt;

  tcgetattr(fd, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO);
  tattr.c_cc[VMIN] = 1;
  if (flag) {
    tattr.c_cc[VTIME] = timeout / 1000000;
  }
  else  tattr.c_cc[VTIME] = timeout;
  tcsetattr(fd, TCSAFLUSH, &tattr);
  rt = read(fd, buf, cnt);
  return rt;
} 

/*===========================================================================*/
/* 함  수  명   : _ansi_key                                                  */
/* 제      목   : ANSI   단말기 KEY 입력 처리                                */
/*---------------------------------------------------------------------------*/
unsigned int  _getch(int     timeout, int flag)
{
  int    i=0, rtn;
  char   buf[32];
  unsigned int msg = 0;

  /* 한꺼번에 3byte의 키값까지 읽을수 있다.                                */
  /* 특수키가 눌렸을경우에 3byte의 코드까지 읽도록 하였다.                 */
  /* 만일 특수키토드가 4byte코드까지 있어야 한다면, 아래의 인수를 4로 변경 */
  rtn = _read(0, buf, 3, timeout, flag);
  buf[rtn] = 0;

  if ( rtn <= 0 ) return 0;

  /* 특수키값에 대한 처리 추틴 */
  if ( buf[0] == 0x1b) {  /* ESC or Extended key */ 
      if (buf[1] == 0 || buf[1] == 0x1b) return (0x1b);
  }
  for (i= 0; i < strlen(buf); i++) {
    msg = (msg * 256) + buf[i];
  }
 
  /* 키값을 반환 한다. */
  return msg;
}

/*--------------------------------------------------------------------------
함수명 : getch()
기능   : 하나의 문자를 키보드로 부터 받아오는 함수
반환   : 키보드에서 읽은 키 코드 
--------------------------------------------------------------------------*/
unsigned int getch()
{
  return(_getch(200000,1));
}



/*---------------------------------------------------------------------------
함수명 : getdata(char *prompt, char *buf, int len, enum ECHO_TYPE echo_type)
기능   : 현재 커서위치로 부터 문자열 prompt를 출력하고 문자열 길이len 만큼 
         키보드로 입력받아 buf에 저장하고 입력받은 char수를 변환시켜주는 함수
반환   : 입력된 글자수
          -1 -> ESC나 다른 여러값으로 취소시켰을 경우 발생
변수   : prompt : 출력될 문자열
         buf    : 입력된 키가 저장될 변수
         len    : 입력할 문자열 수
         echotype: echo여부.
                  ECHO_ON 일때 문자 출력되고,
                  ECHO_OFF일때 문자출력이 되지 않고,
                  ECHO_PASSWD일때 문자대신 '*'로 대치되어서 출력됨.
---------------------------------------------------------------------------*/
int getdata(char *prompt, char *buf, int len, enum ECHO_TYPE echo_type)
{
//  int cur_x, cur_y;
  char ch;
  char cur_len=0;


  if (prompt != NULL)  printf("%s", prompt);
  while ( (fflush(stdout), ch=getch() ) != '\r') {
    if (ch == '\n') break;
    if (ch == K_BS || ch == '\177') {
      if (cur_len == 0) continue;
      cur_len--;
      if (echo_type != ECHO_OFF) {
        putchar(K_BS);
        putchar(' ');
        putchar(K_BS);
      }
      continue;
    }

    if (!ISPRINT(ch)) {
      putchar('\007');
      continue;
    }
    if (cur_len >= len) {
      if (echo_type != ECHO_OFF) putchar('\007');
      continue;
    }
/*
    buf[cur_len++] = ch;
    buf[cur_len]   = '\0';
*/
    *(buf+cur_len++) = ch;
    *(buf+cur_len)   = '\0';


    switch (echo_type) {
      case ECHO_ON     : putchar(ch); break;
      case ECHO_OFF    : break;
      case ECHO_PASSWD : putchar('*'); break;
     default          : putchar('@'); break;
    }
  }  /* end of while loop  */

//  buf[cur_len]   = '\0';
  *(buf+cur_len) = '\0';

  if (echo_type != ECHO_OFF) putchar('\n');
  return(cur_len);
}


/*---------------------------------------------------------------------------
함수명 : erase_old_file(int day)
기능   : ~/.trash  디렉토리의 화일중에서  day일 이전의 화일을 삭제한다.
         디스크 full나는 것을 방지하기 위해... 자동 삭제(휴지통 정리)
         를 한다.
         ~/.trash/.erase_date 화일을 검색하여, 삭제작업을 한 최종 날짜 
         를 얻어, 하루에 두번 이상 삭제하는 작업을 하지 않도록 한다.

인수   : day  삭제할 일수.
          0 : 오늘 자료만 남기고 모두 삭제
          1 : 전일 자료까지만 남기고 그 전일 자료는 삭제
          2 : 이틀전 자료까지만 남기고 그 전일 자료는 삭제(오래된 자료 삭제)
---------------------------------------------------------------------------*/
int erase_old_file(int day)
{
  struct stat sb;
  char buf[1024], path[1024];
  int fp;
  DIR *dp;
  struct tm ftime, *t_time;
  struct tm otime;
  time_t f_time, o_time;
  struct dirent *dirp;
  time_t t;
  int rt;

  /* ~/.trash 디렉토리가 있는지를 검사, 없다면 그냥 리턴 */ 
  sprintf(path, "%s/%s", home, TRASH_FOLDER); 
  if ( stat(path, &sb) < 0 ) return 0;

  /*  ~/.trash/.erase_date 화일을 검사하여 등록일이 오늘 날짜라면,       */
  /* 그냥 반환하고,  등록일이 이전 날짜이거나 화일이 존재하지 않다면,    */
  /* 등록일을 오늘 날짜로 변경하고, 이전 날짜 화일 삭제 작업을 진행한다. */

  sprintf(path, "%s/%s/%s", home, TRASH_FOLDER, ERASE_DATE);

  o_time = 0;
  /* erase_date 화일 열기 */
  if ( (fp=open(path, O_RDWR)) != -1 ) {
    rt =read(fp, &o_time, sizeof(o_time));
		if (rt != sizeof(o_time)) {
			close(fp);
			return -1;
		}
  }
  else if ( (fp=open(path, O_RDWR | O_CREAT, 0600)) == -1 ) return -1;

  /* 현재의 시간을 얻음 */ 
  f_time = time(0);

  /* 현재의 시간, 삭제작업한 시간을 구한다. */
  t_time = localtime(&f_time);
  memcpy(&ftime, t_time, sizeof(ftime)); 
  if (o_time != 0) {
    t_time = localtime(&o_time);
    memcpy(&otime, t_time, sizeof(otime));
    /* 만일 작업한 날짜와 현재의 날짜가 같다면 그냥 반환 */
    if ( (ftime.tm_year == otime.tm_year) &&
         (ftime.tm_mon  == otime.tm_mon ) &&
         (ftime.tm_mday == otime.tm_mday) ) {
      close(fp);
      return 0;
    }
  }

  /* 삭제작업을 진행했다는 등록을 한다. 현재 날짜를 기록한다. */
  lseek(fp, 0L, SEEK_SET);
  write(fp, &f_time, sizeof(f_time));
  /* 화일을 닫는다 */
  close(fp);

  /* 삭제 기준일을 구한다. */
  f_time -= day*60*60*24;


  printf("\n");
  reverse_scr();
  printf("%s", HEAD_FILLER);
  norm_scr();
  printf("\n\n");

  printf("휴지통의 %d일전 일자 화일을 자동으로 삭제하고 있습니다.\n", day+1);
  printf("잠시만 기다리세요.... \n\n");
 
  /* .trash 디렉토리를 검색하여,   . .. .erase_date 화일은 스킵하고, */
  /* 나머지 화일은 해당 날짜 일 보다 오래된 것은 삭제한다.           */
  /* 이때 날짜는 현재 일자의 상대 일자로 해당한다.                   */
  sprintf(path, "%s/%s", home, TRASH_FOLDER); 
  if ( (dp = opendir(path)) == NULL ) return -1;

  /* .trash 디렉토리에 화일을 전부 검색한다. */
  while ( (dirp = readdir(dp)) != NULL ) {

    /*  '.', '..', '.erase_date' 라면 무시하고 다음 화일을 찾는다. */
    if (strcmp(dirp->d_name, ".") == 0 ||
        strcmp(dirp->d_name, "..") == 0 ||
        strcmp(dirp->d_name, ERASE_DATE) == 0) continue;

    /* 화일의 생성일(삭제일)을 구한다.  */
    t = atol(dirp->d_name);

    if (t == 0) continue;
    t_time = localtime(&t);
    memcpy(&otime, t_time, sizeof(otime));

    t_time = localtime(&f_time);
    memcpy(&ftime, t_time, sizeof(ftime));
 
    /* 만일 삭제기준일 보다 화일 생성일(삭제일)이 이전날짜이면, */
    /* 화일을 삭제한다.                                         */
    if ((  ftime.tm_year >  otime.tm_year) ||
        (( ftime.tm_year == otime.tm_year) && ( ftime.tm_mon  >  otime.tm_mon )) ||
        (( ftime.tm_year == otime.tm_year) && ( ftime.tm_mon  == otime.tm_mon ) && ( ftime.tm_mday >  otime.tm_mday)) 
			 ) {
      sprintf(buf, "%s/%s", path, dirp->d_name);
      unlink(buf);
    }
  }
  printf("휴지통을 정리하였습니다. \n\n");
  return 0;
}


