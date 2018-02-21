#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;
#define PORT 8001
int main(int argc, char *argv[]) {
	// Check command line arguments
	if (argc != 2) {
		printf("Invalid arguments, format is: ./executable file.txt");
		exit(-1);
	}

	const char* server_name = "localhost"; //loopback
	const int server_port = PORT;
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// creates binary representation of server name
	// and stores it as sin_addr
	// inet_pton: convert IPv4 and IPv6 addresses from text to binary form

	inet_pton(AF_INET, server_name, &server_address.sin_addr);
	server_address.sin_port = htons(server_port);

	// open socket
	int sock;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("could not create socket\n");
		return 1;
	}
	printf("client socket created\n");

	// Open file to send
	// printf(argv[1]);
	ifstream infile(argv[1]);
	vector <char> datavec;
	char tmp;
	while (!infile.eof()) {
		infile >> tmp;
		datavec.push_back(tmp);
	}
	for (int i = 0; i < datavec.size(); i++)
		cout << datavec[i];

	// test data that will be sent to the server
	const char* data_to_send = "Hi Server!!!";

	// send data loop
	for (int i=0; i<3; i++) {
		int len = sendto(sock, data_to_send, strlen(data_to_send), 0, (struct sockaddr*)&server_address, sizeof(server_address));
		printf("message has been sent to server\n");

		// received echoed data back
		char buffer[100];
		int recv_bytes=recvfrom(sock, buffer, len, 0, NULL, NULL);
		printf("received bytes = %d\n",recv_bytes);
		buffer[len] = '\0';
		printf("recieved: '%s'\n", buffer);
	}
	// End of sending loop

	// close the socket and input file
	close(sock);
	infile.close();
	return 0;
}
