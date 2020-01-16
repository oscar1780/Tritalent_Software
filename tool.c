
/* extract unsigned/signed bits ------------------------------------------------
* extract unsigned/signed bits from byte data
* args   : unsigned char *buff I byte data
*          int    pos    I      bit position from start of data (bits)
*          int    len    I      bit length (bits) (len<=32)
* return : extracted unsigned/signed bits
*-----------------------------------------------------------------------------*/
extern unsigned int getbitu(const unsigned char *buff, int pos, int len)
{
    unsigned int bits=0;
    int i;
    for (i=pos;i<pos+len;i++) bits=(bits<<1)+((buff[i/8]>>(7-i%8))&1u);
    return bits;
}
extern int getbits(const unsigned char *buff, int pos, int len)
{
    unsigned int bits=getbitu(buff,pos,len);
    if (len<=0||32<=len||!(bits&(1u<<(len-1)))) return (int)bits;
    return (int)(bits|(~0u<<len)); /* extend sign */
}
/* set unsigned/signed bits ----------------------------------------------------
* set unsigned/signed bits to byte data
* args   : unsigned char *buff IO byte data
*          int    pos    I      bit position from start of data (bits)
*          int    len    I      bit length (bits) (len<=32)
*         (unsigned) int I      unsigned/signed data
* return : none
*-----------------------------------------------------------------------------*/
extern void setbitu(unsigned char *buff, int pos, int len, unsigned int data)
{
    unsigned int mask=1u<<(len-1);
    int i;
    if (len<=0||32<len) return;
    for (i=pos;i<pos+len;i++,mask>>=1) {
        if (data&mask) buff[i/8]|=1u<<(7-i%8); else buff[i/8]&=~(1u<<(7-i%8));
    }
}
extern void setbits(unsigned char *buff, int pos, int len, int data)
{
    if (data<0) data|=1<<(len-1); else data&=~(1<<(len-1)); /* set sign bit */
    setbitu(buff,pos,len,(unsigned int)data);
}


/* input rtcm 2 message from stream --------------------------------------------
* fetch next rtcm 2 message and input a message from byte stream
* args   : rtcm_t *rtcm IO   rtcm control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 5: input station pos/ant parameters,
*                  6: input time parameter, 7: input dgps corrections,
*                  9: input special message)
* notes  : before firstly calling the function, time in rtcm control struct has
*          to be set to the approximate time within 1/2 hour in order to resolve
*          ambiguity of time in rtcm messages.
*          supported msgs RTCM ver.2: 1,3,9,14,16,17,18,19,22
*          refer [1] for RTCM ver.2
*-----------------------------------------------------------------------------*/
extern int input_rtcm2(rtcm_t *rtcm, unsigned char data)
{
    unsigned char preamb;
    int i;
    
    trace(5,"input_rtcm2: data=%02x\n",data);
    
    if ((data&0xC0)!=0x40) return 0; /* ignore if upper 2bit != 01 */
    
    for (i=0;i<6;i++,data>>=1) { /* decode 6-of-8 form */
        rtcm->word=(rtcm->word<<1)+(data&1);
        
        /* synchronize frame */
        if (rtcm->nbyte==0) {
            preamb=(unsigned char)(rtcm->word>>22);
            if (rtcm->word&0x40000000) preamb^=0xFF; /* decode preamble */
            if (preamb!=RTCM2PREAMB) continue;
            
            /* check parity */
            if (!decode_word(rtcm->word,rtcm->buff)) continue;
            rtcm->nbyte=3; rtcm->nbit=0;
            continue;
        }
        if (++rtcm->nbit<30) continue; else rtcm->nbit=0;
        
        /* check parity */
        if (!decode_word(rtcm->word,rtcm->buff+rtcm->nbyte)) {
            trace(2,"rtcm2 partity error: i=%d word=%08x\n",i,rtcm->word);
            rtcm->nbyte=0; rtcm->word&=0x3;
            continue;
        }
        rtcm->nbyte+=3;
        if (rtcm->nbyte==6) rtcm->len=(rtcm->buff[5]>>3)*3+6;
        if (rtcm->nbyte<rtcm->len) continue;
        rtcm->nbyte=0; rtcm->word&=0x3;
        
        /* decode rtcm2 message */
        return decode_rtcm2(rtcm);
    }
    return 0;
}

/* input rtcm 2 message from file ----------------------------------------------
* fetch next rtcm 2 message and input a messsage from file
* args   : rtcm_t *rtcm IO   rtcm control struct
*          FILE  *fp    I    file pointer
* return : status (-2: end of file, -1...10: same as above)
* notes  : same as above
*-----------------------------------------------------------------------------*/
extern int input_rtcm2f(rtcm_t *rtcm, FILE *fp)
{
    int i,data=0,ret;
    
    trace(4,"input_rtcm2f: data=%02x\n",data);
    
    for (i=0;i<4096;i++) {
        if ((data=fgetc(fp))==EOF) return -2;
        if ((ret=input_rtcm2(rtcm,(unsigned char)data))) return ret;
    }
    return 0; /* return at every 4k bytes */
}

typedef struct {        /* RTCM control struct type */
    int staid;          /* station id */
    int stah;           /* station health */
    int seqno;          /* sequence number for rtcm 2 or iods msm */
    int outtype;        /* output message type */
    gtime_t time;       /* message time */
    gtime_t time_s;     /* message start time */
    obs_t obs;          /* observation data (uncorrected) */
    nav_t nav;          /* satellite ephemerides */
    sta_t sta;          /* station parameters */
    dgps_t *dgps;       /* output of dgps corrections */
    ssr_t ssr[MAXSAT];  /* output of ssr corrections */
    char msg[128];      /* special message */
    char msgtype[256];  /* last message type */
    char msmtype[6][128]; /* msm signal types */
    int obsflag;        /* obs data complete flag (1:ok,0:not complete) */
    int ephsat;         /* update satellite of ephemeris */
    double cp[MAXSAT][NFREQ+NEXOBS]; /* carrier-phase measurement */
    unsigned short lock[MAXSAT][NFREQ+NEXOBS]; /* lock time */
    unsigned short loss[MAXSAT][NFREQ+NEXOBS]; /* loss of lock count */
    gtime_t lltime[MAXSAT][NFREQ+NEXOBS]; /* last lock time */
    int nbyte;          /* number of bytes in message buffer */ 
    int nbit;           /* number of bits in word buffer */ 
    int len;            /* message length (bytes) */
    unsigned char buff[1200]; /* message buffer */
    unsigned int word;  /* word buffer for rtcm 2 */
    unsigned int nmsg2[100]; /* message count of RTCM 2 (1-99:1-99,0:other) */
    unsigned int nmsg3[400]; /* message count of RTCM 3 (1-299:1001-1299,300-399:2000-2099,0:ohter) */
    char opt[256];      /* RTCM dependent options */
} rtcm_t;

/* decode rtcm ver.2 message -------------------------------------------------*/
extern int decode_rtcm2(rtcm_t *rtcm)
{
    double zcnt;
    int staid,seqno,stah,ret=0,type=getbitu(rtcm->buff,8,6);
    
    trace(3,"decode_rtcm2: type=%2d len=%3d\n",type,rtcm->len);
    
    if ((zcnt=getbitu(rtcm->buff,24,13)*0.6)>=3600.0) {
        trace(2,"rtcm2 modified z-count error: zcnt=%.1f\n",zcnt);
        return -1;
    }
    adjhour(rtcm,zcnt);
    staid=getbitu(rtcm->buff,14,10);
    seqno=getbitu(rtcm->buff,37, 3);
    stah =getbitu(rtcm->buff,45, 3);
    if (seqno-rtcm->seqno!=1&&seqno-rtcm->seqno!=-7) {
        trace(2,"rtcm2 message outage: seqno=%d->%d\n",rtcm->seqno,seqno);
    }
    rtcm->seqno=seqno;
    rtcm->stah =stah;
    
    if (rtcm->outtype) {
        sprintf(rtcm->msgtype,"RTCM %2d (%4d) zcnt=%7.1f staid=%3d seqno=%d",
                type,rtcm->len,zcnt,staid,seqno);
    }
    if (type==3||type==22||type==23||type==24) {
        if (rtcm->staid!=0&&staid!=rtcm->staid) {
           trace(2,"rtcm2 station id changed: %d->%d\n",rtcm->staid,staid);
        }
        rtcm->staid=staid;
    }
    if (rtcm->staid!=0&&staid!=rtcm->staid) {
        trace(2,"rtcm2 station id invalid: %d %d\n",staid,rtcm->staid);
        return -1;
    }
    switch (type) {
        case  1: ret=decode_type1 (rtcm); break;
        case  3: ret=decode_type3 (rtcm); break;
        case  9: ret=decode_type1 (rtcm); break;
        case 14: ret=decode_type14(rtcm); break;
        case 16: ret=decode_type16(rtcm); break;
        case 17: ret=decode_type17(rtcm); break;
        case 18: ret=decode_type18(rtcm); break;
        case 19: ret=decode_type19(rtcm); break;
        case 22: ret=decode_type22(rtcm); break;
        case 23: ret=decode_type23(rtcm); break; /* not supported */
        case 24: ret=decode_type24(rtcm); break; /* not supported */
        case 31: ret=decode_type31(rtcm); break; /* not supported */
        case 32: ret=decode_type32(rtcm); break; /* not supported */
        case 34: ret=decode_type34(rtcm); break; /* not supported */
        case 36: ret=decode_type36(rtcm); break; /* not supported */
        case 37: ret=decode_type37(rtcm); break; /* not supported */
        case 59: ret=decode_type59(rtcm); break; /* not supported */
    }
    if (ret>=0) {
        if (1<=type&&type<=99) rtcm->nmsg2[type]++; else rtcm->nmsg2[0]++;
    }
    return ret;
}


/* decode type 1/9: differential gps correction/partial correction set -------*/
static int decode_type1(rtcm_t *rtcm)
{
    int i=48,fact,udre,prn,sat,iod;
    double prc,rrc;
    
    trace(4,"decode_type1: len=%d\n",rtcm->len);
    
    while (i+40<=rtcm->len*8) {
        fact=getbitu(rtcm->buff,i, 1); i+= 1;
        udre=getbitu(rtcm->buff,i, 2); i+= 2;
        prn =getbitu(rtcm->buff,i, 5); i+= 5;
        prc =getbits(rtcm->buff,i,16); i+=16;
        rrc =getbits(rtcm->buff,i, 8); i+= 8;
        iod =getbits(rtcm->buff,i, 8); i+= 8;
        if (prn==0) prn=32;
        if (prc==0x80000000||rrc==0xFFFF8000) {
            trace(2,"rtcm2 1 prc/rrc indicates satellite problem: prn=%d\n",prn);
            continue;
        }
        if (rtcm->dgps) {
            sat=satno(SYS_GPS,prn);
            rtcm->dgps[sat-1].t0=rtcm->time;
            rtcm->dgps[sat-1].prc=prc*(fact?0.32:0.02);
            rtcm->dgps[sat-1].rrc=rrc*(fact?0.032:0.002);
            rtcm->dgps[sat-1].iod=iod;
            rtcm->dgps[sat-1].udre=udre;
        }
    }
    return 7;
}