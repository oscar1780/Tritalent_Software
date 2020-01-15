#ifndef P3_PROTOCOL_H
#define P3_PROTOCOL_H

typedef unsigned char byte;

typedef struct _vel_control_frame{
    byte frame_header;  //default 0x55 0xAA
    byte frame_lenghth; //default 10
    byte msg_id;        //default 0x01
    byte vel[4];        //4 bytes as float vel
    byte angvel[4];     //4 bytes as float angular vel
    byte checksum;      //checksum of frame_length to data
}vel_control_frame;

typedef struct _vel_status_frame{
    byte frame_header;  //default 0x55 0xAA
    byte frame_lenghth; //default 10
    byte msg_id;        //default 0xF1
    byte vel[4];        //4 bytes as float vel
    byte angvel[4];     //4 bytes as float angular vel
    byte checksum;      //checksum of frame_length to data
}vel_status_frame;

typedef struct _warn_status_frame{
    byte frame_header;  //default 0x55 0xAA
    byte frame_lenghth; //defautl 7
    byte msg_id;        //default 0xF2
    byte warn_id[4];    //4 bytes as uint32_t
    byte warn_lev;      //1 bytes as uint8_t
    byte checksum;      //checksum of frame_length to data
}warn_status_frame;

typedef struct _battery_status_frame{
    byte frame_header;  //default 0x55 0xAA
    byte frame_lenghth; //defautl 5
    byte msg_id;        //default 0xF3
    byte voltage[2];    //2 bytes as uint16_t
    byte remain_voltage;//1 bytes as uint8_t
    byte checksum;      //checksum of frame_length to data
}battery_status_frame;

/***********/
extern vel_control_frame vel_control;
extern vel_status_frame  vel_status;
extern warn_status_frame warn_status;
extern battery_status_frame battery_status;
/***********/
#endif // P3_PROTOCOL_H
