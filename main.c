/**
 * EventService is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * EventService is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EventService.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <linux/input.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#define BITS (sizeof(long)*8)

void sendKey(int fd, uint16_t key, bool pressed) {
	struct input_event ev;
	memset(&ev, 0, sizeof(struct input_event));
	ev.type = EV_KEY;
	ev.code = key;
	ev.value = pressed ? 1 : 0;

	write(fd, &ev, sizeof(struct input_event));
}

void sendSync(int fd) {
	struct input_event ev;
	memset(&ev, 0, sizeof(struct input_event));
	ev.type = EV_SYN;

	write(fd, &ev, sizeof(struct input_event));
}

int main (int argc, char **argv)
{
	int fd;
	int i;
	unsigned long bit[EV_MAX];
	int port = 0;
	bool error = false;
	int reqs;
	char buf[256];

	/**
	 * Search for a /dev/input/eventX (where X starts at zero and goes up)
	 * which reports EV_KEY, EV_REP and EV_LED.
	 * Once found, it is used for issuing keyboard commands.
	 */
	while (!error) {
		reqs = 0;
		sprintf(buf, "/dev/input/event%d", port);
		if ((fd = open(buf, O_RDWR)) < 0) {
			error = true;
		} else {
			strcpy(buf, "**Unknown**");
			ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
			memset(bit, 0, sizeof(bit));
			ioctl(fd, EVIOCGBIT(0, EV_MAX), bit);
			for (i = 0; i < EV_MAX; i++) {
				if ((bit[i/BITS] >> (i%BITS)) & 0x01) {
					reqs += (i == EV_KEY);
					reqs += (i == EV_REP);
					reqs += (i == EV_LED);
				}
			}
			if (reqs == 3) {
				printf("Using input device \"%s\" on /dev/input/event%d\n", buf, port);
				break;
			}
			close(fd);
		}
		port++;
	}
	if (error)
		exit(1); // No access
	if (reqs != 3)
		exit(2); // No keyboard

	// Setup UDP server
	int s;
	struct sockaddr_in srvaddr;
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)	
		exit(3);
	memset(&srvaddr, 0, sizeof(struct sockaddr_in));
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(5050);
	srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);	

	if (bind(s, (struct sockaddr*)&srvaddr, sizeof(struct sockaddr_in)) == -1)
		exit(4);

	int len;
	struct sockaddr_in client;
	int slen = sizeof(struct sockaddr_in);
	char udp[256];


	/**
	 * Expects to get a UDP packet prefixed with 0xDEADBEEF followed by 0x0000-0xFFFF depending on
	 * KEY_* you want to press.
	 *
	 * This is NOT meant to run on a public network
	 */
	while (true) {
		if ((len = recvfrom(s, udp, 256, 0, (struct sockaddr *) &client, &slen)) == -1)   // read datagram from server socket
			exit(5); // Network errors
		//printf("Received %d bytes\n", len);
		// CHeck that we have some sensible magic
		if (len == 6 && udp[0] == 0xDE && udp[1] == 0xAD && udp[2] == 0xBE && udp[3] == 0xEF) {
			uint16_t e = (udp[4] << 8) | udp[5];
			//printf("Valid input from client, EV to send: %04x\n", e);
			sendKey(fd, e, true);
			sendSync(fd);
			sendKey(fd, e, false);
			sendSync(fd);
		} else
			printf("Invalid packet, expected 6 bytes got %d\n", len);
	}

	// Should never end up here :)
	exit(0);
}