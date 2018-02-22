#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
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
	ifstream infile(argv[1]);
	vector <char> datavec;
	string line;
	while (getline(infile, line)) {
		for (char &c : line)
			datavec.push_back(c);
		datavec.push_back('\n');
	}
	cout << "File read into memory\n";

	// test data that will be sent to the server
	// const char* data_to_send = "Hi Server!!!";

	//////////////////////////////////////////////////////////////////////////////

	// Big octoblock sending loop
	int cursor = 0;
	int octoblocks = datavec.size() / 8888 + 1;
	while (cursor < datavec.size()) {
		int octolegsize = 1109;
		if (cursor + 8888 > datavec.size())
			octolegsize = (datavec.size() - cursor) / 8;
		cout << "Cursor at: " << cursor << endl;
		cout << "\n\nNew Octoblock with Octoleg size: " << octolegsize << endl;
		// Octoleg sending loop
		char sendbuffer[octolegsize + 2];
		memset(sendbuffer, 0, octolegsize+2);
		for (int j = 0; j < 8; j++) {
			sendbuffer[0] = (1 << j); // Left shift operator to indicate octoleg
			strncpy(sendbuffer+1, &(datavec[cursor]), octolegsize);
			cout << "\nSending: '" << sendbuffer+1 << "'" << endl;

			// Send data to server
			int len = sendto(sock, sendbuffer, octolegsize + 1, 0, (struct sockaddr*)&server_address, sizeof(server_address));
			printf("message has been sent to server\n");

			// received echoed data back
			char buffer[1111];
			memset(buffer, 0, 1111);
			int recv_bytes=recvfrom(sock, buffer, len, 0, NULL, NULL);
			printf("Server received bytes = %d\n",recv_bytes);
			buffer[len] = '\0';
			printf("Server recieved: '%s'\n", buffer);
			cursor += octolegsize;
		} // End of octoleg sending loop
	} // End of octoblock sending loop

	// close the socket and input file
	close(sock);
	infile.close();
	return 0;
}
