/*
   Console for car CAN bus simulator
   Copyright(c) 2023 Adhokshaj Mishra <me@adhokshajmishraonline.in>
*/

#include <cstdlib>
#include <cstring>

#include <chrono>
#include <iostream>
#include <thread>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

struct CanMessage final
{
    struct ID final
    {
        inline static int Door = 411;
        inline static int Signal = 392;
        inline static int Speed = 580;
    };

    struct Position final
    {
        inline static int Door = 2;
        inline static int Signal = 0;
        inline static int Speed = 3;
    };

    struct Length final
    {
        inline static int Door = Position::Door + 1;
        inline static int Signal = Position::Door + 1;
        inline static int Speed = Position::Speed + 2;
    };

    struct Equipment final
    {
        inline static int LeftSignal = 1;
        inline static int RightSignal = 2;
        inline static int Door1 = 1;
        inline static int Door2 = 2;
        inline static int Door3 = 4;
        inline static int Door4 = 8;
    };
};

struct Car final
{
    struct Status final
    {
	struct Door final
	{
	    inline static int Locked = 0;
	    inline static int Unlocked = 1;
	};

	struct TurnSignal final
	{
	    inline static int Off = 0;
	    inline static int On = 1;
	};
    };
};

class Console
{
private:
    int door_status[4];
    int turn_status[2];
    long current_speed;
    int maxdlen;
    int randomize;
    int seed;

    struct timeval tv;

    int can_socket;
    int enable_canfd;
    ifreq ifr;
    sockaddr_can addr;
    iovec iov;
    msghdr msg;
    cmsghdr *cmsg;
    canfd_frame can_frame;
    char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
protected:
    void initialize_can_socket(const char* name)
    {
        if ((can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
        {
            std::cerr << "Error: Cannot initiliaze raw CAN socket" << std::endl;
            exit(-1);
        }
	addr.can_family = AF_CAN;
        strcpy(ifr.ifr_name, name);

        if (ioctl(can_socket, SIOCGIFINDEX, &ifr) < 0)
        {
            std::cerr << "Error: SIOCGIFINDEX failed" << std::endl;
            exit(-3);
        }

	addr.can_ifindex = ifr.ifr_ifindex;
	if (setsockopt(can_socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd)))
        {
            std::cerr << "Error: Cannot enable CAN fd" << std::endl;
            exit(-4);
        }

	iov.iov_base = &can_frame;
	iov.iov_len = sizeof(can_frame);
	msg.msg_name = &addr;
	msg.msg_namelen = sizeof(addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &ctrlmsg;
	msg.msg_controllen = sizeof(ctrlmsg);
	msg.msg_flags = 0;

	if (bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            std::cerr << "Error: Cannot bind to CAN socket" << std::endl;
            exit(-5);
        }

	if (randomize || seed)
	{
	    if (randomize)
		seed = time(NULL);
	    srand(seed);

	    CanMessage::ID::Door = (rand() % 2046) + 1;
	    CanMessage::ID::Signal = (rand() % 2046) + 1;
	    CanMessage::ID::Speed = (rand() % 2046) + 1;

	    CanMessage::Position::Door = rand() % 9;
	    CanMessage::Position::Signal = rand() % 9;
	    CanMessage::Position::Speed = rand() % 9;

	    std::cout << "Randomizer seed: " << seed << std::endl;
	}
    }
public:
    Console()
    {
	current_speed = 0;
	maxdlen = 0;
	randomize = 0;
	seed = 0;

	for (int i = 0; i < 4; ++i)
	{
	    door_status[i] = Car::Status::Door::Locked;
	}

	for (int i = 0; i < 2; ++i)
	{
	    turn_status[i] = Car::Status::TurnSignal::Off;
	}

	initialize_can_socket("vcan0");
    }

    long map(long x, long in_min, long in_max, long out_min, long out_max)
    {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    void updateDoors() 
    {
	// No update if all doors are locked
	if (door_status[0] == Car::Status::Door::Locked && 
	    door_status[1] == Car::Status::Door::Locked &&
	    door_status[2] == Car::Status::Door::Locked && 
	    door_status[3] == Car::Status::Door::Locked) 
	    return;

	// Make the base body red if even one door is unlocked
	if(door_status[0] == Car::Status::Door::Unlocked)
	{
	    std::cout << "Door 1 is UNLOCKED" << std::endl;;
	}
	if(door_status[1] == Car::Status::Door::Unlocked) 
	{
	    std::cout << "Door 2 is UNLOCKED" << std::endl;
	}
	if(door_status[2] == Car::Status::Door::Unlocked) 
	{
	    std::cout << "Door 3 is UNLOCKED" << std::endl;
	}
	if(door_status[3] == Car::Status::Door::Unlocked) 
	{
	    std::cout << "Door 4 is UNLOCKED" << std::endl;
	}
    }

    void updateSpeed()
    {
	std::cout << "Current speed: " << current_speed << std::endl;
    }

    void updateTurnSignals()
    {
	if (turn_status[0] == Car::Status::TurnSignal::Off)
	{
	    std::cout << "Turn signal 1 is OFF" << std::endl;
	}
	if (turn_status[1] == Car::Status::TurnSignal::Off)
	{
	    std::cout << "Turn signal 2 is OFF" << std::endl;
	}
	if (turn_status[0] == Car::Status::TurnSignal::On)
	{
	    std::cout << "Turn signal 1 is ON" << std::endl;
	}
	if (turn_status[1] == Car::Status::TurnSignal::On)
	{
	    std::cout << "Turn signal 2 is ON" << std::endl;
	}
    }

    void updateSpeedStatus()
    {
	int len = can_frame.len > maxdlen ? maxdlen : can_frame.len;
	if (len < CanMessage::Position::Speed + 1)
	    return;

	int speed = can_frame.data[CanMessage::Position::Speed] << 8;
	speed += can_frame.data[CanMessage::Position::Speed + 1];
        speed = speed / 100; // speed in kilometers
        current_speed = speed;

	updateSpeed();
    }

    void updateSignalStatus()
    {
	int len = can_frame.len > maxdlen ? maxdlen : can_frame.len;
	if (len < CanMessage::Position::Signal)
	    return;

	if (can_frame.data[CanMessage::Position::Signal] & 1)
	    turn_status[0] = Car::Status::TurnSignal::On;
	else
	    turn_status[0] = Car::Status::TurnSignal::Off;

	if (can_frame.data[CanMessage::Position::Signal] & 2)
	    turn_status[1] = Car::Status::TurnSignal::On;
	else
	    turn_status[1] = Car::Status::TurnSignal::Off;

	updateTurnSignals();
    }

    void updateDoorStatus()
    {
	int len = can_frame.len > maxdlen ? maxdlen : can_frame.len;
	if (len < CanMessage::Position::Door)
	    return;

	if (can_frame.data[CanMessage::Position::Door] & 1)
	    door_status[0] = Car::Status::Door::Locked;
	else
	    door_status[0] = Car::Status::Door::Unlocked;

	if (can_frame.data[CanMessage::Position::Door] & 2)
	    door_status[1] = Car::Status::Door::Locked;
	else
	    door_status[1] = Car::Status::Door::Unlocked;
	
	if (can_frame.data[CanMessage::Position::Door] & 4)
	    door_status[2] = Car::Status::Door::Locked;
	else
	    door_status[2] = Car::Status::Door::Unlocked;
	
	if (can_frame.data[CanMessage::Position::Door] & 8)
	    door_status[3] = Car::Status::Door::Locked;
	else
	    door_status[3] = Car::Status::Door::Unlocked;

	updateDoors();
    }

    [[noreturn]] void run()
    {
	while(true)
	{
	    int nbytes = recvmsg(can_socket, &msg, 0);
	    if (nbytes < 0)
	    {
		std::cerr << "Error: cannot read data from CAN fd" << std::endl;
		exit(-6);
	    }

	    if ((size_t)nbytes == CAN_MTU)
		maxdlen = CAN_MAX_DLEN;
	    else if ((size_t)nbytes == CANFD_MTU)
		maxdlen = CANFD_MAX_DLEN;
	    else
	    {
		std::cerr << "Error: incompatible CAN frame." << std::endl;
		exit(-7);
	    }

	    for (cmsg = CMSG_FIRSTHDR(&msg);
		    cmsg && (cmsg->cmsg_level == SOL_SOCKET);
		    cmsg = CMSG_NXTHDR(&msg,cmsg))
	    {
		if (cmsg->cmsg_type == SO_TIMESTAMP)
		    tv = *(struct timeval *)CMSG_DATA(cmsg);
		else if (cmsg->cmsg_type == SO_RXQ_OVFL)
		    std::cerr << "Message: CAN packet dropped" << std::endl;
	    }

	    if (can_frame.can_id == CanMessage::ID::Door)
		updateDoorStatus();
	    if (can_frame.can_id == CanMessage::ID::Signal)
		updateSignalStatus();
	    if (can_frame.can_id == CanMessage::ID::Speed)
		updateSpeedStatus();
	}
    }
};

int main()
{
    Console car_console;
    car_console.run();
    return 0;
}
