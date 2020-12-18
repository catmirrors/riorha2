//===========================================================================
//

#ifndef __MPG123_H__
#define	__MPG123_H__

#include        <stdio.h>
#include        <string.h>
#include        <signal.h>

#ifndef WIN32
#include        <sys/signal.h>
#include        <unistd.h>
#endif

#include        <math.h>

#ifdef _WIN32
#undef WIN32
#define WIN32

#define M_PI       3.14159265358979323846
#define M_SQRT2	1.41421356237309504880
#define REAL_IS_FLOAT
#define NEW_DCT9

#define random rand
#define srandom srand

#endif






//===========================================================================
//	
#define	MP3F_IS_MPEG25_0		(1<<20)
#define	MP3F_IS_LSF				(1<<19)
#define	MP3F_LAY				(0x3<<17)
#define	MP3F_ERROR_PROTECTION	(0x1<<16)

#define	MP3F_BITRATE_INDEX		(0xF<<12)
#define	MP3F_SAMPLING_FREQUENCY	(0x3<<10)
#define	MP3F_IS_PADDING			(0x1<<9)
#define	MP3F_EXTENSION			(0x1<<8)
#define	MP3F_MODE				(0x3<<6)	//(0 = stereo / 1 = joint stereo / 2 = dual channel / 3 = mono
#define	MP3F_MODE_EXTENSION		(0x3<<4)
#define	MP3F_IS_COPYRIGHT		(0x1<<3)
#define	MP3F_IS_ORIGINAL		(0x1<<2)
#define	MP3F_EMPHASIS			(0x3<<0)


#define	RHAF_LASTZEROADD		0x1000
#define	RHAF_LASTFULLADD		0x2000

#define	RHA_VER100				0x010B		// rha のバージョン 1.00 （これ以前のバージョンと区別するために、下位４ビットは、0xB 固定とする







typedef float real;
typedef real COSTAB;	//real
typedef real DECWIN;	//real
typedef real realho;	// real hybrid out（ hybrid out 値以降を保持する型）
typedef real real64;	// real ( dct64 以降の値を保持する型）





/* AUDIOBUFSIZE = n*64 with n=1,2,3 ...  */
#define	AUDIOBUFSIZE		16384

#define FALSE                   0
#define TRUE                    1

#define SBLIMIT                 32
#define SSLIMIT                 18

#define MPG_MD_STEREO           0
#define MPG_MD_JOINT_STEREO     1
#define MPG_MD_DUAL_CHANNEL     2
#define MPG_MD_MONO             3

#define MAXFRAMESIZE 1792


/* Pre Shift fo 16 to 8 bit converter table */
#define AUSHIFT (3)

//
struct CMp3Frame {
    int m_stereo;
    int m_jsbound;
    int m_single;
    int m_lsf;
    int m_mpeg25;
    int m_header_change;
    int m_lay;
    int m_error_protection;
    int m_bitrate_index;
    int m_sampling_frequency;
    int m_padding;
    int m_extension;
    int m_mode;
    int m_mode_ext;
    int m_copyright;
    int m_original;
    int m_emphasis;
    int m_framesize; /* computed framesize */

	BOOL DecodeHeaderToFrame( DWORD newhead );
};

//
struct CParameter {
	int quiet;	/* shut up! */
	int tryresync;  /* resync stream after error */
	int verbose;    /* verbose level */
	int checkrange;
};


extern void make_decode_tables( long scaleval );



struct gr_info_s {
      int scfsi;
      unsigned part2_3_length;
      unsigned big_values;
      unsigned scalefac_compress;
      unsigned block_type;
      unsigned mixed_block_flag;
      unsigned table_select[3];
      unsigned subblock_gain[3];
      unsigned maxband[3];
      unsigned maxbandl;
      unsigned maxb;
      unsigned region1start;
      unsigned region2start;
      unsigned preflag;
      unsigned scalefac_scale;
      unsigned count1table_select;
      real *full_gain[3];
      real *pow2gain;
};

struct III_sideinfo
{
  unsigned main_data_begin;
  unsigned private_bits;
  struct {
    struct gr_info_s gr[2];
  } ch[2];
};



extern void init_layer3(int);
extern void init_layer2(void);
extern void make_decode_tables(long scale);
extern void make_conv16to8_table(int);
extern void dct64(real64 *, real64 *, realho *);

extern void synth_ntom_set_step(long,long);

extern unsigned char *conv16to8;
extern const long _GLOBAL_Mp3Freqs[9];
extern real muls[27][64];
extern DECWIN decwin[512+32];
//extern real *pnts[5];
extern COSTAB cos64[16],cos32[8],cos16[4],cos8[2],cos4[1];



#endif	// __MPG123_H__