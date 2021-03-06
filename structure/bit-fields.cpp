#include <catch.hpp>
#include <iostream>

struct frame_header_struct
{
	unsigned char frame_start;
	unsigned int sender_id;
	unsigned char sender_type;
	unsigned char protocol_type;
	union
	{
		struct
		{
			unsigned char priority		: 4, //value of Enum Priority defined above.
						  sequence		: 4; 
		};
		unsigned char frame_seq;
	};

	unsigned char event_type;
	unsigned short int event_sub_type;
	unsigned int transfer_id;
	unsigned char transfer_type;
	unsigned int receiver_id;
	unsigned char receiver_type;
	unsigned short int data_length;
	unsigned char frame_end;
};

TEST_CASE("frame_header_struct", "frame_header_struct")
{
	frame_header_struct packet_in;
	packet_in.frame_seq = 0x34;

	unsigned char priority = packet_in.priority;
	unsigned char sequence = packet_in.sequence;

	packet_in.frame_seq = (priority << 4) | sequence;

	unsigned char p = packet_in.priority;
	unsigned char s = packet_in.sequence;

	printf("%d %d %d %d", priority, sequence, p, s);
	getchar();
}

#pragma pack(push, 1)
struct ST_TEST
{
	unsigned char	ucA : 1;
	unsigned char	ucB : 1;
	unsigned char	ucC : 1;
	unsigned char	ucD : 1;
	unsigned char	ucE : 1;
	unsigned char	ucF : 1;
	unsigned char	ucG : 1;
	unsigned char	ucH : 1;
};
#pragma pack(pop)

TEST_CASE("ST_TEST", "ST_TEST")
{
	ST_TEST stTest;
	stTest.ucA = 1;
	stTest.ucB = 0;
	stTest.ucC = 0;
	stTest.ucD = 0;
	stTest.ucE = 0;
	stTest.ucF = 0;
	stTest.ucG = 1;
	stTest.ucH = 0;
	unsigned char ucTest;
	memcpy(&ucTest, &stTest, 1);

	printf("%x", ucTest);
	getchar();
}