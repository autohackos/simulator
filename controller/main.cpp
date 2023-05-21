/*
   Control panel for car CAN bus simulator
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

#include "../common/can.hpp"
#include "../common/car.hpp"

class Controller
{
private:
    unsigned long long startup_time, current_time, last_acceleration_time, last_signal_time;
    int difficulty;

    char door_state;
    char signal_state;
    float current_speed;
    int throttle;
    int turning;

    int can_socket;
    int enable_canfd;
    sockaddr_can addr;
    ifreq ifr;
    canfd_frame can_frame;
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
	if (bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
	    std::cerr << "Error: Cannot bind to CAN socket" << std::endl;
	    exit(-5);
	}
    }

    unsigned long long getTicks()
    {
	clock_t now = clock();
	return (unsigned long long)(now - startup_time);
    }
public:
    Controller()
    {
	//initialize the clock time
	startup_time = clock();
	current_time = startup_time;
	last_acceleration_time = startup_time;
	last_signal_time = startup_time;

	//initialize vehicle state
	door_state = 0xf;
	signal_state = 0;
	current_speed = 0;
	throttle = 0;
	turning = 0;

	enable_canfd = 1;

	initialize_can_socket("vcan0");
    }

    void sendPacket(int mtu)
    {
	if (write(can_socket, &can_frame, mtu) != mtu)
	{
	    std::cerr << "Error: Cannot write complate CAN frame" << std::endl;
	    exit(-2);
	}
    }

    void randomizePacket(int start, int stop)
    {
	if (difficulty < 2)
	    return;
	for (int i = start; i < stop; ++i)
	{
	    if(rand() % 3 < 1)
		can_frame.data[i] = rand() % 255;
	}
    }

    void lockDoor(char door)
    {
	door_state |= door;
	memset(&can_frame, 0, sizeof(can_frame));
	can_frame.can_id = CanMessage::ID::Door;
	can_frame.len = CanMessage::Length::Door;
	can_frame.data[CanMessage::Position::Door] = door_state;

	if (CanMessage::Position::Door)
	    randomizePacket(0, CanMessage::Position::Door);
	if (CanMessage::Length::Door > CanMessage::Position::Door + 1)
	    randomizePacket(CanMessage::Position::Door + 1, CanMessage::Length::Door);

	sendPacket(CAN_MTU);
    }

    void unlockDoor(char door)
    {
	door_state &= ~door;
	memset(&can_frame, 0, sizeof(can_frame));
	can_frame.can_id = CanMessage::ID::Door;
	can_frame.len = CanMessage::Length::Door;
	can_frame.data[CanMessage::Position::Door] = door_state;

	if (CanMessage::Position::Door)
	    randomizePacket(0, CanMessage::Position::Door);
	if (CanMessage::Length::Door > CanMessage::Position::Door + 1)
	    randomizePacket(CanMessage::Position::Door + 1, CanMessage::Length::Door);

	sendPacket(CAN_MTU);
    }

    void sendTurnSignal()
    {
	memset(&can_frame, 0, sizeof(can_frame));
	can_frame.can_id = CanMessage::ID::Signal;
	can_frame.len = CanMessage::Length::Signal;
	can_frame.data[CanMessage::Position::Signal] = signal_state;

	if (CanMessage::Position::Signal)
	    randomizePacket(0, CanMessage::Position::Signal);
	if (CanMessage::Length::Signal > CanMessage::Position::Signal + 1)
	    randomizePacket(CanMessage::Position::Signal + 1, CanMessage::Length::Signal);

	sendPacket(CAN_MTU);
    }

    void sendSpeed()
    {
	int kmph = current_speed * 100;
	memset(&can_frame, 0, sizeof(can_frame));
	can_frame.can_id = CanMessage::ID::Speed;
	can_frame.len = CanMessage::Length::Speed;
	
	if (kmph)
	{
	    // we have to split the speed data, and set that in correct order
	    can_frame.data[CanMessage::Position::Speed + 1] = (char)kmph & 0xff;
	    can_frame.data[CanMessage::Position::Speed] = (char)(kmph >> 8) & 0xff;
	}
	else
	{
	    can_frame.data[CanMessage::Position::Speed] = 1;
	    can_frame.data[CanMessage::Position::Speed + 1] = rand() % 255+100;
	}

	if (CanMessage::Position::Speed)
	    randomizePacket(0, CanMessage::Position::Speed);
	if (CanMessage::Length::Speed > CanMessage::Position::Speed + 1)
	    randomizePacket(CanMessage::Position::Speed + 1, CanMessage::Length::Speed);

	sendPacket(CAN_MTU);
    }

    void checkAcceleration()
    {
	float rate = CarParameters::MaximumSpeed / (CarParameters::AccelerationRate * 100);

	// refresh at every 10ms
	if (current_time > last_acceleration_time + 10)
	{
	    if (throttle < 0)
	    {
		current_speed -= rate;
		if (current_speed < 1)
		    current_speed = 0;
	    }
	    if (throttle > 0)
	    {
		current_speed += rate;
		if (current_speed > CarParameters::MaximumSpeed)
		    current_speed = CarParameters::MaximumSpeed;
	    }

	    sendSpeed();
	    last_acceleration_time = current_time;
	}
    }

    void checkTurnSignal()
    {
	// refresh every 500ms
	if (current_time > last_signal_time + 500)
	{
	    if (turning < 0)
		signal_state ^= CanMessage::Equipment::LeftSignal;
	    else if (turning > 0)
		signal_state ^= CanMessage::Equipment::RightSignal;
	    else
		signal_state = 0;

	    sendTurnSignal();
	    last_signal_time = current_time;
	}
    }

    [[noreturn]] void run()
    {
	while(true)
	{
	    std::this_thread::sleep_for(std::chrono::milliseconds(10));

	    throttle = 1;
	    turning = 2;

	    unlockDoor(CanMessage::Equipment::Door2);
	    current_time = getTicks();
	    checkAcceleration();
	    checkTurnSignal();
	}
    }
};

int main()
{
    Controller ctl;
    ctl.run();
    return 0;
}
