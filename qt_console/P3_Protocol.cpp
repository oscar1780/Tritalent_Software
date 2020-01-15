#include "P3_Protocol.h"

void init_p3_cmd()
{
    vel_control.frame_header=0x55AA;
    vel_control.frame_lenghth=10;
    vel_control.msg_id=0x01;
    vel_control.vel=0;
    vel_control.angvel=0;
    vel_control.checksum=0;
}
