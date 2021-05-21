/*
    WebVideoCfg.c -- video signal handle,  based on goahead 
    Copyright (c) All Rights Reserved. See details at the end of the file.
	2021/5/20
 */

#include    "goahead.h"
#include    "js.h"
#include <signal.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
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

#define CURRENT_VIDEO_OUT 1


typedef enum VIDEOCFG_TYE_tag{
    WEBVID_INPUT_SEL=0, //HTML router1Input
    WEBVID_SWTNEXT, 
    WEBVID_VIDADJUST, 

	WEBVID_OUT_DVI_D, //3 
	WEBVID_OUT_DP,
	WEBVID_OUT_SDI,
	WEBVID_OUT_DVI_I,
	
	WEBVID_VINTYPE,  //7
	WEBVID_DITHER, 
	WEBVID_OUTFMT,
	WEBVID_SMARTLOCK,

	
    WEBVID_SCAN,  //11
    WEBVID_SWTVIDOUT,  
    WEBVID_GRAB, 

    WEBVID_VINSTATUS,  //14

	WEBVID_SDITYPE,

	WEBVID_EXCHGOUTPUT,

    WEBVID_CNT    
}GET_VIDEOCFG_TYE;
static int g_uiVoutIndex;

extern GMutex*  g_scan_mutex;
stVIDSIGIN_STATUS  g_stSigInStat;
extern int g_IsInShowScan;
extern stVIDSIGIN_STATUS AllSigInStat[20];
void websvrEchoScanVinSigInfo(stVIDSIGIN_STATUS *pstSigInStat)
{
	if(pstSigInStat == NULL) return;

	g_mutex_lock(g_scan_mutex);
//	memset(&g_stSigInStat, 0, sizeof(g_stSigInStat));
	memcpy(&g_stSigInStat, pstSigInStat, sizeof(g_stSigInStat));
	printf("[smvwebsvr print2] : %s g_stSigInStat.uiSigInId = %d %d\n", __func__, g_stSigInStat.uiSigInId, g_stSigInStat.uiFrameHActive);

	AllSigInStat[g_stSigInStat.uiSigInId].uiSigInId = g_stSigInStat.uiSigInId;
    AllSigInStat[g_stSigInStat.uiSigInId].uiFrameHActive = g_stSigInStat.uiFrameHActive;

    if(g_stSigInStat.bIsInterLace != 1)
    {
        AllSigInStat[g_stSigInStat.uiSigInId].uiFrameVActive = g_stSigInStat.uiFrameVActive;  
    }
    else
    {
       AllSigInStat[g_stSigInStat.uiSigInId].uiFrameVActive = g_stSigInStat.uiFrameVActive*2;       
    }

	g_mutex_unlock(g_scan_mutex);
	
	if(pstSigInStat->uiSigInId >= 14)
	{
		sleep(2);
		g_mutex_lock(g_scan_mutex);
		memset(&g_stSigInStat, 0, sizeof(g_stSigInStat));
		g_IsInShowScan = 0;
		g_mutex_unlock(g_scan_mutex);		
	}

	if(g_AbortFlag == 1)
	{
		g_mutex_lock(g_scan_mutex);
		memset(&g_stSigInStat, 0, sizeof(g_stSigInStat));
        g_mutex_unlock(g_scan_mutex);	
	
	}
		
	return;
}

static int WebVid_GetVideoCfg(int eid, Webs *wp, int argc, char **argv)
{
    int iRet,type,data;
    char* pType, pData;
    char pOut[16];
    int uiVinIndex;
	int uiVal;
	int uiLen;
	
	stVIDSIGIN_STATUS arrstVidSigInStat[eVIDSIGIN_Cnt -1];
	
	char* ptr;
	
	uiLen =0;
	if(wp->query !=NULL)uiLen =strlen(wp->query);
	
	if(uiLen>0)
	{
		ptr=strstr(wp->query,"vidoutid=");
		ptr +=strlen("vidoutid=");	
		g_uiVoutIndex = strtol(ptr, NULL, 10);		
	}

	
    if (ejArgs(argc, argv, T("%s"), &pType) < 1) {
        websError(wp, 400, T("GetSlider Insufficient args\n"));
        printf("WebVid_GetVideoCfg : %s,get ejArgs failed\n",__FUNCTION__);
        return -1;
    } 
	

	if(g_uiVoutIndex <eVIDSIGOUT_G1_REF || g_uiVoutIndex>eVIDSIGOUT_G2_LIV)
	{
		printf("WebVid_GetVideoCfg : Error! video-out index:%d\n",g_uiVoutIndex);
		return -1;
	}
	
	type = strtol(pType, NULL, 10);
	    
    switch(type){
        case WEBVID_INPUT_SEL: 
			mvMethodDisp_VIDROUT_GetRouter(g_uiVoutIndex,&uiVinIndex);

			if(g_uiVoutIndex==eVIDSIGOUT_G2_LIV) uiVinIndex -=eVIDSIGIN_DP3;
			else uiVinIndex --;
			websWrite(wp, "%d",uiVinIndex);
		break;

        case WEBVID_OUT_DVI_D: 
			mvMethodVidPort_Vout_GetOnOff(g_uiVoutIndex,eDEVPORTOUT_DVI_A,&uiVal);

			uiVal = uiVal <=0 ? 0:1;
			websWrite(wp, "%d",uiVal);
		break;

        case WEBVID_OUT_DP: 
			mvMethodVidPort_Vout_GetOnOff(g_uiVoutIndex,eDEVPORTOUT_DP_C,&uiVal);

			uiVal = uiVal <=0 ? 0:1;
			websWrite(wp, "%d",uiVal);
		break;

        case WEBVID_OUT_SDI: 
			mvMethodVidPort_Vout_GetOnOff(g_uiVoutIndex,eDEVPORTOUT_SDI_D,&uiVal);

			uiVal = uiVal <=0 ? 0:1;
			websWrite(wp, "%d",uiVal);
		break;		

        case WEBVID_OUT_DVI_I: 
			mvMethodVidPort_Vout_GetOnOff(g_uiVoutIndex,eDEVPORTOUT_DVII_B,&uiVal);

			uiVal = uiVal <=0 ? 0:1;
			websWrite(wp, "%d",uiVal);
		break;
		
		case WEBVID_VINTYPE: 
			mvMethodDisp_VIDROUT_GetRouter(g_uiVoutIndex,&uiVinIndex);
			iRet= mvMethodVidSig_Vin_GetAnalogType(uiVinIndex, &uiVal); 

			
			if(uiVal<=0)uiVal =0;
			if(uiVal>2)uiVal =2;
			websWrite(wp, "%d",uiVal);
			return uiVal;

			break;

        case WEBVID_DITHER: 
            break;


        case WEBVID_OUTFMT: 
            iRet= mvMethodVidSig_Vout_GetSigFormat(g_uiVoutIndex, &uiVal); 

			
            uiVal = uiVal <=0 ? 0:1;
			websWrite(wp, "%d",uiVal);
            break;


        case WEBVID_SMARTLOCK: 
            iRet= mvMethodVidSig_Vout_GetSmartLock(g_uiVoutIndex, &uiVal); 
			
            uiVal = uiVal <=0 ? 0:1;
			websWrite(wp, "%d",uiVal);
            break;

		case WEBVID_SCAN: 
			break;

		
		case WEBVID_VINSTATUS: 	
			iRet =mvMethodVidSig_Vin_GetStatus(eVIDSIGIN_ALL, arrstVidSigInStat);

			char buff[512];
			char tmpbuff[128];
			memset(buff,0,sizeof(buff));
			int i;

			sprintf(buff, "\'{");
			if(g_uiVoutIndex==eVIDSIGOUT_G1_REF)
			{			
				for(i=eVIDSIGIN_ALL;i<eVIDSIGIN_DP2;i++)
				{
					memset(tmpbuff,0,sizeof(tmpbuff));				
					sprintf(tmpbuff,"\"%d\":%d",i, arrstVidSigInStat[i].bIsValid);
					strcat(buff,tmpbuff);	
					if(i<eVIDSIGIN_DP2-1)strcat(buff,",");
				}				
			}
			else
			{
				for(i=eVIDSIGIN_DP2;i<eVIDSIGIN_Cnt-1;i++)
				{
					memset(tmpbuff,0,sizeof(tmpbuff));
				
					sprintf(tmpbuff,"\"%d\":%d",i-eVIDSIGIN_DP2, arrstVidSigInStat[i].bIsValid);//arrstVidSigInStat[i].bIsValid
					strcat(buff,tmpbuff);	
					if(i<eVIDSIGIN_Cnt-2)strcat(buff,",");
				}	
			}
			strcat(buff, "}\'");	

			websWrite(wp, buff);
		
			break;		

		case WEBVID_SDITYPE:
			iRet = mvMethodVidPort_Vout_GetSDIType(g_uiVoutIndex, &uiVal);

			uiVal = uiVal <= 0 ? 0 : 1;
			websWrite(wp, "%d", uiVal);
			break;

		case WEBVID_EXCHGOUTPUT:
			
			mvMethodDisp_VIDROUT_GetExchgOutput(&uiVal);
			websWrite(wp, "%d", uiVal);
			break;

		default: break;
    }   
    return 0;
}


static void WebVid_SetVideoCfg(Webs *wp)
{
    int iRet,type,data;
    char *pType,*pData;
    int uiVinIndex;
    int uiVal;
    

	pType =websGetVar(wp, T("type"), T("0"));
    type = strtol(pType, NULL, 10 );
	pData =websGetVar(wp, T("data"), T("0"));	
    data = strtol(pData, NULL, 10 );

    switch(type){
        case WEBVID_INPUT_SEL: 
			
			if(g_uiVoutIndex==eVIDSIGOUT_G1_REF)uiVinIndex =data+1;  // uiVinIndex: 1,2,....,
			else uiVinIndex =data+eVIDSIGIN_DP3;  // uiVinIndex: 16,17
					
		
			iRet= mvMethodDisp_VIDROUT_SetRouter(g_uiVoutIndex,uiVinIndex);

			break;

        case WEBVID_SWTNEXT: 
				
			mvMethodDisp_VIDROUT_SwtNextInput(g_uiVoutIndex, &uiVal);

			
			if(g_uiVoutIndex==eVIDSIGOUT_G1_REF)websRedirect(wp,"/VideoRef.html?vidoutid=1"); // uiVinIndex: 1,2,....,
			else websRedirect(wp,"/VideoLiv.html?vidoutid=2");  // uiVinIndex: 16,17			
			
			break;   

        case WEBVID_VIDADJUST: 
			

			break;
			
        case WEBVID_OUT_DVI_D: 
			
			uiVal = data <=0 ? 0:1;
			mvMethodVidPort_Vout_SetOnOff(g_uiVoutIndex,eDEVPORTOUT_DVI_A,data);
		break;

        case WEBVID_OUT_DP: 
			
			uiVal = data <=0 ? 0:1;	
			mvMethodVidPort_Vout_SetOnOff(g_uiVoutIndex,eDEVPORTOUT_DP_C,data);
		break;

        case WEBVID_OUT_SDI: 
				
			uiVal = data <=0 ? 0:1;
			mvMethodVidPort_Vout_SetOnOff(g_uiVoutIndex,eDEVPORTOUT_SDI_D,data);
		break;		

        case WEBVID_OUT_DVI_I: 
			
			uiVal = data <=0 ? 0:1;	
			mvMethodVidPort_Vout_SetOnOff(g_uiVoutIndex,eDEVPORTOUT_DVII_B,data);
		break;

		case WEBVID_VINTYPE: 			
			if(data<0)uiVal =0;
			else if(data>2)uiVal =2;
			else uiVal =data;
			mvMethodDisp_VIDROUT_GetRouter(g_uiVoutIndex,&uiVinIndex);
			
			iRet= mvMethodVidSig_Vin_SetAnalogType(uiVinIndex, uiVal); 
		break;

        case WEBVID_DITHER: 
            break;

        case WEBVID_OUTFMT: 
			if(data<=0)uiVal =0;
			else uiVal =1;	
		
			
            iRet= mvMethodVidSig_Vout_SetSigFormat(g_uiVoutIndex, uiVal); 
            break;  

        case WEBVID_SMARTLOCK: 
			if(data<=0)uiVal =0;
			else uiVal =1;			
		
            iRet= mvMethodVidSig_Vout_SetSmartLock(g_uiVoutIndex, data); 
            break; 

		case WEBVID_SCAN: 
			//do scan
			//memset(AllSigInStat, 0, sizeof(stVIDSIGIN_STATUS)*20);
			mvMethodVidSig_Vin_ScanPorts(eVIDSIGIN_ALL,uiVal);			
			break;

		case WEBVID_SWTVIDOUT: 
			//do exchange output
	
			iRet= mvMethodDisp_VIDROUT_ExchgOutput();	
			break;

        case WEBVID_GRAB: 
		
			g_mutex_lock(g_scan_mutex);
			memset(&g_stSigInStat,0,sizeof(g_stSigInStat));
			g_mutex_unlock(g_scan_mutex);

			iRet= mvMethodDisp_GRABIMG_DoSingleGrabImage();

			grab_flag = 1;
			grabecho_flag = 0;
			printf("[smvwebsvr print] : before send to web grab_flag = %d\n", grab_flag);

			break;

		case WEBVID_SDITYPE:
		
			if (data <= 0)uiVal = 0;
			else uiVal = 1;

			iRet = mvMethodVidPort_Vout_SetSDIType(g_uiVoutIndex, uiVal);
			break;
		         
		default: break;
    }
    
    
    //web   
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);  
    websWrite(wp, T("ret:%d"), iRet); 
    websFlush(wp, 0);
    websDone(wp);
}


static void WebVid_SetStartScan(Webs *wp)
{
	int iRet;
    int uiVinIndex;
	char strCfgFile[MAX_CFGFILE_NAME_LEN];
	
    mvMethodDisp_VIDROUT_GetRouter(eVIDSIGOUT_G1_REF,&uiVinIndex);

	iRet= mvMethodVidSig_Vin_GetName(uiVinIndex, strCfgFile); //底层库注意：　这个strCfgFile建议确保是jason格式信息，这样上层好解析些 ?
	
	//web   
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);  
    websWrite(wp, T("ret:%s"), strCfgFile); 
    websFlush(wp, 0);
    websDone(wp);
}
static void WebVid_SetStopScan(Webs *wp)
{
	int iRet;
    int uiRout1Vin,uiRout2Vin;
	char strCfgFile[MAX_CFGFILE_NAME_LEN];
	
    iRet = mvMethodVidSig_Vin_ScanPort_Abort();
	
	//web   
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);  
    websWrite(wp, T("ret:%d"), iRet); 
    websFlush(wp, 0);
    websDone(wp);
}

static void WebVid_GetPortIn(Webs *wp)
{
	int uiVinIndex;
	mvMethodDisp_VIDROUT_GetRouter(g_uiVoutIndex,&uiVinIndex);	
	printf("WebVid_GetPortIn : uiVinIndex = %d\n", uiVinIndex);
	
	//web   
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);  
    websWrite(wp, T("%d"), uiVinIndex - 1);
    websFlush(wp, 0);
    websDone(wp);
}

static void WebVid_GetrefPortIn(Webs *wp)
{
	int uiVinIndex;
	mvMethodDisp_VIDROUT_GetRouter(1, &uiVinIndex);	
	printf("WebVid_GetPortIn : uiVinIndex = %d\n", uiVinIndex);
	
	//web   
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);  
    websWrite(wp, T("%d"), uiVinIndex - 1);
    websFlush(wp, 0);
    websDone(wp);
}

static void WebVid_GetExchgOutput(Webs *wp)
{
	int uiVal;
	
	mvMethodDisp_VIDROUT_GetExchgOutput(&uiVal);
	
	//web   
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);  
    websWrite(wp, T("%d"), uiVal);
    websFlush(wp, 0);
    websDone(wp);
}

void formDefineVideoCfg(void)
{      
    int iRet,iValue;
    
    websDefineJst(T("GetVideoCfg"), WebVid_GetVideoCfg);
    websDefineAction(T("SetVideoCfg"), WebVid_SetVideoCfg);
	
	websDefineAction(T("SetStartScan"), WebVid_SetStartScan);
    websDefineAction(T("SetStopScan"), WebVid_SetStopScan);
	websDefineAction(T("GetPortIn"), WebVid_GetPortIn);
	websDefineAction(T("GetrefPortIn"), WebVid_GetrefPortIn);
	websDefineAction(T("GetExchgOutput"), WebVid_GetExchgOutput);

}
/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */