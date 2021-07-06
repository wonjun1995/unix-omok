#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define PORTNUM 9000
#define CLIENTNUM 3 // 클라이언트가 1명일 때 소켓기술자의 값은 3
#define USERNUM 2	// user구조체 배열의 갯수
#define NONE 0		// NONE, BLACK, WHITE 바둑판의 값들을 설정하기 위한 매크로
#define BLACK 1
#define WHITE 2

struct opt {		// 옵션을 담을 구조체 정의 및 선언
	int u;			// u옵션의 set/clear를 확인해 주는 flag
	int p;			// p옵션의 set/clear를 확인해 주는 flag
}opt;

struct user {		// 로그인 기능을 구현하기 위한 구조체 정의
	char name[20];
	char pwd[20];
};

struct user user[USERNUM] = {{ // 로그인 데이터 선언
	.name = "jht",
	.pwd = "1234"
},{
	.name = "jwj",
	.pwd = "qwer"
}};

int start = 0;				// 타이머 구현을 위한 전역변수

/*
바둑의 승리조건을 확인하기 위한 함수이다.
*/
int Win_Check(int Board[][19]) {
   int i;                                          
   int j;
   for (i = 2; i < 18; i++) {                     // 가로 또는 세로를 검사하기 위한 반복문
      for (j = 0; j < 20; j++) {                  
         if (Board[j][i - 2] == 1 && Board[j][i - 1] == 1 && Board[j][i] == 1 && Board[j][i + 1] == 1 && Board[j][i + 2] == 1) {   // 가로방향
            printf("*** 흑돌 승리 ***\n");
            return 1;
         }
         else if (Board[j][i - 2] == 2 && Board[j][i - 1] == 2 && Board[j][i] == 2 && Board[j][i + 1] == 2 && Board[j][i + 2] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }

         else if (Board[i - 2][j] == 1 && Board[i - 1][j] == 1 && Board[i][j] == 1 && Board[i + 1][j] == 1 && Board[i + 2][j] == 1) {   // 세로방향
            printf("*** 흑돌 승리 ***\n");
            return 1;
         }
         else if (Board[i - 2][j] == 2 && Board[i - 1][j] == 2 && Board[i][j] == 2 && Board[i + 1][j] == 2 && Board[i + 2][j] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }
      }
   }

   for (i = 2; i < 18; i++) {                  // 대각선
      for (j = 2; j < 18; j++) {               
         if (Board[j - 2][i - 2] == 1 && Board[j - 1][i - 1] == 1 && Board[j][i] == 1 && Board[j + 1][i + 1] == 1 && Board[j + 2][i + 2] == 1) { //왼쪽 위에서 오른쪽 밑으로 내려가는 대각선
            return 1;
         }
         else if (Board[j - 2][i - 2] == 2 && Board[j - 1][i - 1] == 2 && Board[j][i] == 2 && Board[j + 1][i + 1] == 2 && Board[j + 2][i + 2] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }

         else if (Board[j + 2][i - 2] == 1 && Board[j + 1][i - 1] == 1 && Board[j][i] == 1 && Board[j - 1][i + 1] == 1 && Board[j - 2][i + 2] == 1) { // 왼쪽 아래에서 오른쪽 위로 올라가는 대각선
            printf("*** 흑돌 승리 ***\n");
            return 1;
         }
         else if (Board[j + 2][i - 2] == 2 && Board[j + 1][i - 1] == 2 && Board[j][i] == 2 && Board[j - 1][i + 1] == 2 && Board[j - 2][i + 2] == 2) {
            printf("*** 백돌 승리 ***\n");
            return 1;
         }
      }
   } //조건이 갖춰지면 1, 아니면 0.
   return 0;
}

/*
오목판을 print하는 함수이다.
*/
void print_board(int board[][19]){
	int i, j;
	system("clear");
	printf("123456789ABCDEFGHIJ\n");
	for (i = 0; i < 19; i++) {
         for (j = 0; j < 19; j++) {
            if (board[i][j] % 3 == BLACK) {
               printf("●");
            }
            else if (board[i][j] % 3 == WHITE) {
               printf("○");
            }
            else {
               printf("□");
            }
         }
         printf("  %d\n",i+1);
      }
}

/*
들어온 입력을 토큰화 시켜 board에 이미 돌이 놓여져 있는 경우 다시 입력을 받는 함수
*/
void put_board(char * buf, int board[][19], int *i,int *j){
	char temp[BUFSIZ];
	char* ptr;
	char *temp_ptr;
	int n, m;
	
	while(1){
		strcpy(temp, buf);							// strtok() 함수는 원래의 문자열을 수정하기 때문에 temp변수에 복사해준다.
		if(!(temp[1] == ',' || temp[2] == ',')){	// 문자열이 ','로 이루어지지 않았다면 다시 입력을 받는다.
			if(!strcmp(temp, "quit")){				// 문자열이 quit이라면 종료한다.
				return;
			}
			printf("','를 이용해 입력해 주세요.\n");
			scanf("%s", buf);
			continue;
		}
		ptr = strtok(temp, ",");					// ','를 구분자로 사용해 temp의 값을 나눈다.
		n = (int)strtol(ptr, &temp_ptr, 10);
		ptr = strtok(NULL, ",");
		m = (int)strtol(ptr, &temp_ptr, 10);
		
		n--;	// 배열은 0~18의 값을 사용하기 때문에 하나씩 값을 빼준다.
		m--;
		if(n < 0 || n > 18){
			printf("둘 수 있는 좌표값의 범위를 초과하였습니다.\n");
    		printf("착수하여 주세요. 행:1~19   열:1~19\n");
    		scanf("%s", buf);
			continue;
		}
		if(m < 0 || m > 18){
			printf("둘 수 있는 좌표값의 범위를 초과하였습니다.\n");
    		printf("착수하여 주세요. 행:1~19   열:1~19\n");
    		scanf("%s", buf);
			continue;
		}
		if(board[n][m] != NONE){					// 이미 board에 BLACK이나 WHITE가 존재한다면 다시 입력을 받는다.
			printf("돌이 이미 존재합니다. 다른 곳에 놓아주세요.\n");
    		printf("착수하여 주세요. 행:1~19   열:1~19\n");
    		scanf("%s", buf);
			continue;
		}
		break;
	}
	// set_board()함수가 i, j값을 사용하기 때문에 call by reference로 값을 바꿔준다.
	*i = n;
	*j = m;
}

/*
i, j의 값으로 board의 값을 세팅한다.
count가 짝수라면 BLACK, 홀수라면 WHITE값을 board에 넣는다.
*/
void set_board(int board[][19],int count, int i, int j){
	
	if ((count % 2) == 0) {
        	if (board[i][j] == NONE) {
            	board[i][j] = BLACK;
        	}
    	}
    	else {
        	if (board[i][j] == NONE) {
            	board[i][j] = WHITE;
        	}
    	}
}

/*
send의 예외처리와 quit이 입력되면 종료하는 함수.
*/
void my_send(int sock, char* msg){			// 소켓기술자 저장
	if(send(sock, msg, BUFSIZ , 0)==-1){	// 메세지를 보낸다.
		perror("send");
		exit(1);
	}
}

/*
recv의 예외처리를 포함한 함수
방은 메세지값이 quit이라면 1을 리턴한다.
*/
int my_recv(int sock, char *buf){
	int n;
	if((n = recv(sock, buf, BUFSIZ, 0)) == -1){
		perror("recv");
		exit(1);
	}
	buf[n] = '\0';
	if(!strcmp(buf, "quit")){
		printf("상대방이 기권하였습니다.\n");
		return 1 ;
	}
	return 0;
}

/*
소켓을 얻어오고 connect()함수를 모아놓은 함수
*/
void connect_request(int *sockfd, struct sockaddr_in *server_addr)
{
	// socket을 얻어옴
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	memset(server_addr->sin_zero, 0, sizeof(server_addr->sin_zero));
	
	server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(PORTNUM);
	server_addr->sin_addr.s_addr = inet_addr("127.0.0.1");
	
	if(connect(*sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}
}

void * omok(void* sd){
	int sockfd = (intptr_t)sd;
	char buf[BUFSIZ], name[BUFSIZ], pwd[BUFSIZ];
	int board[19][19];								// 오목판 배열 선언
	int count = 0, j = 0, i = 0;					// count : 흑돌 백돌을 정하기 위한 변수
	int player = 0; 								// 옵션의 구현을 위한 변수 c와 로그인여부를 확인하는 sign변수를 선언한다.
	
	//바둑판 초기화
	for (i = 0; i < 19; i++) {
		for (j = 0; j < 19; j++) {
			board[i][j] = 0;
		}
	}
	system("clear");
	
	my_recv(sockfd, buf);						// client 0 : 상대방을 기다리는 중입니다. | client 1 : 대국을 시작합니다.
	printf("%s",buf);
	if(strcmp(buf,"상대방을 기다려주세요.\n")){ // client 1 인 경우 
		print_board(board);						// 오목판을 프린트한다.
		printf("상대방의 차례입니다.\n");	
		player = 1;								// client 1 이기 때문에 player를 1로 셋한다.
		start = 1;
	}
	if(my_recv(sockfd, buf)){					// client 0 : 대국을 시작합니다 .	| client 1 : 상대방의 메세지
		start = 2;								// timer쓰레드를 종료시키기 위해 start 변수를 2로 설정해 준다.
		return 0;
	}	
	print_board(board);
	start = 1;
	if(player){									// client 1인경우 상대방의 메세지를 받기 때문에
		put_board(buf, board, &i, &j);			// put_board() 와 set_board()를 해준다.
		set_board(board, count, i, j);			
		count++;								// client의 차례가 지났으니 count하나 더해준다.
	}else{
		printf("%s",buf);						// client 0인경우 "대국을 시작합니다."라는 메세지를 출력한다.
	}
	
	while(1){
		print_board(board);
		if (Win_Check(board)) {					// 승리조건을 체크한다.;
        	break;								// 승리시 while문 탈출
    	}
		
		printf("사용자의 차례입니다.\n");	
		printf("돌을 둘 좌표를 입력하세요 행:1~19   열:1~19 ex) 13,5\n");
    	printf("종료를 원하시면 quit을 적어주세요\n");
		
		scanf("%s",buf);						// 보낼 내용을 입력받는다.
		
		put_board(buf, board,&i, &j);
		set_board(board, count, i, j);
		count++;
		
		print_board(board);
		
		printf("상대방의 차례입니다.\n");	
		my_send(sockfd, buf);	
		if(!strcmp(buf, "quit")){				// 보낸 내용이 quit이라면 서버에 quit 메세지를 보내고 종료한다.
			printf("기권하였습니다.\n");
			break ;
		}
		if (Win_Check(board)) {					// send()보다 먼저 win_check()시
    		break;								// socket을 닫아버리는데 다른 client가 recv()기다리기때문에
    	}										// seg fault발생한다. 따라서 send() 뒤에서 체크한다. 
		
		if(my_recv(sockfd, buf)){
			break;
		}
		put_board(buf, board,&i, &j);
		set_board(board, count, i, j);
		count++;
	}
	start = 2;									// omok()쓰레드 종료 전 timer쓰레드를 종료시키기 위해 start 변수를 2로 설정해 준다.
	return 0;
}

/*
타이머를 구현한 쓰레드
*/
void * counting(void * arg){
	int timer = 0;
	while(1){
		if(start == 1){
			timer++;
			sleep(1);
		}
		else if(start == 2){
			break;
		}
	}
	printf("총 대국 시간 : %d초\n",timer);
	return 0;
}
int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in server_addr;
	pthread_t omok_thread, counting_thread;
	int thread_result;
	char buf[BUFSIZ], name[BUFSIZ], pwd[BUFSIZ];
	int i = 0;
	int c, sign= 0;								// 옵션의 구현을 위한 변수 c와 로그인여부를 확인하는 sign변수를 선언한다.

	extern char* optarg;
	extern int optind;
	
	while ((c = getopt(argc, argv, "up")) != -1) {
		switch (c) {
		case 'u': // 
			if(!argv[optind]) {
				printf("아이디를 입력해 주세요.\n");
				printf("./omok -up [username] [pwd]\n");
				return 0;
			}
			opt.u =1;
			break;
		case 'p' :
			if(!argv[optind]) {
				printf("아이디를 입력해 주세요.\n");
				printf("./omok -up [username] [pwd]\n");
				return 0;
			}
			if(!argv[optind + 1]) {
				printf("비밀번호를 입력해 주세요.\n");
				printf("./omok -up [username] [pwd]\n");
				return 0;
			}
			opt.p =1;
			break;
		default:
			break;
		}
	}
	if(!opt.u && opt.p){
		printf("option 'u'와 함께 사용해야 합니다.\n");
		return 0;
	}
	if(!opt.u) {
		printf("아이디를 입력하시오.\n");
		scanf("%s", name);
	}
	for(i =0 ; i < USERNUM; i++){
		if(!strcmp(opt.u ? argv[optind] : name, user[i].name)){ // 아이디가 존재할 경우
			if(!opt.p){
				printf("비밀번호를 입력하시오.\n");
				scanf("%s", pwd);
			} 
			if(!strcmp(opt.p ? argv[optind + 1] : pwd, user[i].pwd)){
				sign = 1;// 통과
			}
			else { // 비밀번호가 틀림
				printf("비밀번호가 틀렸습니다.\n");
				if(opt.p) printf("./omok -up [username] [pwd]\n");
				return 0;
			}
		}
	}
	if(!sign){ // 아이디가 존재하지 않을경우
		printf("등록된 아이디가 존재하지 않습니다.\n");
		return 0;
	}
	
	
	connect_request(&sockfd, &server_addr);
	pthread_create(&omok_thread, NULL, omok, (void *)(intptr_t)sockfd);
	pthread_create(&counting_thread, NULL, counting, NULL);
	
	pthread_join(omok_thread, (void **)&thread_result);
	pthread_join(counting_thread, (void **)&thread_result);

	printf("client-quited\n");
	close(sockfd);
	return 0;
}
