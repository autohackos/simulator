/*
   Copyright(c) 2023 Adhokshaj Mishra <me@adhokshajmishraonline.in>
*/

#ifndef CAN_HPP
#define CAN_HPP

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

#endif
