#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define MAX_CLIENT_CNT 10000

char PORT[6];
int server_sock;
int client_sock[MAX_CLIENT_CNT] = { 0 };
struct sockaddr_in client_addr[MAX_CLIENT_CNT] = { 0 };

pthread_t tid[MAX_CLIENT_CNT];
int exitFlag[MAX_CLIENT_CNT];

pthread_mutex_t mlock;

char* html_form = "<!DOCTYPE html>\n"
"<html>\n"
"<head>"
"<meta charset=\"UTF-8\">\n"
"<title>HTML Form</title>"
"</head>\n"
"<body>\n"
" <h2>Chatting</h2>\n"
"<div style=\"display:flex\">\n"
"<form action=\"http://ydh.o-r.kr:12345\" method=\"get\">\n"
"  <input type=\"text\" name=\"nickname\" placeholder=\"Input name\" autocomplete=\"off\" required >\n"
"  </br>"
"  <input type=\"text\" name=\"msg\" placeholder=\"Input Message\" required>"
"  <input type=\"submit\" value=\"Submit\">\n"
"</form>\n"
"<div style=\"width:500px; height:500px; border:1px solid black; position:relative;\">"; 


char* html_form2 =" ";


char chat_log[1000][MAX_CLIENT_CNT];
int log_idx;


void interrupt(int arg) {
	printf("\nYou typed Ctrl + C\n");
	printf("Bye\n");

	for (int i = 0; i < MAX_CLIENT_CNT; i++) {
		if (client_sock[i] != 0) {
			pthread_cancel(tid[i]);
			pthread_join(tid[i], 0);
			close(client_sock[i]);
		}
	}
	close(server_sock);
	exit(1);
}

void removeEnterChar(char* buf) {
	int len = strlen(buf);
	for (int i = len - 1; i >= 0; i--) {
		if (buf[i] == '\n') {
			buf[i] = '\0';
			break;
		}
	}
}

int getClientID() {
	for (int i = 0; i < MAX_CLIENT_CNT; i++) {
		if (client_sock[i] == 0)
			return i;
	}
	return -1;
}

void* client_handler(void* arg) {
	int id = *(int*)arg;

	char client_IP[100];

	strcpy(client_IP, inet_ntoa(client_addr[id].sin_addr));
	printf("INFO :: Connect new Client (ID : %d, IP : %s)\n", id, client_IP);

	char buf[MAX_CLIENT_CNT] = { 0 };

	write(client_sock[id], "HTTP/1.1 200 OK\nContent-Type: text/html\n\n",
		strlen("HTTP/1.1 200 OK\nContent-Type: text/html\n\n"));
	write(client_sock[id], html_form, strlen(html_form));

	while (1) {

		memset(buf, 0, MAX_CLIENT_CNT);
		int len = read(client_sock[id], buf, MAX_CLIENT_CNT);

		if (len == 0) {
			printf("INFO :: Disconnect with client.. BYE\n");
			exitFlag[id] = 1;
			break;
		}

		if (!strcmp("exit", buf)) {
			printf("INFO :: Client want close.. BYE\n");
			exitFlag[id] = 1;
			break;
		}

		removeEnterChar(buf);
		pthread_mutex_lock(&mlock);

		chat_log[log_idx][0] = '[';
		strncpy(chat_log[log_idx] + 1, buf + 15, (int)(strstr(buf, "msg=") - buf - 16));
		strcat(chat_log[log_idx], "]    ");
		char msg[1000];
		memset(msg, 0, 1000);
		strncpy(msg, buf + (int)(strstr(buf, "msg=") - buf) + 4, (int)(strstr(buf, " HTTP/1.1") - strstr(buf, "msg=")) - 4);

		strcat(chat_log[log_idx], msg);

		printf("%d\n", log_idx);

		log_idx++;

		if (client_sock[id] != 0) {
			if (log_idx > 20) {
				for (int j = log_idx - 20; j < log_idx; j++) {
					write(client_sock[id], chat_log[j], strlen(chat_log[j]));
					write(client_sock[id], "</br>", strlen("</br>"));
				}
			}
			else {
				for (int j = 0; j < log_idx; j++) {
					write(client_sock[id], chat_log[j], strlen(chat_log[j]));
					write(client_sock[id], "</br>", strlen("</br>"));
				}
			}
		}

  for (int i = 0; i < 10; i++) {
			if (i != id) {
				write(client_sock[i], chat_log[log_idx-1], strlen(chat_log[log_idx - 1]));
				write(client_sock[i], "</br>", strlen("</br>"));
			}
		}

		
		//write(client_sock[id], html_form2, strlen(html_form2));
		pthread_mutex_unlock(&mlock);

	}

	close(client_sock[id]);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("ERROR Input Port Num\n");
		exit(1);
	}
	strcpy(PORT, argv[1]);

	signal(SIGINT, interrupt);
	pthread_mutex_init(&mlock, NULL);

	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock == -1) {
		printf("ERROR :: 1_Socket Create Error\n");
		exit(1);
	}
	printf("Server On..\n");

	int optval = 1;
	setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(optval));

	struct sockaddr_in server_addr = { 0 };
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(PORT));

	if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		printf("ERROR :: 2_bind Error\n");
		exit(1);
	}
	printf("Bind Success\n");

	if (listen(server_sock, 5) == -1) {
		printf("ERROR :: 3_listen Error");
		exit(1);
	}
	printf("Wait Client...\n");

	socklen_t client_addr_len = sizeof(struct sockaddr_in);
	int id_table[MAX_CLIENT_CNT];
	while (1) {

		int id = getClientID();
		id_table[id] = id;

		if (id == -1) {
			printf("WARNING :: Client FULL\n");
			sleep(1);
			continue;
		}

		memset(&client_addr[id], 0, sizeof(struct sockaddr_in));

		client_sock[id] = accept(server_sock, (struct sockaddr*)&client_addr[id], &client_addr_len);
		if (client_sock[id] == -1) {
			printf("ERROR :: 4_accept Error\n");
			break;
		}

		pthread_create(&tid[id], NULL, client_handler, (void*)&id_table[id]);

		for (int i = 0; i < MAX_CLIENT_CNT; i++) {
			if (exitFlag[i] == 1) {
				exitFlag[i] = 0;
				pthread_join(tid[i], 0);
				client_sock[i] = 0;
			}
		}
	}
	pthread_mutex_destroy(&mlock);
	close(server_sock);
	return 0;
}
