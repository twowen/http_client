/**
 ** HTTP-message = start-line
 **                *( header-field CRLF )
 **                CRLF
 **                [ message-body ]
 **
 ** Example:
 **          POST /?id=1 HTTP/1.1
 **          Host: echo.paw.cloud
 **          Content-Type: application/json; charset=utf-8
 **          User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.12; rv:53.0) Gecko/20100101 Firefox/53.0
 **          Connection: close
 **          Content-Length: 136
 **
 **          {
 **             "status": "ok",
 **             "extended": true,
 **             "results": [
 **               {"value": 0, "type": "int64"},
 **               {"value": 1.0e+3, "type": "decimal"}
 **             ]
 **          }
 **
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main (void)
{
	char *host = "noodoe.com";	// target URI
	char * PORT_NUM = "80";		// HTTP port

	char request[0xfff], response[0xfff];
	char *requestLine = "GET / HTTP/1.1\r\n";
	char *headerFmt = "Host: %s\r\n";
	char *CRLF = "\r\n";

	int cfd;					// Socket file descriptor
//	int gaiStatus;				// getaddrinfo status
	struct addrinfo hints;		// hints參數, 設定getaddrinfo()的回傳方式
	struct addrinfo *result;	// getaddrinfo()執行結果的addrinfo結構指標

	size_t bufferLen = strlen(headerFmt) + strlen(host) + 1;
	char *buffer = (char *)malloc(bufferLen);

	strcpy(request, requestLine);
	sprintf(buffer, headerFmt, host);
	strcat(request, buffer);
	strcat(request, CRLF);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;		// 使用IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// 串流Socket
	hints.ai_flags = AI_NUMERICSERV;	// 將getaddrinfo()第2參數(PORT_NUM)視為數字

	// 由hostname取得IP address
	if (getaddrinfo(host, PORT_NUM, &hints, &result) != 0) {
        perror("getaddrinfo");
        goto LABEL_FREE_MEM;
        exit(EXIT_FAILURE);
	}

	// Create socket
	cfd = socket(result->ai_family, result->ai_socktype, 0);
	if (cfd == -1) {
        perror("socket");
        goto LABEL_RELEASE_ADDRI;
        exit(EXIT_FAILURE);
	}

	// Create connection
	if (connect(cfd, result->ai_addr, result->ai_addrlen) < 0) {
		perror("connect");
		goto LABEL_CLOSE_FD;
		exit(EXIT_FAILURE);
	}

	// 格式化輸出請求訊息
	printf("----------\nRequest:\n----------\n%s\n", request);

	// 發送請求
	if (send(cfd, request, strlen(request), 0) < 0) {
        perror("send");
        goto LABEL_EXIT_ERR;
		exit(EXIT_FAILURE);
	}

	// 接收回應
	if (recv(cfd, response, 0xfff, 0) < 0) {
        perror("recv");
        goto LABEL_EXIT_ERR;
		exit(EXIT_FAILURE);
	}

	// 格式化輸出回應訊息
	printf("----------\nResponse:\n----------\n%s\n", response);

	sleep(1);

LABEL_EXIT_ERR:
LABEL_CLOSE_FD:
	//shutdown(cfd, SHUT_WR);
	close(cfd);

LABEL_RELEASE_ADDRI:
	// 釋放getaddrinfo (linked list)記憶體空間
	freeaddrinfo(result);
	result = NULL;

LABEL_FREE_MEM:
	free(buffer);
	buffer = NULL;
}
