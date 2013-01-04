
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "Arduino.h"

#define PWM_PIN       11
#define PWM_VALUE     OCR2A
#define PWM_INTERRUPT TIMER1_COMPA_vect

#define VIBRATO_SPEED_CTRL 0
#define VIBRATO_DEPTH_CTRL 1
#define LOWPASS_FREQ_CTRL 2
#define LOWPASS_RAMP_CTRL 3
#define LOWPASS_RESONANCE_CTRL 4

void setup();
void loop();
static void reset_sample();

// 0 for rough sawtooth, 1 for sawtooth, 2 for sine, 3 for square
static unsigned char g_wave_type;

// tone frequency
static unsigned short g_base_freq;
static unsigned short g_curr_freq;
static unsigned char g_freq_ramp;
static unsigned char g_freq_ramp_cnt;

// base frequency generation
static unsigned short g_phase;

// envelope
static unsigned char g_env_stage;
static unsigned short g_env_time;
static unsigned short g_env_lengths[2];

// low-pass filter
static unsigned char g_lpf_resonance;
static unsigned char g_lpf_base_freq;
static unsigned char g_lpf_curr_freq;
static char g_lpf_ramp;
static unsigned char g_lpf_ramp_cnt;
static short g_lpf_prev;
static short g_lpf_prev_delta;

// vibrato
static unsigned short g_vib_phase;
static unsigned char g_vib_strength;
static unsigned char g_vib_speed;
static short g_vib_fix_accum;

// button status
static uint8_t g_button_status;

static const short phase_to_delta[4] = {1,-1,-1,1};

/* TODO: for some reason, this doesn't work properly
const prog_int16_t mysin_table[2048] = {
   0,  3,  6,  9,  12,  15,  18,  21,  
   25,  28,  31,  34,  37,  40,  43,  47,  
   50,  53,  56,  59,  62,  65,  69,  72,  
   75,  78,  81,  84,  87,  90,  94,  97,  
   100,  103,  106,  109,  112,  115,  119,  122,  
   125,  128,  131,  134,  137,  140,  144,  147,  
   150,  153,  156,  159,  162,  165,  168,  171,  
   175,  178,  181,  184,  187,  190,  193,  196,  
   199,  202,  205,  209,  212,  215,  218,  221,  
   224,  227,  230,  233,  236,  239,  242,  245,  
   248,  251,  254,  257,  260,  264,  267,  270,  
   273,  276,  279,  282,  285,  288,  291,  294,  
   297,  300,  303,  306,  309,  312,  315,  318,  
   321,  324,  327,  330,  333,  336,  339,  342,  
   344,  347,  350,  353,  356,  359,  362,  365,  
   368,  371,  374,  377,  380,  383,  386,  388,  
   391,  394,  397,  400,  403,  406,  409,  412,  
   414,  417,  420,  423,  426,  429,  432,  434,  
   437,  440,  443,  446,  449,  451,  454,  457,  
   460,  463,  466,  468,  471,  474,  477,  479,  
   482,  485,  488,  491,  493,  496,  499,  501,  
   504,  507,  510,  512,  515,  518,  521,  523,  
   526,  529,  531,  534,  537,  539,  542,  545,  
   547,  550,  553,  555,  558,  561,  563,  566,  
   568,  571,  574,  576,  579,  581,  584,  587,  
   589,  592,  594,  597,  599,  602,  604,  607,  
   609,  612,  615,  617,  620,  622,  625,  627,  
   629,  632,  634,  637,  639,  642,  644,  647,  
   649,  652,  654,  656,  659,  661,  664,  666,  
   668,  671,  673,  675,  678,  680,  683,  685,  
   687,  690,  692,  694,  696,  699,  701,  703,  
   706,  708,  710,  712,  715,  717,  719,  721,  
   724,  726,  728,  730,  732,  735,  737,  739,  
   741,  743,  745,  748,  750,  752,  754,  756,  
   758,  760,  762,  765,  767,  769,  771,  773,  
   775,  777,  779,  781,  783,  785,  787,  789,  
   791,  793,  795,  797,  799,  801,  803,  805,  
   807,  809,  811,  813,  814,  816,  818,  820,  
   822,  824,  826,  828,  829,  831,  833,  835,  
   837,  839,  840,  842,  844,  846,  847,  849,  
   851,  853,  854,  856,  858,  860,  861,  863,  
   865,  866,  868,  870,  871,  873,  875,  876,  
   878,  879,  881,  883,  884,  886,  887,  889,  
   890,  892,  894,  895,  897,  898,  900,  901,  
   903,  904,  906,  907,  908,  910,  911,  913,  
   914,  916,  917,  918,  920,  921,  922,  924,  
   925,  927,  928,  929,  930,  932,  933,  934,  
   936,  937,  938,  939,  941,  942,  943,  944,  
   946,  947,  948,  949,  950,  951,  953,  954,  
   955,  956,  957,  958,  959,  960,  962,  963,  
   964,  965,  966,  967,  968,  969,  970,  971,  
   972,  973,  974,  975,  976,  977,  978,  978,  
   979,  980,  981,  982,  983,  984,  985,  986,  
   986,  987,  988,  989,  990,  990,  991,  992,  
   993,  994,  994,  995,  996,  997,  997,  998,  
   999,  999,  1000,  1001,  1001,  1002,  1003,  1003,  
   1004,  1004,  1005,  1006,  1006,  1007,  1007,  1008,  
   1008,  1009,  1009,  1010,  1010,  1011,  1011,  1012,  
   1012,  1013,  1013,  1014,  1014,  1015,  1015,  1015,  
   1016,  1016,  1017,  1017,  1017,  1018,  1018,  1018,  
   1019,  1019,  1019,  1019,  1020,  1020,  1020,  1020,  
   1021,  1021,  1021,  1021,  1022,  1022,  1022,  1022,  
   1022,  1022,  1023,  1023,  1023,  1023,  1023,  1023,  
   1023,  1023,  1023,  1023,  1023,  1023,  1023,  1023,  
   1023,  1023,  1023,  1023,  1023,  1023,  1023,  1023,  
   1023,  1023,  1023,  1023,  1023,  1023,  1023,  1022,  
   1022,  1022,  1022,  1022,  1022,  1021,  1021,  1021,  
   1021,  1020,  1020,  1020,  1020,  1019,  1019,  1019,  
   1019,  1018,  1018,  1018,  1017,  1017,  1017,  1016,  
   1016,  1015,  1015,  1015,  1014,  1014,  1013,  1013,  
   1012,  1012,  1011,  1011,  1010,  1010,  1009,  1009,  
   1008,  1008,  1007,  1007,  1006,  1006,  1005,  1004,  
   1004,  1003,  1003,  1002,  1001,  1001,  1000,  999,  
   999,  998,  997,  997,  996,  995,  994,  994,  
   993,  992,  991,  990,  990,  989,  988,  987,  
   986,  986,  985,  984,  983,  982,  981,  980,  
   979,  978,  978,  977,  976,  975,  974,  973,  
   972,  971,  970,  969,  968,  967,  966,  965,  
   964,  963,  962,  960,  959,  958,  957,  956,  
   955,  954,  953,  951,  950,  949,  948,  947,  
   946,  944,  943,  942,  941,  939,  938,  937,  
   936,  934,  933,  932,  930,  929,  928,  927,  
   925,  924,  922,  921,  920,  918,  917,  916,  
   914,  913,  911,  910,  908,  907,  906,  904,  
   903,  901,  900,  898,  897,  895,  894,  892,  
   890,  889,  887,  886,  884,  883,  881,  879,  
   878,  876,  875,  873,  871,  870,  868,  866,  
   865,  863,  861,  860,  858,  856,  854,  853,  
   851,  849,  847,  846,  844,  842,  840,  839,  
   837,  835,  833,  831,  829,  828,  826,  824,  
   822,  820,  818,  816,  814,  813,  811,  809,  
   807,  805,  803,  801,  799,  797,  795,  793,  
   791,  789,  787,  785,  783,  781,  779,  777,  
   775,  773,  771,  769,  767,  765,  762,  760,  
   758,  756,  754,  752,  750,  748,  745,  743,  
   741,  739,  737,  735,  732,  730,  728,  726,  
   724,  721,  719,  717,  715,  712,  710,  708,  
   706,  703,  701,  699,  696,  694,  692,  690,  
   687,  685,  683,  680,  678,  675,  673,  671,  
   668,  666,  664,  661,  659,  656,  654,  652,  
   649,  647,  644,  642,  639,  637,  634,  632,  
   629,  627,  625,  622,  620,  617,  615,  612,  
   609,  607,  604,  602,  599,  597,  594,  592,  
   589,  587,  584,  581,  579,  576,  574,  571,  
   568,  566,  563,  561,  558,  555,  553,  550,  
   547,  545,  542,  539,  537,  534,  531,  529,  
   526,  523,  521,  518,  515,  512,  510,  507,  
   504,  501,  499,  496,  493,  491,  488,  485,  
   482,  479,  477,  474,  471,  468,  466,  463,  
   460,  457,  454,  451,  449,  446,  443,  440,  
   437,  434,  432,  429,  426,  423,  420,  417,  
   414,  412,  409,  406,  403,  400,  397,  394,  
   391,  388,  386,  383,  380,  377,  374,  371,  
   368,  365,  362,  359,  356,  353,  350,  347,  
   344,  342,  339,  336,  333,  330,  327,  324,  
   321,  318,  315,  312,  309,  306,  303,  300,  
   297,  294,  291,  288,  285,  282,  279,  276,  
   273,  270,  267,  264,  260,  257,  254,  251,  
   248,  245,  242,  239,  236,  233,  230,  227,  
   224,  221,  218,  215,  212,  209,  205,  202,  
   199,  196,  193,  190,  187,  184,  181,  178,  
   175,  171,  168,  165,  162,  159,  156,  153,  
   150,  147,  144,  140,  137,  134,  131,  128,  
   125,  122,  119,  115,  112,  109,  106,  103,  
   100,  97,  94,  90,  87,  84,  81,  78,  
   75,  72,  69,  65,  62,  59,  56,  53,  
   50,  47,  43,  40,  37,  34,  31,  28,  
   25,  21,  18,  15,  12,  9,  6,  3,  
   0,  -3,  -6,  -9,  -12,  -15,  -18,  -21,  
   -25,  -28,  -31,  -34,  -37,  -40,  -43,  -47,  
   -50,  -53,  -56,  -59,  -62,  -65,  -69,  -72,  
   -75,  -78,  -81,  -84,  -87,  -90,  -94,  -97,  
   -100,  -103,  -106,  -109,  -112,  -115,  -119,  -122,  
   -125,  -128,  -131,  -134,  -137,  -140,  -144,  -147,  
   -150,  -153,  -156,  -159,  -162,  -165,  -168,  -171,  
   -175,  -178,  -181,  -184,  -187,  -190,  -193,  -196,  
   -199,  -202,  -205,  -209,  -212,  -215,  -218,  -221,  
   -224,  -227,  -230,  -233,  -236,  -239,  -242,  -245,  
   -248,  -251,  -254,  -257,  -260,  -264,  -267,  -270,  
   -273,  -276,  -279,  -282,  -285,  -288,  -291,  -294,  
   -297,  -300,  -303,  -306,  -309,  -312,  -315,  -318,  
   -321,  -324,  -327,  -330,  -333,  -336,  -339,  -342,  
   -344,  -347,  -350,  -353,  -356,  -359,  -362,  -365,  
   -368,  -371,  -374,  -377,  -380,  -383,  -386,  -388,  
   -391,  -394,  -397,  -400,  -403,  -406,  -409,  -412,  
   -414,  -417,  -420,  -423,  -426,  -429,  -432,  -434,  
   -437,  -440,  -443,  -446,  -449,  -451,  -454,  -457,  
   -460,  -463,  -466,  -468,  -471,  -474,  -477,  -479,  
   -482,  -485,  -488,  -491,  -493,  -496,  -499,  -501,  
   -504,  -507,  -510,  -512,  -515,  -518,  -521,  -523,  
   -526,  -529,  -531,  -534,  -537,  -539,  -542,  -545,  
   -547,  -550,  -553,  -555,  -558,  -561,  -563,  -566,  
   -568,  -571,  -574,  -576,  -579,  -581,  -584,  -587,  
   -589,  -592,  -594,  -597,  -599,  -602,  -604,  -607,  
   -609,  -612,  -615,  -617,  -620,  -622,  -625,  -627,  
   -629,  -632,  -634,  -637,  -639,  -642,  -644,  -647,  
   -649,  -652,  -654,  -656,  -659,  -661,  -664,  -666,  
   -668,  -671,  -673,  -675,  -678,  -680,  -683,  -685,  
   -687,  -690,  -692,  -694,  -696,  -699,  -701,  -703,  
   -706,  -708,  -710,  -712,  -715,  -717,  -719,  -721,  
   -724,  -726,  -728,  -730,  -732,  -735,  -737,  -739,  
   -741,  -743,  -745,  -748,  -750,  -752,  -754,  -756,  
   -758,  -760,  -762,  -765,  -767,  -769,  -771,  -773,  
   -775,  -777,  -779,  -781,  -783,  -785,  -787,  -789,  
   -791,  -793,  -795,  -797,  -799,  -801,  -803,  -805,  
   -807,  -809,  -811,  -813,  -814,  -816,  -818,  -820,  
   -822,  -824,  -826,  -828,  -829,  -831,  -833,  -835,  
   -837,  -839,  -840,  -842,  -844,  -846,  -847,  -849,  
   -851,  -853,  -854,  -856,  -858,  -860,  -861,  -863,  
   -865,  -866,  -868,  -870,  -871,  -873,  -875,  -876,  
   -878,  -879,  -881,  -883,  -884,  -886,  -887,  -889,  
   -890,  -892,  -894,  -895,  -897,  -898,  -900,  -901,  
   -903,  -904,  -906,  -907,  -908,  -910,  -911,  -913,  
   -914,  -916,  -917,  -918,  -920,  -921,  -922,  -924,  
   -925,  -927,  -928,  -929,  -930,  -932,  -933,  -934,  
   -936,  -937,  -938,  -939,  -941,  -942,  -943,  -944,  
   -946,  -947,  -948,  -949,  -950,  -951,  -953,  -954,  
   -955,  -956,  -957,  -958,  -959,  -960,  -962,  -963,  
   -964,  -965,  -966,  -967,  -968,  -969,  -970,  -971,  
   -972,  -973,  -974,  -975,  -976,  -977,  -978,  -978,  
   -979,  -980,  -981,  -982,  -983,  -984,  -985,  -986,  
   -986,  -987,  -988,  -989,  -990,  -990,  -991,  -992,  
   -993,  -994,  -994,  -995,  -996,  -997,  -997,  -998,  
   -999,  -999,  -1000,  -1001,  -1001,  -1002,  -1003,  -1003,  
   -1004,  -1004,  -1005,  -1006,  -1006,  -1007,  -1007,  -1008,  
   -1008,  -1009,  -1009,  -1010,  -1010,  -1011,  -1011,  -1012,  
   -1012,  -1013,  -1013,  -1014,  -1014,  -1015,  -1015,  -1015,  
   -1016,  -1016,  -1017,  -1017,  -1017,  -1018,  -1018,  -1018,  
   -1019,  -1019,  -1019,  -1019,  -1020,  -1020,  -1020,  -1020,  
   -1021,  -1021,  -1021,  -1021,  -1022,  -1022,  -1022,  -1022,  
   -1022,  -1022,  -1023,  -1023,  -1023,  -1023,  -1023,  -1023,  
   -1023,  -1023,  -1023,  -1023,  -1023,  -1023,  -1023,  -1023,  
   -1024,  -1023,  -1023,  -1023,  -1023,  -1023,  -1023,  -1023,  
   -1023,  -1023,  -1023,  -1023,  -1023,  -1023,  -1023,  -1022,  
   -1022,  -1022,  -1022,  -1022,  -1022,  -1021,  -1021,  -1021,  
   -1021,  -1020,  -1020,  -1020,  -1020,  -1019,  -1019,  -1019,  
   -1019,  -1018,  -1018,  -1018,  -1017,  -1017,  -1017,  -1016,  
   -1016,  -1015,  -1015,  -1015,  -1014,  -1014,  -1013,  -1013,  
   -1012,  -1012,  -1011,  -1011,  -1010,  -1010,  -1009,  -1009,  
   -1008,  -1008,  -1007,  -1007,  -1006,  -1006,  -1005,  -1004,  
   -1004,  -1003,  -1003,  -1002,  -1001,  -1001,  -1000,  -999,  
   -999,  -998,  -997,  -997,  -996,  -995,  -994,  -994,  
   -993,  -992,  -991,  -990,  -990,  -989,  -988,  -987,  
   -986,  -986,  -985,  -984,  -983,  -982,  -981,  -980,  
   -979,  -978,  -978,  -977,  -976,  -975,  -974,  -973,  
   -972,  -971,  -970,  -969,  -968,  -967,  -966,  -965,  
   -964,  -963,  -962,  -960,  -959,  -958,  -957,  -956,  
   -955,  -954,  -953,  -951,  -950,  -949,  -948,  -947,  
   -946,  -944,  -943,  -942,  -941,  -939,  -938,  -937,  
   -936,  -934,  -933,  -932,  -930,  -929,  -928,  -927,  
   -925,  -924,  -922,  -921,  -920,  -918,  -917,  -916,  
   -914,  -913,  -911,  -910,  -908,  -907,  -906,  -904,  
   -903,  -901,  -900,  -898,  -897,  -895,  -894,  -892,  
   -890,  -889,  -887,  -886,  -884,  -883,  -881,  -879,  
   -878,  -876,  -875,  -873,  -871,  -870,  -868,  -866,  
   -865,  -863,  -861,  -860,  -858,  -856,  -854,  -853,  
   -851,  -849,  -847,  -846,  -844,  -842,  -840,  -839,  
   -837,  -835,  -833,  -831,  -829,  -828,  -826,  -824,  
   -822,  -820,  -818,  -816,  -814,  -813,  -811,  -809,  
   -807,  -805,  -803,  -801,  -799,  -797,  -795,  -793,  
   -791,  -789,  -787,  -785,  -783,  -781,  -779,  -777,  
   -775,  -773,  -771,  -769,  -767,  -765,  -762,  -760,  
   -758,  -756,  -754,  -752,  -750,  -748,  -745,  -743,  
   -741,  -739,  -737,  -735,  -732,  -730,  -728,  -726,  
   -724,  -721,  -719,  -717,  -715,  -712,  -710,  -708,  
   -706,  -703,  -701,  -699,  -696,  -694,  -692,  -690,  
   -687,  -685,  -683,  -680,  -678,  -675,  -673,  -671,  
   -668,  -666,  -664,  -661,  -659,  -656,  -654,  -652,  
   -649,  -647,  -644,  -642,  -639,  -637,  -634,  -632,  
   -629,  -627,  -625,  -622,  -620,  -617,  -615,  -612,  
   -609,  -607,  -604,  -602,  -599,  -597,  -594,  -592,  
   -589,  -587,  -584,  -581,  -579,  -576,  -574,  -571,  
   -568,  -566,  -563,  -561,  -558,  -555,  -553,  -550,  
   -547,  -545,  -542,  -539,  -537,  -534,  -531,  -529,  
   -526,  -523,  -521,  -518,  -515,  -512,  -510,  -507,  
   -504,  -501,  -499,  -496,  -493,  -491,  -488,  -485,  
   -482,  -479,  -477,  -474,  -471,  -468,  -466,  -463,  
   -460,  -457,  -454,  -451,  -449,  -446,  -443,  -440,  
   -437,  -434,  -432,  -429,  -426,  -423,  -420,  -417,  
   -414,  -412,  -409,  -406,  -403,  -400,  -397,  -394,  
   -391,  -388,  -386,  -383,  -380,  -377,  -374,  -371,  
   -368,  -365,  -362,  -359,  -356,  -353,  -350,  -347,  
   -344,  -342,  -339,  -336,  -333,  -330,  -327,  -324,  
   -321,  -318,  -315,  -312,  -309,  -306,  -303,  -300,  
   -297,  -294,  -291,  -288,  -285,  -282,  -279,  -276,  
   -273,  -270,  -267,  -264,  -260,  -257,  -254,  -251,  
   -248,  -245,  -242,  -239,  -236,  -233,  -230,  -227,  
   -224,  -221,  -218,  -215,  -212,  -209,  -205,  -202,  
   -199,  -196,  -193,  -190,  -187,  -184,  -181,  -178,  
   -175,  -171,  -168,  -165,  -162,  -159,  -156,  -153,  
   -150,  -147,  -144,  -140,  -137,  -134,  -131,  -128,  
   -125,  -122,  -119,  -115,  -112,  -109,  -106,  -103,  
   -100,  -97,  -94,  -90,  -87,  -84,  -81,  -78,  
   -75,  -72,  -69,  -65,  -62,  -59,  -56,  -53,  
   -50,  -47,  -43,  -40,  -37,  -34,  -31,  -28,  
   -25,  -21,  -18,  -15,  -12,  -9,  -6,  -3, 
};
*/

void setup()
{
  pinMode(PWM_PIN, OUTPUT);
  
  // Set up Timer 2 to do pulse width modulation on the speaker
  // pin.
  
  // Use internal clock (datasheet p.160)
  ASSR &= ~(_BV(EXCLK) | _BV(AS2));
  
  // Set fast PWM mode  (p.157)
  TCCR2A |= _BV(WGM21) | _BV(WGM20);
  TCCR2B &= ~_BV(WGM22);
  
  // Do non-inverting PWM on pin OC2A (p.155)
  // On the Arduino this is pin 11.
  TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
  TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));
  
  // No prescaler (p.158)
  TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);
  
  // Set initial pulse width to the first sample.
  OCR2A = 0x80;
  
  // Set up Timer 1 to send a sample every interrupt.
  
  cli();
  
  TIMSK0 &= (~TOIE0); // disable Timer0 - this means that delay() is no longer available
  
  // Set CTC mode (Clear Timer on Compare Match) (p.133)
  // Have to set OCR1A *after*, otherwise it gets reset to 0!
  TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
  TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));
  
  // No prescaler (p.134)
  TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);
  
  // Set the compare register (OCR1A).
  // OCR1A is a 16-bit register, so we have to do this with
  // interrupts disabled to be safe.
  OCR1A = 976;    // 16e6 / 16384
  
  // Enable interrupt when TCNT1 == OCR1A (p.136)
  TIMSK1 |= _BV(OCIE1A);
  sei();
  
  // set up the initial values for all the controls
  g_wave_type = 1; // sawtooth

  g_env_lengths[0] = 1000;
  g_env_lengths[1] = 1500;

  g_base_freq = 440;
  g_freq_ramp = 0;
  g_freq_ramp_cnt = 0;

  g_lpf_base_freq = 0;
  g_lpf_ramp = 0;
  g_lpf_resonance = 0;
  
  g_vib_speed = 0;
  g_vib_strength = 0;
  
  // start playing
  reset_sample();
  
  // input #1
  pinMode(4,INPUT);
  digitalWrite(4,HIGH);
  g_button_status = 0; // no buttons are pressed
}

void loop()
{
  uint8_t new_status; 

  // The loop is pretty simple - it just updates the parameters
  g_vib_speed = analogRead(VIBRATO_SPEED_CTRL) / 4;
  g_vib_strength = analogRead(VIBRATO_DEPTH_CTRL) / 4;
  g_lpf_base_freq = analogRead(LOWPASS_FREQ_CTRL) / 4;
  g_lpf_ramp = (char)((short)analogRead(LOWPASS_RAMP_CTRL) / 4 - 0x80);
  //g_lpf_resonance = analogRead(LOWPASS_RESONANCE_CTRL) / 4;

  new_status = digitalRead(4)==LOW ? 1 : 0;
  if ((g_button_status == 0) && (new_status == 1)) {
    reset_sample();
    g_env_stage = 0;
  }
  else if ((g_button_status == 1) && (new_status == 0) && (g_env_stage != 3)) {
    g_env_stage = 2;
  }

  g_button_status = new_status;

/*
  Slider(xpos, (ypos++)*18, p_env_attack, 0, "ATTACK TIME");
  Slider(xpos, (ypos++)*18, p_env_decay, 0, "DECAY TIME");

  Slider(xpos, (ypos++)*18, p_base_freq, 0, "START FREQUENCY");
  Slider(xpos, (ypos++)*18, p_freq_ramp, 1, "SLIDE");

  Slider(xpos, (ypos++)*18, p_vib_strength, 0, "VIBRATO DEPTH");
  Slider(xpos, (ypos++)*18, p_vib_speed, 0, "VIBRATO SPEED");

  Slider(xpos, (ypos++)*18, p_lpf_freq, 0, "LP FILTER CUTOFF");
  Slider(xpos, (ypos++)*18, p_lpf_ramp, 1, "LP FILTER CUTOFF SWEEP");
  Slider(xpos, (ypos++)*18, p_lpf_resonance, 0, "LP FILTER RESONANCE");
  Slider(xpos, (ypos++)*18, p_hpf_freq, 0, "HP FILTER CUTOFF");
  Slider(xpos, (ypos++)*18, p_hpf_ramp, 1, "HP FILTER CUTOFF SWEEP");
*/
}

static void reset_sample()
{
  g_phase = 0;
  g_curr_freq = g_base_freq;
  
  // reset filters
  g_lpf_prev = 0;
  g_lpf_prev_delta = 0;
  g_lpf_curr_freq = g_lpf_base_freq;
  
  // reset vibrato
  g_vib_phase = 0;
  g_vib_fix_accum = 0;
  
  // reset envelope
  g_env_stage=3;
  g_env_time=0;
}

/* interrupt handler - that's where the synthesis happens */
SIGNAL(PWM_INTERRUPT)
{
  short ssample;
  unsigned short fp;
  unsigned char env_vol;
  unsigned short vibrated_freq;
  short vib_fix;
  
  // use another oscillator with a lower frequency for the vibrato
  g_vib_phase += g_vib_speed;
  
  if(g_vib_strength > 0)
  {
    // vib_fix should be between -0x80 and 0x7f
    g_vib_fix_accum += phase_to_delta[g_vib_phase/0x4000];
    vib_fix = ((g_vib_fix_accum / 0x80) * (short)g_vib_strength) / 0x10;
    
    // fix the frequency according to the vibrato
    if ((vib_fix < 0) && ((unsigned short)(-vib_fix) >= g_curr_freq)) vibrated_freq = 1;
    else vibrated_freq = g_curr_freq + vib_fix;
  }
  else {
    vibrated_freq = g_curr_freq;
  }
  
  //
  // volume envelope
  //
  
  // compute by stage, keeping env_vol between zero and 0xff
  if (g_env_stage == 0) {
    g_env_time++;
    if (g_env_time >= g_env_lengths[0]) {
      g_env_time = 0;
      g_env_stage = 1;
    }
    env_vol=(unsigned char)(g_env_time / (g_env_lengths[0]/0x100 + 1));
  }
  else if (g_env_stage == 2) {
    g_env_time++;
    if (g_env_time >= g_env_lengths[1]) {
      g_env_time = 0;
      g_env_stage = 3;
    }
    env_vol=255-(unsigned char)(g_env_time / (g_env_lengths[1]/0x100 + 1));
  }
  else if (g_env_stage == 1) {
    env_vol = 255;
  }
  else {
    env_vol = 0;
  }
  
  //
  // phase of the current wave
  //
  
  g_phase += vibrated_freq;
  if (g_phase & 0xC000)
  {
    g_phase &= 0x3FFF;
    
    //
    // wave length wraparound. this is a good time to make changes in the frequency
    //
    
    // fix the frequency according to the ramp
    if (g_freq_ramp && (g_env_stage == 1)) {
      g_freq_ramp_cnt++;
      if (g_freq_ramp_cnt == g_freq_ramp) {
        g_freq_ramp_cnt = 0;
        g_curr_freq += 1;
      }
    }
  }
  
  //
  // current sample computation
  //
  
  // base waveform
  fp = g_phase >> 3; // keep fp between zero and 2047
  ssample = 0;
  if (g_wave_type < 2)
  {
    if (g_wave_type == 0) { // rough sawtooth
      ssample = (short)1023-(short)fp;
    }
    else { // sawtooth
      ssample = (fp < 1024) ? (short)1023-(short)fp*2 : (short)fp*2-3072;
    }
  }
  //else if (g_wave_type == 2) { // sine
  //  ssample = (short)pgm_read_word(&mysin_table[fp]);
  //}
  else { // square
    ssample = (fp & 1024) ? 1023 : 0;
  }
  
  // ssample is between -1024 and 1023
  
  //
  // low-pass filter
  //
  
  if (g_lpf_base_freq != 255) {
    // to avoid ramping the low-pass frequency too quickly, use a counter
    g_lpf_ramp_cnt++;
    // adjust the low-pass filter current frequency
    if ((g_lpf_ramp < 0) && (g_lpf_ramp_cnt == (unsigned char)-g_lpf_ramp)) {
      g_lpf_ramp_cnt = 0;
      if (g_lpf_curr_freq) g_lpf_curr_freq--;
    }
    else if ((g_lpf_ramp > 0) && (g_lpf_ramp_cnt == (unsigned char)g_lpf_ramp)) {
      g_lpf_ramp_cnt = 0;
      if (g_lpf_curr_freq + 1 != 0) g_lpf_curr_freq++;
    }
    
    if (g_lpf_curr_freq > 128) g_lpf_curr_freq = 128;
    
    g_lpf_prev_delta += (((short)ssample-(short)g_lpf_prev) / 0x100) * (short)g_lpf_curr_freq;
    //g_lpf_prev_delta -= (g_lpf_prev_delta * g_lpf_resonance / 0xff);
    g_lpf_prev += g_lpf_prev_delta;
    
    if (g_lpf_prev > 1023) g_lpf_prev = 1023;
    if (g_lpf_prev < -1024) g_lpf_prev = -1024;
    
    // filter output
    ssample = g_lpf_prev;
  }
  else {
    g_lpf_prev = ssample;
    g_lpf_prev_delta = 0;
  }
  
  // now sample is between -1024 and 1023
  // scale it between -128 and 127
  ssample=ssample/8;
  
  // adjust with the volume envelope
  ssample *= env_vol;
  ssample /= 256;
  
  PWM_VALUE=ssample + 128;
}

