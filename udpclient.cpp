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
#include <sstream>
using namespace std;
#define PORT 8001

int arrayAllTrue(bool arr[], int n) {
	for (int i = 0; i<n; i++)
		if (arr[i] == false)
			return i;
	return -1;
}

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

	// Send initial data to server: filename, filesize
	ostringstream oss;
	oss << argv[1] << " " << datavec.size();
	int leng = sendto(sock, oss.str().c_str(), oss.str().size(), 0, (struct sockaddr*)&server_address, sizeof(server_address));
	if (leng < 0) {
		printf("Send error!\n");
		return -1;
	}

	// Big octoblock sending loop
	int octoblockcursor = 0;
	int octoblocks = datavec.size() / 8888 + 1;
	while (octoblockcursor < datavec.size()) {
		bool octolegACK[8] = {false,false,false,false,false,false,false,false};
		int octolegsize = 1109;
		if (octoblockcursor + 8888 > datavec.size())
			octolegsize = (datavec.size() - octoblockcursor) / 8;
		// cout << "Cursor at: " << cursor << endl;
		cout << "\n\nNew Octoblock with Octoleg size: " << octolegsize << endl;

		// Octoleg sending loop
		char sendbuffer[octolegsize + 2];
		memset(sendbuffer, 0, octolegsize+2);
		for (int j = 0; j < 8; ) {
			int cursor = octoblockcursor + octolegsize*j;
			sendbuffer[0] = j; // Left shift operator to indicate octoleg
			strncpy(sendbuffer+1, &(datavec[cursor]), octolegsize);
			cout << "\nSending: '" << sendbuffer+1 << "'" << endl;

			// Send data to server
			int len = sendto(sock, sendbuffer, octolegsize + 1, 0, (struct sockaddr*)&server_address, sizeof(server_address));
			printf("message has been sent to server\n");

			// Receive ACK back from server
			char ACK[2];
			int recv_bytes=recvfrom(sock, ACK, 1, 0, NULL, NULL);
			ACK[1] = '\0';
			printf("Server recieved octoleg: '%d'\n", ACK[0]);
			octolegACK[ACK[0]] = true;

			// Check to make sure all octolegs were ACK'd by server
			if (arrayAllTrue(octolegACK, 8) != -1) {
				j = arrayAllTrue(octolegACK, 8);
			} else {
				j = 8;
			}

			// cursor += octolegsize;
		} // End of octoleg sending loop
		octoblockcursor += 8*octolegsize;
	} // End of octoblock sending loop

	// Signal end of octoblocks
	// char endsignal[1];
	// endsignal[0] = 0x11;
	// sendto(sock, endsignal, sizeof(endsignal), 0, (struct sockaddr*)&server_address, sizeof(server_address));
	// cout << "All octoblocks of file sent, signalling end to server" << endl;

	// close the socket and input file
	close(sock);
	infile.close();
	return 0;
}
