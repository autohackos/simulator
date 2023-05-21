/*
   Copyright(c) 2023 Adhokshaj Mishra <me@adhokshajmishraonline.in>
*/

#ifndef CAR_HPP
#define CAR_HPP

struct CarParameters final
{
    inline static float MaximumSpeed = 90.0;
    inline static float AccelerationRate = 8.0;
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

#endif
