/*
    WebAjustment.c -- Adjust screen display handler , based on goahead

    Copyright (c) All Rights Reserved. See details at the end of the file.
	2021/5/20
 */

#include "goahead.h"
#include "js.h"
#include <signal.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
//#include <webs.h>
#include <stdlib.h>
#include "globle.h"
#include <gio/gio.h>
#include "mvwebsvr.h"
#include "mvMethodComm_e.h"
#include "mvMethodDisplay_e.h"
#include "mvMethodTime_e.h"
#include "mvMethodDev_e.h"
#include "mvMethodVidPort_e.h"
#include "mvMethodVidSig_e.h"
#include "mvMethodOsd_e.h"

typedef enum PRESET_NUM_tag {
	RESTORE_CURRENT_PRESET,
	NEXT_PRESET,
	SAVE_PRESET,
	RESTORE_DEFAULT_PRESET
}PRESET_NUM;

typedef enum AJUST_TYE_tag {
	WEBADJ_BRIGHT = 0, //0
	WEBADJ_CONTRAST,
	WEBADJ_COLOR,
	WEBADJ_HUE,
	WEBADJ_SHARP,
	WEBADJ_NOISE,
	WEBADJ_PHASE,
	WEBADJ_FREQ, //7  
	WEBADJ_CLAMP,
	WEBADJ_BANDWIDTH,
	WEBADJ_AUTO_FULL,   //10 auto setup
	WEBADJ_AUTO_PHASE,
	WEBADJ_AUTO_GAIN,
	WEBADJ_AUTO_GEO, //13 
	WEBADJ_TEMP, //color temp
	WEBADJ_DISPLAY,
	WEBADJ_SCALING,
	WEBADJ_RESTORE, //17 settings Restore
	WEBADJ_NEXT,
	WEBADJ_SAVE,

	WEBADJ_DITHER,  //20
	WEBADJ_TEMP_TEMP,
	WEBADJ_TEMP_R,
	WEBADJ_TEMP_G,
	WEBADJ_TEMP_B,

	WEBADJ_DISPLAY_GAMMA,
	WEBADJ_SCALING_VAL1,	//26
	WEBADJ_SCALING_VAL2,

	WEBADJ_BANDWIDTH_VAL,   //28

	WEBADJ_INVIDINFO_NAME,   //29
	WEBADJ_INVIDINFO_SETID,	 //30
	WEBADJ_INVIDINFO_RESOL,  //31
	WEBADJ_INVIDINFO_VFRQ,   //32
	WEBADJ_INVIDINFO_HFRQ,   //33
	WEBADJ_INVIDINFO_PCLK,	 //34

	WEBADJ_VINID,   		//35

	WEBADJ_DP_Lane_ID,
	WEBADJ_DP_ADJ,

	WEBADJ_SCALING_ADJ_HSCAL,

	WEBADJ_BISVALID,

	WEBADJ_HSTART,//40
	WEBADJ_VSTART,

	WEBADJ_SETANALOGMODEOFF,
	WEBADJ_SETANALOGMODEON,

	WEBADJ_OVERLAY_CTRL_METHOD,

	VIDEOCFG_CNT
}AJUST_TYE;

static stSCAL_INPUT_DP_INFO stDpInfo;
static int LaneVL;
static int LaneEL;
static stDISP_COLORTEMP g_stCTemp;
static int g_uiVoutIndex;
static int bSignalFlag = 0;
static int g_uiLane_Id = 1;
static int g_uiVinIndex;


long long Get_Cur_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


#if 1
static int WebAdj_GetAjust(int eid, Webs * wp, int argc, char** argv)
{
	int iRet, type, data;
	char* pType;
	int uiVal;
	char* ptr;
	char strBuff[128];
	VISIZE stVidSize;
	int uiFmtPN;
	float fkHz;
	int uiIndex;

	static int uiVinIndex = -1;
	stVIDGOUT_SCAL stSigScal;
	stSCAL_INPUT_FREQ_INFO stVinFreqInfo;
	stSCAL_VSTART_INFO stVstartInfo;
	stSCAL_HSTART_INFO stHstartInfo;
	static stVIDSIGIN_STATUS stSigStat;

	static int getRoutflag = -1;
	
	//if (wp->query != NULL) printf("WebAdj_GetAjust : wp url:%s query=%s\n", wp->url, wp->query);

	if (wp->query != NULL)
	{
		ptr = strstr(wp->query, "vidoutid=");
		ptr += strlen("vidoutid=");
		g_uiVoutIndex = strtol(ptr, NULL, 10);
	}

	if (ejArgs(argc, argv, T("%s"), &pType) < 1) {
		websError(wp, 400, T("GetSlider Insufficient args\n"));
		printf("WebAdj_GetAjust : %s,get ejArgs failed\n", __FUNCTION__);
		return -1;
	}

	if (g_uiVoutIndex <eVIDSIGOUT_G1_REF || g_uiVoutIndex>eVIDSIGOUT_G2_LIV)
	{
		printf("WebAdj_GetAjust : Error! video-out index:%d\n", g_uiVoutIndex);
		return -1;
	}

	if (-1 == getRoutflag)
	{
		mvMethodDisp_VIDROUT_GetRouter(g_uiVoutIndex, &uiVinIndex);
		g_uiVinIndex = uiVinIndex;
		getRoutflag = 1;
	}
		
	type = strtol(pType, NULL, 10);
	switch (type) {
	case WEBADJ_BRIGHT:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vdisp_GetLightness(uiVinIndex, &uiVal);

			websWrite(wp, T("%d"), uiVal);
		}
		else
			websWrite(wp, T("%d"), 0);
		break;

	case WEBADJ_CONTRAST:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vdisp_GetContrast(uiVinIndex, &uiVal);
			websWrite(wp, T("%d"), uiVal);
		}
		else
			websWrite(wp, T("%d"), 0);
		break;

	case WEBADJ_COLOR:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vdisp_GetColor(uiVinIndex, &uiVal);
			websWrite(wp, T("%d"), uiVal);
		}
		else
			websWrite(wp, T("%d"), 0);
		break;

	case WEBADJ_HUE:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vdisp_GetHue(uiVinIndex, &uiVal);
			websWrite(wp, T("%d"), uiVal);
		}
		else
		{
			websWrite(wp, T("%d"), 0);
		}
		break;

	case WEBADJ_SHARP:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vdisp_GetSharpness(uiVinIndex, &uiVal);

			websWrite(wp, T("%d"), uiVal);
		}
		else
			websWrite(wp, T("%d"), 0);
		break;

	case WEBADJ_NOISE:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vin_GetNoiseReduction(uiVinIndex, &uiVal);

			websWrite(wp, T("%d"), uiVal);
		}
		else
			websWrite(wp, T("%d"), 0);
		break;

	case WEBADJ_PHASE:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vin_GetPhase(uiVinIndex, &uiVal);

			websWrite(wp, T("%d"), uiVal);
		}
		else
			websWrite(wp, T("%d"), 0);
		break;

	case WEBADJ_FREQ:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vin_GetFrequency(uiVinIndex, &stVinFreqInfo);
			
			if (stVinFreqInfo.uiFreq < 600 
			 || stVinFreqInfo.uiFreq_Min < 600
			 || stVinFreqInfo.uiFreq_Max < 600
			 || stVinFreqInfo.uiFreq > 4000 
			 || stVinFreqInfo.uiFreq_Min > 4000
			 || stVinFreqInfo.uiFreq_Max > 4000
			 || stVinFreqInfo.uiFreq_Min > stVinFreqInfo.uiFreq_Max
			 || stVinFreqInfo.uiFreq > stVinFreqInfo.uiFreq_Max
			 || stVinFreqInfo.uiFreq_Min > stVinFreqInfo.uiFreq)
				websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"), 600, 4000, 600);
			else
				websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"),
					stVinFreqInfo.uiFreq_Min, stVinFreqInfo.uiFreq_Max, stVinFreqInfo.uiFreq);
		}
		else
			websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"),
				600, 4000, 600);
		

		break;

	case WEBADJ_CLAMP:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vin_GetClampWidth(uiVinIndex, &uiVal);

			websWrite(wp, T("%d"), uiVal);
		}
		else
			websWrite(wp, T("%d"), 0);
		break;

	case WEBADJ_BANDWIDTH:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vin_GetBandWidth(uiVinIndex, &uiVal);

			if (uiVal == -1)
			{
				websWrite(wp, T("{\"data\":%d,\"val\":%d}"), -1, 0);
			}
			else
			{
				websWrite(wp, T("{\"data\":%d,\"val\":%d}"), 0, uiVal);
			}
		}
		else
		{
			websWrite(wp, T("{\"data\":%d,\"val\":%d}"), -1, 0);
		}
		break;

	case WEBADJ_TEMP:
		if (bSignalFlag == 1)
		{
			memset(&g_stCTemp, 0, sizeof(stDISP_COLORTEMP));
			iRet = mvMethodVidSig_Vdisp_GetCTemp(uiVinIndex, &g_stCTemp);
			g_stCTemp.uiDispSigId = uiVinIndex;

			if (g_stCTemp.uiCTempVal <= DISP_CTEMP_NATIVE) {
				websWrite(wp, T("{\"data\":%d,\"val\":%d,\"r\":%d,\"g\":%d,\"b\":%d}"),
					g_stCTemp.uiCTempVal, 0, 0, 0, 0);
			}
			else if (g_stCTemp.uiCTempVal == DISP_CTEMP_RGB) {

				websWrite(wp, T("{\"data\":%d,\"val\":%d,\"r\":%d,\"g\":%d,\"b\":%d}"),
					g_stCTemp.uiCTempVal, 0, g_stCTemp.uiCTR, g_stCTemp.uiCTG, g_stCTemp.uiCTB);
			}
			else {
				if (g_stCTemp.uiCTempVal > 13)
					websWrite(wp, T("{\"data\":%d,\"val\":%d,\"r\":%d,\"g\":%d,\"b\":%d}"), DISP_CTEMP_6500, 13 - DISP_CTEMP_6500, 0, 0, 0);
				else
					websWrite(wp, T("{\"data\":%d,\"val\":%d,\"r\":%d,\"g\":%d,\"b\":%d}"), DISP_CTEMP_6500, g_stCTemp.uiCTempVal - DISP_CTEMP_6500, 0, 0, 0);
			}
		}
		else
			websWrite(wp, T("{\"data\":%d,\"val\":%d,\"r\":%d,\"g\":%d,\"b\":%d}"),
				0, 0, 0, 0, 0);
		break;

	case WEBADJ_DISPLAY:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vdisp_GetGamma(uiVinIndex, &uiVal);
			if (uiVal <= DISP_GAMMA_NATIVE)
				websWrite(wp, T("{\"data\":%d,\"val\":%d}"), 0, 0);
			else if (uiVal >= DISP_GAMMA_DICOM)
			{
				uiVal -= 1;
				websWrite(wp, T("{\"data\":%d,\"val\":%d}"), 2, uiVal);
			}
			else
				websWrite(wp, T("{\"data\":%d,\"val\":%d}"), 0, 0);
		}
		else
			websWrite(wp, T("{\"data\":%d,\"val\":%d}"), 0, 0);
		break;

	case WEBADJ_SCALING:
		if (bSignalFlag == 1)
		{
			memset(&stSigScal, 0, sizeof(stVIDGOUT_SCAL));
			mvMethodVidSig_Vout_GetScaling(uiVinIndex, &stSigScal);
			
			if (stSigScal.uiScalVal < VIDGOUT_SCALVAL_NATIVE || stSigScal.uiScalVal >= VIDGOUT_SCALVAL_Cnt)
				stSigScal.uiScalVal = VIDGOUT_SCALVAL_NATIVE;

			if ((stSigScal.uiScalVal <= VIDGOUT_SCALVAL_FULL) || (stSigScal.uiScalVal == VIDGOUT_SCALVAL_4_3)) {
				websWrite(wp, T("{\"data\":%d,\"val1\":%d,\"val2\":%d}"), stSigScal.uiScalVal, 0, 0);
			}
			else {
				websWrite(wp, T("{\"data\":%d,\"val1\":%d,\"val2\":%d}"),
					VIDGOUT_SCALVAL_ADJUST, stSigScal.uiAdjVal, stSigScal.uiFramHth);
			}
		}
		else
			websWrite(wp, T("{\"data\":%d,\"val1\":%d,\"val2\":%d}"), 0, 0, 0);
		break;

	case WEBADJ_DITHER:
		if (bSignalFlag == 1)
		{
			mvMethodVidSig_Vdisp_GetDither(uiVinIndex, &uiVal);

			uiVal = uiVal <= 0 ? 0 : 1;
			websWrite(wp, "%d", uiVal);
		}
		else
			websWrite(wp, "%d", 0);
		break;

	case WEBADJ_INVIDINFO_NAME:

		memset(strBuff, 0, sizeof(strBuff));
		mvMethodVidPort_Vin_GetName(uiVinIndex, strBuff);
		printf("WebAdj_GetAjust : WEBADJ_INVIDINFO_NAME : %d %s\n", uiVinIndex, strBuff);

		websWrite(wp, T("%s"), strBuff);

		break;

	case WEBADJ_INVIDINFO_SETID:
		memset(&stSigStat, 0, sizeof(stSigStat));
		mvMethodVidSig_Vin_GetStatus(uiVinIndex, &stSigStat);
		websWrite(wp, T("%d"), stSigStat.uiSigInId);
		break;

	case WEBADJ_INVIDINFO_RESOL:

		if (bSignalFlag == 1)
		{
			if (stSigStat.bIsInterLace != 1)
				websWrite(wp, T("%d * %d"), stSigStat.uiFrameHActive, stSigStat.uiFrameVActive);
			else
				websWrite(wp, T("%d * %di"), stSigStat.uiFrameHActive, stSigStat.uiFrameVActive * 2);
		}
		else
			websWrite(wp, T("%d * %d"), 0, 0);

		getRoutflag = -1;

		break;

	case WEBADJ_INVIDINFO_VFRQ:
		if (bSignalFlag == 1)
		{

			fkHz = (float)stSigStat.uiFrameVFreqP10 / 10;
			if (stSigStat.bIsInterLace == 1)
				websWrite(wp, T("%.2f Hz"), fkHz);
			else
				websWrite(wp, T("%.2f Hz"), fkHz);
		}
		else
			websWrite(wp, T("%.2f Hz"), 0);
		break;

	case WEBADJ_INVIDINFO_HFRQ:
		if (bSignalFlag == 1)
		{

			fkHz = (float)stSigStat.uiFrameHFreqP10K / 10;
			websWrite(wp, T("%.2f kHz "), fkHz);
		}
		else
			websWrite(wp, T("%.2f kHz "), 0);
		break;

	case WEBADJ_INVIDINFO_PCLK:
		if (bSignalFlag == 1)
		{

			fkHz = (float)stSigStat.uiPixelClkP10M / 10;
			websWrite(wp, T("%.2f MHz "), fkHz);
		}
		else
			websWrite(wp, T("%.2f MHz "), 0);
		
		getRoutflag = -1;

		break;

	case WEBADJ_VINID:
		websWrite(wp, T("%d"), uiVinIndex);
		break;
	case WEBADJ_BISVALID:


		memset(&stSigStat, 0, sizeof(stSigStat));
		mvMethodVidSig_Vin_GetStatus(uiVinIndex, &stSigStat);

		if (stSigStat.bIsValid != 1)
		{
			bSignalFlag = 0;
			websWrite(wp, T("%d"), 0);
		}
		else
		{
			bSignalFlag = 1;
			websWrite(wp, T("%d"), 1);
		}

		printf("WebAdj_GetAjust : WEBADJ_BISVALID : %d\n", bSignalFlag);
		
		
		break;
	case WEBADJ_HSTART:
		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vin_GetHStart(uiVinIndex, &stHstartInfo);
			if (stHstartInfo.uiHStart_Min > stHstartInfo.uiHStart_Max
			 || stHstartInfo.uiHStart_Min > stHstartInfo.uiHStart
			 || stHstartInfo.uiHStart > stHstartInfo.uiHStart_Max)
				websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"), 0, 400, 0);
			else
				websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"),
					stHstartInfo.uiHStart_Min, stHstartInfo.uiHStart_Max, stHstartInfo.uiHStart);
						
		}
		else
			websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"),
				0, 400, 0);

		getRoutflag = -1;
		break;

	case WEBADJ_VSTART:

		if (bSignalFlag == 1)
		{
			iRet = mvMethodVidSig_Vin_GetVStart(uiVinIndex, &stVstartInfo);
			if (stVstartInfo.uiVStart_Min > stVstartInfo.uiVStart_Max
			 || stVstartInfo.uiVStart_Min > stVstartInfo.uiVStart
			 || stVstartInfo.uiVStart > stVstartInfo.uiVStart_Max)
				websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"), 0, 400, 0);
			else
				websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"),
					stVstartInfo.uiVStart_Min, stVstartInfo.uiVStart_Max, stVstartInfo.uiVStart);
			
		}
		else
			websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"),
				0, 400, 0);

		getRoutflag = -1;
		
		break;

	case WEBADJ_SETANALOGMODEOFF:

		mvMethodVidSig_Vdisp_SetAnalogModeDebug(0);
		websWrite(wp, T("%d"), 0);
		break;

	case WEBADJ_SETANALOGMODEON:

		mvMethodVidSig_Vdisp_SetAnalogModeDebug(1);
		websWrite(wp, T("%d"), 1);
		break;	

	case WEBADJ_OVERLAY_CTRL_METHOD:
		mvMethodVidSig_Vdisp_GetOverlayCtrlMethod(&uiIndex);
		websWrite(wp, T("%d"), uiIndex);
		break;
	}
	return 0;
}
#endif
static int WebAdj_GetPresetParams(int eid, Webs* wp, int argc, char** argv)
{
	int iRet, type, data;
	char* pType;
	int uiVal;
	char* ptr;
	char strBuff[128];
	VISIZE stVidSize;
	int uiFmtPN;
	float fkHz;

	int uiVinIndex;
	stSCAL_PRESET_PARAMS_INFO stPrstInfo;

	if (wp->query != NULL)
	{
		ptr = strstr(wp->query, "vidoutid=");
		ptr += strlen("vidoutid=");
		g_uiVoutIndex = strtol(ptr, NULL, 10);
	}

	if (ejArgs(argc, argv, T("%s"), &pType) < 1) {
		websError(wp, 400, T("GetSlider Insufficient args\n"));
		printf("%s,get ejArgs failed\n", __FUNCTION__);
		return -1;
	}

	if (g_uiVoutIndex <eVIDSIGOUT_G1_REF || g_uiVoutIndex>eVIDSIGOUT_G2_LIV)
	{
		printf("WebAdj_GetPresetParams : Error! video-out index:%d\n", g_uiVoutIndex);
		return -1;
	}
	mvMethodDisp_VIDROUT_GetRouter(g_uiVoutIndex, &uiVinIndex);


	type = strtol(pType, NULL, 10);
	mvMethodVidSig_Vdisp_GetPresetParams(uiVinIndex, &stPrstInfo);

	websWrite(wp,
		T("{\"bDither\":%d,\"uiBrightness\":%d,\"uiContrast\":%d,\"uiColor\":%d,\"uiHue\":%d,\"uiSharpness\":%d,\"uiCTempVal\":%d,\"uiCTR\":%d,\"uiCTG\":%d,\"uiCTB\":%d,\"uiGamma\":%d,\"uiScalVal\":%d,\"uiScalAdjVal\":%d,\"uiNR\":%d,\"uiClampwidth\":%d,\"BandwidthValue\":%d,\"uiVGAPhase\":%d,\"uiVGAHtotal\":%d,\"uiVGAHtotal_Min\":%d,\"uiVGAHtotal_Max\":%d}"),
		stPrstInfo.bDither, stPrstInfo.uiBrightness, stPrstInfo.uiContrast, stPrstInfo.uiColor, stPrstInfo.uiHue,
		stPrstInfo.uiSharpness, stPrstInfo.uiCTempVal, stPrstInfo.uiCTR, stPrstInfo.uiCTG, stPrstInfo.uiCTB,
		stPrstInfo.uiGamma, stPrstInfo.uiScalVal, stPrstInfo.uiScalAdjVal, stPrstInfo.uiNR, stPrstInfo.uiClampwidth,
		stPrstInfo.BandwidthValue, stPrstInfo.uiVGAPhase, stPrstInfo.uiVGAHtotal, stPrstInfo.uiVGAHtotal_Min,
		stPrstInfo.uiVGAHtotal_Max);

	return 0;
}


#define MAX_CMD_ECHO_SPAN_MS 100
long g_last_cmd_time = 1;

static void WebAdj_SetAjust(Webs* wp)
{
	
	int iRet, type, data;
	char* pType, * pData;
	int uiVinIndex;
	int uiVal, uiR, uiG, uiB;
	int uiCnt;
	int uiHScalVal;
	static int uiTmpType;
	static int uiMinMaxVal = 0;
	long lCurCmdTime;
	int autoCnt = 20;
	int autoRet;

	char strBuff[128];
	stVIDGOUT_SCAL stSigScal;
	stSCAL_INPUT_FREQ_INFO stVinFreqInfo;
	
	type = strtol(websGetVar(wp, T("type"), T("0")), NULL, 10);
	data = strtol(websGetVar(wp, T("data"), T("0")), NULL, 10);

	if (g_uiVoutIndex <eVIDSIGOUT_G1_REF || g_uiVoutIndex>eVIDSIGOUT_G2_LIV)
	{
		printf("WebAdj_SetAjust : Notice! video-out index:%d\n", g_uiVoutIndex);
	}

	if (g_uiVoutIndex == eVIDSIGOUT_G1_REF)
	{
		mvMethodDisp_VIDROUT_GetRouter(g_uiVoutIndex, &uiVinIndex);
	}
	else if (g_uiVoutIndex == eVIDSIGOUT_G2_LIV)
	{
		uiVinIndex = g_uiVinIndex;
	}
	
	printf("[smvwebsvr print] : WebAdj_SetAjust : uiVinIndex = %d\n", uiVinIndex);


	switch (type) {
	case WEBADJ_BRIGHT:

		iRet = mvMethodVidSig_Vdisp_SetLightness(uiVinIndex, data);

		break;
	case WEBADJ_CONTRAST:

		iRet = mvMethodVidSig_Vdisp_SetContrast(uiVinIndex, data);

		break;
	case WEBADJ_COLOR:

		iRet = mvMethodVidSig_Vdisp_SetColor(uiVinIndex, data);
		break;
	case WEBADJ_HUE:

		iRet = mvMethodVidSig_Vdisp_SetHue(uiVinIndex, data);
		break;
	case WEBADJ_SHARP:

		iRet = mvMethodVidSig_Vdisp_SetSharpness(uiVinIndex, data);
		break;
	case WEBADJ_NOISE:

		iRet = mvMethodVidSig_Vin_SetNoiseReduction(uiVinIndex, data);
		break;
	case WEBADJ_PHASE:

		iRet = mvMethodVidSig_Vin_SetPhase(uiVinIndex, data);
		break;
	case WEBADJ_FREQ:
	
		stVinFreqInfo.uiFreq = data;

		iRet = mvMethodVidSig_Vin_SetFrequency(uiVinIndex, &stVinFreqInfo);
		break;

	case WEBADJ_CLAMP:


		iRet = mvMethodVidSig_Vin_SetClampWidth(uiVinIndex, data);
		break;
	case WEBADJ_BANDWIDTH:

		uiVal = strtol(websGetVar(wp, T("tempval"), T("0")), NULL, 10);

		iRet = mvMethodVidSig_Vin_SetBandWidth(uiVinIndex, uiVal);
		break;
	case WEBADJ_AUTO_FULL:
	case WEBADJ_AUTO_PHASE:
	case WEBADJ_AUTO_GAIN:
	case WEBADJ_AUTO_GEO:

		printf("[websvr info] : WebAdj_SetAjust : VGA_AUTO : data=%d\n", data);
		if (uiVinIndex != 4 && uiVinIndex != 6) break;
		iRet = mvMethodVidSig_Vdisp_AutoSet(uiVinIndex, data, 1);
		sleep(5);
		autoCnt = 60;
		while(autoCnt)
		{
			autoRet = mvMethodVidSig_Vdisp_GetAutoSet();
			if(autoRet == 0)
			{
				sleep(1);
				autoCnt--;
				continue;
			}
			else if (autoRet == 1)
			{
				//auto success
				iRet = 1;
				break;
			}
			else
			{
				//auto fail
				iRet = 2;
				break;
			}
		}

		//sleep(20);

		break;
	case WEBADJ_TEMP:
		g_stCTemp.uiDispSigId = uiVinIndex;

		uiVal = strtol(websGetVar(wp, T("tempval"), T("0")), NULL, 10);
		uiR = strtol(websGetVar(wp, T("r"), T("0")), NULL, 10);
		uiG = strtol(websGetVar(wp, T("g"), T("0")), NULL, 10);
		uiB = strtol(websGetVar(wp, T("b"), T("0")), NULL, 10);


		if (data == DISP_CTEMP_NATIVE) {
			g_stCTemp.uiCTempVal = DISP_CTEMP_NATIVE;

		}if (data == DISP_CTEMP_RGB) {
			g_stCTemp.uiCTempVal = DISP_CTEMP_RGB;

			g_stCTemp.uiCTR = uiR;
			g_stCTemp.uiCTG = uiG;
			g_stCTemp.uiCTB = uiB;


		}
		else if (data == DISP_CTEMP_6500)
		{
			g_stCTemp.uiCTempVal = uiVal + DISP_CTEMP_6500;

		}
		else g_stCTemp.uiCTempVal = DISP_CTEMP_NATIVE;


		iRet = mvMethodVidSig_Vdisp_SetCTemp(uiVinIndex, &g_stCTemp);
		break;

	case WEBADJ_TEMP_R:
		g_stCTemp.uiCTR = data;
		iRet = mvMethodVidSig_Vdisp_SetCTemp(uiVinIndex, &g_stCTemp);
		break;

	case WEBADJ_TEMP_G:
		g_stCTemp.uiCTG = data;
		iRet = mvMethodVidSig_Vdisp_SetCTemp(uiVinIndex, &g_stCTemp);
		break;

	case WEBADJ_TEMP_B:
		g_stCTemp.uiCTB = data;
		iRet = mvMethodVidSig_Vdisp_SetCTemp(uiVinIndex, &g_stCTemp);
		break;

	case WEBADJ_DISPLAY:
		if (data <= DISP_GAMMA_NATIVE)uiVal = DISP_GAMMA_NATIVE;
		else if (data == DISP_GAMMA_DICOM)uiVal = DISP_GAMMA_DICOM;
		else if (data == DISP_GAMMA_1_8)
		{
			uiVal = strtol(websGetVar(wp, T("tempval"), T("0")), NULL, 10);
		
			uiVal += 1;
		}

		iRet = mvMethodVidSig_Vdisp_SetGamma(uiVinIndex, uiVal);
		break;


	case WEBADJ_DISPLAY_GAMMA:
		uiVal = data;
		uiVal += 1;

		iRet = mvMethodVidSig_Vdisp_SetGamma(uiVinIndex, uiVal);

		break;

	case WEBADJ_SCALING:
		memset(&stSigScal, 0, sizeof(stSigScal));

		stSigScal.uiScalVal = data;
		stSigScal.uiSigId = uiVinIndex;
		if (data == VIDGOUT_SCALVAL_ADJUST) {
			stSigScal.uiAdjVal = strtol(websGetVar(wp, T("val1"), T("0")), NULL, 10);
			stSigScal.uiFramHth = strtol(websGetVar(wp, T("val2"), T("0")), NULL, 10);
		}


		iRet = mvMethodVidSig_Vout_SetScaling(uiVinIndex, &stSigScal);
		break;



	case WEBADJ_SCALING_ADJ_HSCAL:
		memset(&stSigScal, 0, sizeof(stSigScal));
		
		uiHScalVal = strtol(websGetVar(wp, T("hscalval"), T("0")), NULL, 10);
		
		stSigScal.uiScalVal = VIDGOUT_SCALVAL_ADJUST;
		stSigScal.uiAdjVal = data;
		stSigScal.uiSigId = uiVinIndex;
		stSigScal.uiFramHth = uiHScalVal;
		iRet = mvMethodVidSig_Vout_SetScaling(uiVinIndex, &stSigScal);
		break;


	case WEBADJ_RESTORE:
		iRet = mvMethodVidSig_Vin_SetOnOff(uiVinIndex, 0);
		uiCnt = 6;
		do
		{
			mvMethodVidSig_Vin_GetOnOff(uiVinIndex, &uiVal);
			if (uiVal == MV_TRUE)break;
			uiCnt--;
		} while (uiCnt > 0);

		memset(strBuff, 0, sizeof(strBuff));
		sprintf(strBuff, "/Adjustments.html?vidoutid=%d", g_uiVoutIndex);
		websRedirect(wp, strBuff);
		break;
	case WEBADJ_NEXT:
		iRet = mvMethodDevStat_Restore(1);
		break;
	case WEBADJ_SAVE:
		break;

	case WEBADJ_DITHER:
		if (data <= 0)uiVal = 0;
		else uiVal = 1;

		iRet = mvMethodVidSig_Vdisp_SetDither(uiVinIndex, uiVal);
		break;

	case WEBADJ_BANDWIDTH_VAL:
		
		uiVal = data;
		iRet = mvMethodVidSig_Vin_SetBandWidth(uiVinIndex, uiVal);
		break;


	case WEBADJ_DP_Lane_ID:
		
		if (g_uiVinIndex == 2 || g_uiVinIndex == 9 || g_uiVinIndex == 16)
		{
			if (data > 0 && data < 5)
			{
				stDpInfo.uiLane_Num = data;
				stDpInfo.uiLane_ID = 0;
			}
			else if (data > 9 && data < 14)
			{
				stDpInfo.uiLane_VL = data - 10;
				stDpInfo.uiLane_ID = g_uiLane_Id;
			}
			else if (data > 19 && data < 24)
			{
				stDpInfo.uiLane_ID = g_uiLane_Id;
				stDpInfo.uiLane_EL = data - 20;			
			}
			else if (data == 0)
			{
				stDpInfo.uiLane_VL = 255;
			}
			else if (data == -1)
			{
				stDpInfo.uiLane_EL = 255;
			}
			

			
			break;
		}
		else
			break;
	case WEBADJ_DP_ADJ:
		if (g_uiVinIndex == 2 || g_uiVinIndex == 9 || g_uiVinIndex == 16)
		{
			if (data > 0 && data < 5)
			{
				stDpInfo.uiLane_Num = data;
				stDpInfo.uiLane_ID = 0;

			}
			else if (data == 255)
			{
				stDpInfo.uiLane_Num = 255;
				stDpInfo.uiLane_ID = g_uiLane_Id;
				
			}
			else
			{
				stDpInfo.uiLane_VL = 255;
				stDpInfo.uiLane_EL = 255;
			}
			
			
	
			
			mvMethodVidSig_Vin_SetDpDebugInfo(uiVinIndex, &stDpInfo);
		}
		break;

	case WEBADJ_HSTART:

		iRet = mvMethodVidSig_Vin_SetHStart(uiVinIndex, data);
		break;

	case WEBADJ_VSTART:


		iRet = mvMethodVidSig_Vin_SetVStart(uiVinIndex, data);
		break;
	case WEBADJ_OVERLAY_CTRL_METHOD:
		iRet = mvMethodVidSig_Vdisp_SetOverlayCtrlMethod(data);
		
		break;

	default:
		iRet = -1;
		break;

	}

	//web   
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);
	websWrite(wp, T("ret:%d"), iRet);
	websFlush(wp, 0);
	websDone(wp);
}

void WebAdj_GetCTempVal(Webs* wp)
{
	int ret;
	stDISP_COLORTEMP stCTemp;

	memset(&stCTemp, 0, sizeof(stDISP_COLORTEMP));
	stCTemp.uiDispSigId = g_stCTemp.uiDispSigId;
	stCTemp.uiCTempVal = DISP_CTEMP_RGB;
	ret = mvMethodVidSig_Vdisp_GetCTemp(g_stCTemp.uiDispSigId, &stCTemp);
	if (ret != MV_SUCCESS)
	{
		printf("WebAdj_GetCTempVal : Notice! Get color temperature fail\n");
	}
	stCTemp.uiDispSigId = g_stCTemp.uiDispSigId;

	g_stCTemp.uiCTempVal = DISP_CTEMP_RGB;

	stCTemp.uiCTempVal = DISP_CTEMP_RGB;
	mvMethodVidSig_Vdisp_SetCTemp(g_stCTemp.uiDispSigId, &stCTemp);


	char buff[256];
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	memset(buff, 0, 256);
	sprintf(buff, " {\"data\":\"%d\",\"val\":\"%d\",\"r\":\"%d\",\"g\":\"%d\",\"b\":\"%d\"}",
		stCTemp.uiCTempVal,
		0,
		stCTemp.uiCTR,
		stCTemp.uiCTG,
		stCTemp.uiCTB);

	websWrite(wp, "[");
	websWrite(wp, buff);
	websWrite(wp, "]");

	websDone(wp);



}

void WebAdj_GetScaleVal(Webs* wp)
{
	int ret;
	stVIDGOUT_SCAL stSigScal;
	int uiVinIndex;
	mvMethodDisp_VIDROUT_GetRouter(g_uiVoutIndex, &uiVinIndex);

	memset(&stSigScal, 0, sizeof(stVIDGOUT_SCAL));
	stSigScal.uiSigId = uiVinIndex;
	stSigScal.uiScalVal = VIDGOUT_SCALVAL_ADJUST;
	mvMethodVidSig_Vout_GetScaling(uiVinIndex, &stSigScal);


	char buff[256];
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	memset(buff, 0, 256);
	sprintf(buff, " {\"data\":\"%d\",\"val1\":\"%d\",\"val2\":\"%d\"}",
		VIDGOUT_SCALVAL_ADJUST,
		stSigScal.uiAdjVal,
		stSigScal.uiFramHth);

	websWrite(wp, "[");
	websWrite(wp, buff);
	websWrite(wp, "]");

	websDone(wp);
}


static void WebAdj_SetPreset(Webs* wp)
{
	int iRet, type, data;
	char* pType, * pData;
	int uiVinIndex;
	int uiCnt;

	char strBuff[128];

	type = strtol(websGetVar(wp, T("type"), T("0")), NULL, 10);
	data = strtol(websGetVar(wp, T("data"), T("0")), NULL, 10);

	if (g_uiVoutIndex <eVIDSIGOUT_G1_REF || g_uiVoutIndex>eVIDSIGOUT_G2_LIV)
	{
		printf("WebAdj_SetPreset : Notice! video-out index:%d\n", g_uiVoutIndex);
	}

	mvMethodDisp_VIDROUT_GetRouter(g_uiVoutIndex, &uiVinIndex);

	switch (type) {
	case RESTORE_CURRENT_PRESET:

		iRet = mvMethodVidSig_Vdisp_RestoreCurrentPreset(uiVinIndex, 0);
		sleep(3);
		break;
	case NEXT_PRESET:

		iRet = mvMethodVidSig_Vdisp_SelectNextPreset(uiVinIndex, 0);
		sleep(3);

		break;
	case SAVE_PRESET:

		iRet = mvMethodVidSig_Vdisp_SaveCurrentPreset(uiVinIndex, data);
		sleep(3);

		break;
	case RESTORE_DEFAULT_PRESET:

		iRet = mvMethodVidSig_Vdisp_ClearAllPreset(uiVinIndex, data);
		sleep(3);

		break;
	default:
		iRet = -1;
		break;

	}

	//web   
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);
	websWrite(wp, T("ret:%d"), iRet);
	websFlush(wp, 0);
	websDone(wp);
}

static int WebAdj_GetDpDebugInfo(Webs* wp)
{	
	memset(&stDpInfo, 0, sizeof(stDpInfo));

	stDpInfo.uiLane_ID = g_uiLane_Id;

	mvMethodVidSig_Vin_GetDpDebugInfo(g_uiVinIndex, &stDpInfo);

	char buff[256];
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	memset(buff, 0, 256);
	sprintf(buff, "{\"Lane_num\":%d,\"Lane_ID\":%d,\"Lane_VL\":%d,\"Lane_EL\":%d,\"Lane_CR\":%d,\"Lane_EQ\":%d}",
		stDpInfo.uiLane_Num, stDpInfo.uiLane_ID, stDpInfo.uiLane_VL,
		stDpInfo.uiLane_EL, stDpInfo.uiLane_CR, stDpInfo.uiLane_EQ);

	websWrite(wp, "[");
	websWrite(wp, buff);
	websWrite(wp, "]");
	websDone(wp);

	stDpInfo.uiLane_VL = 255;
	stDpInfo.uiLane_EL = 255;

	return 0;
}


static int WebAdj_GetDpDebugInfo1(Webs* wp)
{	
	memset(&stDpInfo, 0, sizeof(stDpInfo));

	stDpInfo.uiLane_ID = g_uiLane_Id;

	mvMethodVidSig_Vin_GetDpDebugInfo(g_uiVinIndex, &stDpInfo);

	char buff[256];
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	memset(buff, 0, 256);
	sprintf(buff, "{\"Lane_num\":%d,\"Lane_ID\":%d,\"Lane_VL\":%d,\"Lane_EL\":%d,\"Lane_CR\":%d,\"Lane_EQ\":%d}",
		stDpInfo.uiLane_Num, stDpInfo.uiLane_ID, stDpInfo.uiLane_VL,
		stDpInfo.uiLane_EL, stDpInfo.uiLane_CR, stDpInfo.uiLane_EQ);


	websWrite(wp, "[");
	websWrite(wp, buff);
	websWrite(wp, "]");
	websDone(wp);

	return 0;
}

static int WebAdj_GetPhase(Webs* wp)
{
	int uiVinIndex;
	int uival;

	mvMethodVidSig_Vin_GetPhase(g_uiVinIndex, &uival);
	printf("[websvr info] : WebAdj_GetPhase : uiVinIndex=%d uiVal = %d\n", g_uiVinIndex, uival);

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	websWrite(wp, T("%d"), uival);

	websDone(wp);


	return 0;
}


static int WebAdj_GetFrequency(Webs* wp)
{
	int uiVinIndex;
	stSCAL_INPUT_FREQ_INFO stVinFreqInfo;
	memset(&stVinFreqInfo, 0, sizeof(stVinFreqInfo));

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	mvMethodVidSig_Vin_GetFrequency(g_uiVinIndex, &stVinFreqInfo);

	if (stVinFreqInfo.uiFreq < 600 
	 || stVinFreqInfo.uiFreq_Min < 600
	 || stVinFreqInfo.uiFreq_Max < 600
	 || stVinFreqInfo.uiFreq > 4000 
	 || stVinFreqInfo.uiFreq_Min > 4000
	 || stVinFreqInfo.uiFreq_Max > 4000
	 || stVinFreqInfo.uiFreq_Min > stVinFreqInfo.uiFreq_Max
	 || stVinFreqInfo.uiFreq > stVinFreqInfo.uiFreq_Max
	 || stVinFreqInfo.uiFreq_Min > stVinFreqInfo.uiFreq)
		websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"), 600, 4000, 600);
	else
		websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"),
			stVinFreqInfo.uiFreq_Min, stVinFreqInfo.uiFreq_Max, stVinFreqInfo.uiFreq);

	printf("[websvr info] : WebAdj_GetFrequency : uiVinIndex=%d uiVal=%d min=%d max=%d\n", 
			g_uiVinIndex, stVinFreqInfo.uiFreq,stVinFreqInfo.uiFreq_Min, stVinFreqInfo.uiFreq_Max);
		

	websDone(wp);
	
	return 0;
}


static int WebAdj_GetHTotal(Webs* wp)
{
	int uiVinIndex;
	stSCAL_INPUT_FREQ_INFO stVinFreqInfo;
	memset(&stVinFreqInfo, 0, sizeof(stVinFreqInfo));

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	mvMethodDisp_VIDROUT_GetRouter(1, &uiVinIndex);

	mvMethodVidSig_Vin_GetFrequency(uiVinIndex, &stVinFreqInfo);
	if (stVinFreqInfo.uiFreq < 600 
	 || stVinFreqInfo.uiFreq_Min < 600
	 || stVinFreqInfo.uiFreq_Max < 600
	 || stVinFreqInfo.uiFreq > 4000 
	 || stVinFreqInfo.uiFreq_Min > 4000
	 || stVinFreqInfo.uiFreq_Max > 4000
	 || stVinFreqInfo.uiFreq_Min > stVinFreqInfo.uiFreq_Max
	 || stVinFreqInfo.uiFreq > stVinFreqInfo.uiFreq_Max
	 || stVinFreqInfo.uiFreq_Min > stVinFreqInfo.uiFreq)
		websWrite(wp, T("[{\"min\":%d,\"max\":%d,\"val\":%d}]"), 600, 4000, 600);
	else
		websWrite(wp, T("[{\"min\":%d,\"max\":%d,\"val\":%d}]"),
			stVinFreqInfo.uiFreq_Min, stVinFreqInfo.uiFreq_Max, stVinFreqInfo.uiFreq);

	printf("[websvr info] : WebAdj_GetFrequency : uiVinIndex=%d uiVal=%d min=%d max=%d\n", 
			uiVinIndex, stVinFreqInfo.uiFreq,stVinFreqInfo.uiFreq_Min, stVinFreqInfo.uiFreq_Max);
		

	websDone(wp);
	
	return 0;
}


static int WebAdj_GetVstart(Webs* wp)
{
	int uiVinIndex;
	stSCAL_VSTART_INFO stVstartInfo;
	memset(&stVstartInfo, 0, sizeof(stVstartInfo));

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	mvMethodVidSig_Vin_GetVStart(g_uiVinIndex, &stVstartInfo);
	websWrite(wp, T("[{\"min\":%d,\"max\":%d,\"val\":%d}]"),
				stVstartInfo.uiVStart_Min, stVstartInfo.uiVStart_Max, stVstartInfo.uiVStart);

	printf("[websvr info] : WebAdj_GetVstart : uiVinIndex=%d uiVal=%d min=%d max=%d\n", 
			g_uiVinIndex, stVstartInfo.uiVStart, stVstartInfo.uiVStart_Min, stVstartInfo.uiVStart_Max);
		

	websDone(wp);
	
	return 0;
}


static int WebAdj_GetHstart(Webs* wp)
{
	int uiVinIndex;
	int uival;
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);
	
	stSCAL_HSTART_INFO stHstartInfo;
	memset(&stHstartInfo, 0, sizeof(stHstartInfo));
	
	mvMethodVidSig_Vin_GetHStart(g_uiVinIndex, &stHstartInfo);

	if (stHstartInfo.uiHStart_Min > stHstartInfo.uiHStart_Max
			 || stHstartInfo.uiHStart_Min > stHstartInfo.uiHStart
			 || stHstartInfo.uiHStart > stHstartInfo.uiHStart_Max)
		websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"), 0, 400, 0);
	else
		websWrite(wp, T("{\"min\":%d,\"max\":%d,\"val\":%d}"),
			stHstartInfo.uiHStart_Min, stHstartInfo.uiHStart_Max, stHstartInfo.uiHStart);


	printf("[websvr info] : WebAdj_GetVstart : uiVinIndex=%d uiVal=%d min=%d max=%d\n", 
			g_uiVinIndex, stHstartInfo.uiHStart, stHstartInfo.uiHStart_Min, stHstartInfo.uiHStart_Max);
		

	websDone(wp);


	return 0;
}


static int WebAdj_GetVstart_new(Webs* wp)
{
	int uiVinIndex;
	stSCAL_VSTART_INFO stVstartInfo;
	memset(&stVstartInfo, 0, sizeof(stVstartInfo));

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	mvMethodDisp_VIDROUT_GetRouter(1, &uiVinIndex);


	mvMethodVidSig_Vin_GetVStart(uiVinIndex, &stVstartInfo);
	if (stVstartInfo.uiVStart_Min > stVstartInfo.uiVStart_Max
			 || stVstartInfo.uiVStart_Min > stVstartInfo.uiVStart
			 || stVstartInfo.uiVStart > stVstartInfo.uiVStart_Max)
		websWrite(wp, T("[{\"min\":%d,\"max\":%d,\"val\":%d}]"), 0, 400, 0);
	else
		websWrite(wp, T("[{\"min\":%d,\"max\":%d,\"val\":%d}]"),
			stVstartInfo.uiVStart_Min, stVstartInfo.uiVStart_Max, stVstartInfo.uiVStart);

	printf("[websvr info] : WebAdj_GetVstart : uiVinIndex=%d uiVal=%d min=%d max=%d\n", 
			uiVinIndex, stVstartInfo.uiVStart, stVstartInfo.uiVStart_Min, stVstartInfo.uiVStart_Max);
		

	websDone(wp);
	
	return 0;
}


static int WebAdj_GetHstart_new(Webs* wp)
{
	int uiVinIndex;
	int uival;
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	mvMethodDisp_VIDROUT_GetRouter(1, &uiVinIndex);
	
	stSCAL_HSTART_INFO stHstartInfo;
	memset(&stHstartInfo, 0, sizeof(stHstartInfo));
	
	mvMethodVidSig_Vin_GetHStart(uiVinIndex, &stHstartInfo);

	if (stHstartInfo.uiHStart_Min > stHstartInfo.uiHStart_Max
			 || stHstartInfo.uiHStart_Min > stHstartInfo.uiHStart
			 || stHstartInfo.uiHStart > stHstartInfo.uiHStart_Max)
		websWrite(wp, T("[{\"min\":%d,\"max\":%d,\"val\":%d}]"), 0, 400, 0);
	else
		websWrite(wp, T("[{\"min\":%d,\"max\":%d,\"val\":%d}]"),
			stHstartInfo.uiHStart_Min, stHstartInfo.uiHStart_Max, stHstartInfo.uiHStart);

	printf("[websvr info] : WebAdj_GetVstart : uiVinIndex=%d uiVal=%d min=%d max=%d\n", 
			uiVinIndex, stHstartInfo.uiHStart, stHstartInfo.uiHStart_Min, stHstartInfo.uiHStart_Max);
		

	websDone(wp);


	return 0;
}



static int WebAdj_GetResol(Webs* wp)
{
	int uiVinIndex;
	int uival;
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	stVIDSIGIN_STATUS stSigStat;

	mvMethodDisp_VIDROUT_GetRouter(1, &uiVinIndex);
	g_uiVoutIndex = 1;

	memset(&stSigStat, 0, sizeof(stSigStat));
	mvMethodVidSig_Vin_GetStatus(uiVinIndex, &stSigStat);

	if (stSigStat.bIsValid != 1)
	{
		if (uiVinIndex == 4 || uiVinIndex == 6)
			websWrite(wp, T("%d,%d"), 640, 480);
		else
			websWrite(wp, T("%d,%d"), 640, 240);
	}	
	else
	{
		if (stSigStat.bIsInterLace != 1)
			websWrite(wp, T("%d,%d"), stSigStat.uiFrameHActive, stSigStat.uiFrameVActive);
		else
			websWrite(wp, T("%d,%di"), stSigStat.uiFrameHActive, stSigStat.uiFrameVActive * 2);
	}
		
	websDone(wp);

	return 0;
}


static int WebAdj_GetAnalogOverlay(Webs* wp)
{

	mvMethodVidSig_Vdisp_GetAnalogOverlay();


	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	char buff[20480];
	memset(buff,0,sizeof(buff));
	char tmpbuff[256];
	memset(tmpbuff,0,sizeof(tmpbuff));
				
	for(int i = 0; i < g_groupnum; i++)
	{
		memset(tmpbuff,0,sizeof(tmpbuff));	
		strcat(buff,"{");

		if (0 == strlen(stAnalogInputParams[i].PresetName))
			memcpy(stAnalogInputParams[i].PresetName, "null", sizeof(stAnalogInputParams[i].PresetName));

        sprintf(tmpbuff, "\"ModeIndex\":%d,\"PresetName\":\"%s\",\"H_Freq\":%d,\"V_Freq\":%d,\"V_Total\":%d,\"H_Total\":%d,\"H_Active\":%d,\"V_Active\":%d,\"H_Sync\":%d,\"V_Sync\":%d,\"V_Sync_W\":%d,\"ValidPortFlag\":%d,\"ModeType\":%d",
		stAnalogInputParams[i].uiModeIndex, stAnalogInputParams[i].PresetName, 
		stAnalogInputParams[i].uiH_Freq, stAnalogInputParams[i].uiV_Freq, 
		stAnalogInputParams[i].uiV_Total, stAnalogInputParams[i].uiH_Total,
		stAnalogInputParams[i].uiH_Active, stAnalogInputParams[i].uiV_Active,
		stAnalogInputParams[i].uiH_Sync_p, stAnalogInputParams[i].uiV_Sync_p, stAnalogInputParams[i].uiV_Sync_Width,
		stAnalogInputParams[i].uiValidPortFlag, stAnalogInputParams[i].ModeType);
	
		strcat(buff,tmpbuff);	
		strcat(buff,"}");
		if(i < g_groupnum - 1)strcat(buff,",");
	}				
				
	printf("WebAdj_GetAnalogOverlay : %s\n", buff);
	
	websWrite(wp, "[");
	websWrite(wp, T("{index:%d}"), g_curIndex);

	if (strlen(buff))
		websWrite(wp, ",");
		
	websWrite(wp, buff);
	websWrite(wp, "]");
	websDone(wp);
	return 0;
}



static int WebAdj_GetAnalogInputParams(Webs* wp)
{

	int uiVinIndex;
	stSCAL_ANALOG_INPUT_PARAMS Analog;
	int i;
	
	mvMethodVidSig_Vdisp_GetAnalogInputParams(&Analog);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);
	char buff[1024];	
	memset(buff, 0, 1024);
	sprintf(buff, "{\"H_Freq\":%d,\"V_Freq\":%d,\"V_Total\":%d}", 
	Analog.uiH_Freq,Analog.uiV_Freq, Analog.uiV_Total);
	websWrite(wp, "[");
	websWrite(wp, buff);
	websWrite(wp, "]");
	websDone(wp);
	return 0;
}




static void WebAdj_CleanModes(Webs* wp)
{   

	int iRet;
	iRet = mvMethodVidSig_Vdisp_ClearAnalogModeIndexTable();
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);
	websWrite(wp, T("ret:%d"), iRet);
	websFlush(wp, 0);
	websDone(wp);
}



static void WebAdj_SetAnalogOverlay(Webs* wp)
{
	// if (!bSignalFlag)
	// 	return -1;
	int data;
	int ret;

	data = strtol(websGetVar(wp, T("data"), T("0")), NULL, 10);
	printf("WebAdj_SetAnalogOverlay  data=%d\n",data );
	ret = mvMethodVidSig_Vdisp_SetAnalogOverlay(data);

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);
	websWrite(wp, T("ret:%d"), ret);
	websFlush(wp, 0);
	websDone(wp);

}



static void WebAdj_SetAnalogModeParams(Webs* wp)
{
	// if (!bSignalFlag)
	// 	return -1;
	int data,ret,type;
	int HFreq,VFreq,VTotal,HTotal,HActive,VActive;
	int HStart,VStart,ValidPortFlag,ModeType;
	char *name;
	stSCAL_ANALOG_MODE_PARAMS OverlayMode;
	memset(&OverlayMode, 0, sizeof(stSCAL_ANALOG_MODE_PARAMS));

	type = strtol(websGetVar(wp, T("type"), T("0")), NULL, 10);
	name = websGetVar(wp, T("name"), T("0"));	
	HFreq = strtol(websGetVar(wp, T("HFreq"), T("0")), NULL, 10);
	VFreq = strtol(websGetVar(wp, T("VFreq"), T("0")), NULL, 10);
	VTotal = strtol(websGetVar(wp, T("VTotal"), T("0")), NULL, 10);
	HTotal = strtol(websGetVar(wp, T("HTotal"), T("0")), NULL, 10);
	HActive = strtol(websGetVar(wp, T("HActive"), T("0")), NULL, 10);
	VActive = strtol(websGetVar(wp, T("VActive"), T("0")), NULL, 10);
	HStart = strtol(websGetVar(wp, T("HStart"), T("0")), NULL, 10);
	VStart = strtol(websGetVar(wp, T("VStart"), T("0")), NULL, 10);
	ValidPortFlag = strtol(websGetVar(wp, T("ValidPortFlag"), T("0")), NULL, 10);
	ModeType = strtol(websGetVar(wp, T("ModeType"), T("0")), NULL, 10);

	OverlayMode.uiH_Freq = HFreq;
	OverlayMode.uiV_Freq = VFreq;
	OverlayMode.uiV_Total = VTotal;
	OverlayMode.uiH_Total = HTotal;
	OverlayMode.uiH_Active = HActive;
	OverlayMode.uiV_Active = VActive;
	OverlayMode.uiH_Start = HStart;
	OverlayMode.uiV_Start = VStart;
	OverlayMode.uiValidPortFlag = ValidPortFlag;
	OverlayMode.ModeType = ModeType;

	//OverlayMode.uiModeIndex = ModeIndex;
	strcpy(OverlayMode.PresetName, name);

	//if (!bSignalFlag)
	//{
	if (OverlayMode.uiH_Total < 80 + OverlayMode.uiH_Active)
		OverlayMode.uiH_Total = OverlayMode.uiH_Active + 80;

	if (OverlayMode.ModeType == 4)
		OverlayMode.uiV_Active = (OverlayMode.uiV_Active + 1) / 2;

	printf("[smvwebsvr print] : OverlayMode.uiH_Total = %d, OverlayMode.uiV_Active = %d\n", 
				OverlayMode.uiH_Total, OverlayMode.uiV_Active);
	//}
	
	//save
	if(type == 1)
	{
		ret = mvMethodVidSig_Vdisp_SetAnalogModeParams(&OverlayMode);	
	}
	//debug
	if(type == 2)
	{

		ret = mvMethodVidSig_Vdisp_DebugAnalogModeParams(&OverlayMode);
	}

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);
	websWrite(wp, T("ret:%d"), ret);
	websFlush(wp, 0);
	websDone(wp);
}

static void WebAdj_SetAnalogModeDebug(Webs* wp)
{

	int data, ret;

	
	data = strtol(websGetVar(wp, T("data"), T("0")), NULL, 10);



	mvMethodVidSig_Vdisp_SetAnalogModeDebug(data);

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);
	websWrite(wp, T("ret:%d"), ret);
	websFlush(wp, 0);
	websDone(wp);


}



void formDefineAjust(void)
{
	//websDefineJst(T("GetPresetParams"), WebAdj_GetPresetParams);
	websDefineJst(T("GetAjust"), WebAdj_GetAjust);
	websDefineAction(T("SetAjust"), WebAdj_SetAjust);

	websDefineAction(T("SetPreset"), WebAdj_SetPreset);

	websDefineAction(T("GetCTempVal"), WebAdj_GetCTempVal);
	websDefineAction(T("GetScaleVal"), WebAdj_GetScaleVal);

	websDefineAction("GetDpDebugInfo", WebAdj_GetDpDebugInfo);
	websDefineAction("GetDpDebugInfo1", WebAdj_GetDpDebugInfo1);
	websDefineAction("GetPhase", WebAdj_GetPhase);
	websDefineAction("GetFrequency", WebAdj_GetFrequency);
	websDefineAction("GetHTotal", WebAdj_GetHTotal);
	
	websDefineAction("GetVstart", WebAdj_GetVstart);
	websDefineAction("GetHstart", WebAdj_GetHstart);


	websDefineAction("GetVstartNew", WebAdj_GetVstart_new);
	websDefineAction("GetHstartNew", WebAdj_GetHstart_new);

	websDefineAction("GetResol", WebAdj_GetResol);



	//websDefineAction(T("SetDpDebugInfo"), WebAdj_SetDpDebugInfo);

	websDefineAction("GetAnalogOverlay", WebAdj_GetAnalogOverlay);
	websDefineAction("GetAnalogInputParams", WebAdj_GetAnalogInputParams);

	websDefineAction("SetAnalogOverlay", WebAdj_SetAnalogOverlay);
	websDefineAction("SetAnalogModeParams", WebAdj_SetAnalogModeParams);

	websDefineAction("cleanmodes", WebAdj_CleanModes);

	websDefineAction(T("SetAnalogModeDebug"), WebAdj_SetAnalogModeDebug);

}
/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */