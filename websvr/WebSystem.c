
/*
    WebSystem.c -- system manage , based on goahead

    Copyright (c) All Rights Reserved. See details at the end of the file.
	2021/5/20
 */

#include    "goahead.h"
#include    "js.h"
#include <stdlib.h>
#include <signal.h>
#include <stdlib.h>
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


#define MAX_CFGFILE_NAME_LEN 128
#define CONFIG_FILE_PATH "/opt/svm/tmp"
#define FIRMWARE_FILE_PATH "/opt/svm/tmp"

#define LOG_FILE_DIR "/opt/log"
#define LOG_FILE_FISTNAME "mvlog.log"

#define SYSCFG_FILE_DIR "/opt/cfg"
#define SYSCFG_FILE_FISTNAME "SystemConfig.json"
#define SYSCFG_DOWNLOAD "SystemConfig.svmcfg"

#define ANALOGCFG_FILE_FISTNAME "AnalogModeParams.json"
#define PRESETCFG_FILE_FISTNAME "PresetModeParams.json"

extern char g_DownloadFileName[MAX_CFGFILE_NAME_LEN];
extern char g_UploadFileName[MAX_CFGFILE_NAME_LEN];

typedef enum GET_SYSTEM_TYE_tag{
    WEBSYS_STIME=0, //time
    WEBSYS_WTIME,
    WEBSYS_RESET,
    WEBSYS_SN=3,  //info
    WEBSYS_FIRMVER,
    WEBSYS_MAC100,
    WEBSYS_MAC1000, 
    WEBSYS_LOG=7, //logs
	WEBSYS_REBOOT,
	WEBSYS_DOWNLOG,
	WEBSYS_DOWNCFG,
	WEBSYS_UPLOADCFG,
	WEBSYS_FACTORY,
	WEBSYS_SUBMIT, //13
	WEBSYS_UPDATE,

	WEBSYS_HARDWAREVER,//15
	WEBSYS_2796REFVER,
	WEBSYS_2796LIVEVER,
	WEBSYS_32626VER,
	WEBSYS_MCUVER,
	WEBSYS_FPGAREF,
	WEBSYS_FPGALIVE,

	WEBSYS_UPDATE_ALL,//22
	WEBSYS_UPDATE_IMX7,
	WEBSYS_UPDATE_2797,
	WEBSYS_UPDATE_2796,
	WEBSYS_UPDATE_32626,
	WEBSYS_UPDATE_MCU,
	WEBSYS_UPDATE_FPGAINPUT,
	WEBSYS_UPDATE_FPGAREF,
	WEBSYS_UPDATE_FPGALIVE,
	WEBSYS_UPDATE_FPGAREFSDI,
	WEBSYS_UPDATE_FPGALIVESDI,//32

	WEBSYS_FPGAINPUT,

	WEBSYS_DOWNDPLOG,

	WEBSYS_CIOS_CONFIG,//35

    
    WEBSYS_DownAnalog,//36
    WEBSYS_UploadAnalog,//37
    WEBSYS_DownPreset,//38
    WEBSYS_UploadPreset,//39

	WEBSYS_UPDATE_SUBMCU,

	WEBSYS_SETSTARTPOINT,//41
	WEBSYS_STARTDIAGNOSIS,//42

	WEBSYS_DICOM_COMPATIBILITY,//43

	WEBSYS_INPUT_CONFIG,//44

	WEBSYS_CNT,
}GET_SYSTEM_TYE;



extern GMutex*  g_update_mutex;
DEVVER_UPDATE_INFO g_stDevVerUpdateInfo;
extern char AllUpdateChipInfo[13];
extern char AllUpdateStatInfo[13];

void websvrEchoDevVerUpdateInfo(DEVVER_UPDATE_INFO *pstDevVerUpdateInfo)
{
	if(pstDevVerUpdateInfo==NULL)
		return;

	g_mutex_lock(g_update_mutex);
	memset(&g_stDevVerUpdateInfo,0,sizeof(g_stDevVerUpdateInfo));
	memcpy(&g_stDevVerUpdateInfo,pstDevVerUpdateInfo,sizeof(g_stDevVerUpdateInfo));	
	
	g_mutex_unlock(g_update_mutex);

	return ;
}


static int WebSys_GetSystem(int eid, Webs* wp, int argc, char** argv)
{
	int iRet,type,data;
    char* pType, pData;
    char pOut[32];
    stTIME_INFO stTime;
	char strFile[MAX_CFGFILE_NAME_LEN];
	char strTmp[MAX_CFGFILE_NAME_LEN];
    
    if (ejArgs(argc, argv, T("%s"), &pType) < 1) {
        websError(wp, 400, T("GetSlider Insufficient args\n"));
        printf("WebSys_GetSystem : %s,get ejArgs failed\n",__FUNCTION__);
        return -1;
    } 
	type = strtol(pType, NULL, 10);
	
    switch(type){
        case WEBSYS_STIME:
            iRet =  mvMethodTime_GetCalibTime(&stTime);
            return websWrite(wp, T("%02d/%02d/%4d %02d:%02d:%02d"),stTime.uiMon,stTime.uiMday,
                             stTime.uiYear, stTime.uiHour, stTime.uiMin,stTime.uiSec);
        case WEBSYS_WTIME:
            iRet = mvMethodTime_GetWorkTime(&data);			
            return websWrite(wp, T("%d"),data);    

        case WEBSYS_SN:
            mvMethodDevVer_GetSerialNo(eDEVSN_DEV,pOut);
            return websWrite(wp, T("%s"),pOut); 
			
        case WEBSYS_FIRMVER:
			memset(strTmp,0,sizeof(strTmp));
            mvMethodDevVer_GetVerCode(eDEVVER_LINUX_APP,strTmp);
            return websWrite(wp, T("%s"),strTmp); 
        case WEBSYS_MAC100:
            mvMethodComm_Net_GetMac(eDEVCOMM_NET_ETH0,pOut);
            return websWrite(wp, T("%s"),pOut);
        case WEBSYS_MAC1000:
            mvMethodComm_Net_GetMac(eDEVCOMM_NET_ETH1,pOut);
            return websWrite(wp, T("%s"),pOut);
		case WEBSYS_LOG:
            // api ? 
            return websWrite(wp, T("%d"),iRet);	
		case WEBSYS_HARDWAREVER:
			memset(strTmp,0,sizeof(strTmp));
            mvMethodDevVer_GetVerCode(13,strTmp);
            return websWrite(wp, T("%s"),strTmp); 
		case WEBSYS_2796REFVER:
			memset(strTmp,0,sizeof(strTmp));
            mvMethodDevVer_GetVerCode(2,strTmp);
            return websWrite(wp, T("%s"),strTmp); 
		case WEBSYS_2796LIVEVER:
			memset(strTmp,0,sizeof(strTmp));
            mvMethodDevVer_GetVerCode(3,strTmp);
            return websWrite(wp, T("%s"),strTmp); 
		case WEBSYS_32626VER:
			memset(strTmp,0,sizeof(strTmp));
            mvMethodDevVer_GetVerCode(4,strTmp);
            return websWrite(wp, T("%s"),strTmp);
		case WEBSYS_MCUVER:
			memset(strTmp,0,sizeof(strTmp));
            mvMethodDevVer_GetVerCode(7,strTmp);
            return websWrite(wp, T("%s"),strTmp);
		case WEBSYS_FPGAREF:
			memset(strTmp,0,sizeof(strTmp));
            mvMethodDevVer_GetVerCode(8,strTmp);
            return websWrite(wp, T("%s"),strTmp);
		case WEBSYS_FPGALIVE:
			memset(strTmp,0,sizeof(strTmp));
            mvMethodDevVer_GetVerCode(9,strTmp);
            return websWrite(wp, T("%s"),strTmp);
		case WEBSYS_FPGAINPUT:
			memset(strTmp,0,sizeof(strTmp));
            mvMethodDevVer_GetVerCode(12,strTmp);
            return websWrite(wp, T("%s"),strTmp);
		case WEBSYS_CIOS_CONFIG:
			mvMethodVidPort_Vout_GetCiosConfig(&data);
			return websWrite(wp, T("%d"),data);  
		case WEBSYS_UPDATE_SUBMCU:
			memset(strTmp,0,sizeof(strTmp));
            mvMethodDevVer_GetVerCode(14,strTmp);
            return websWrite(wp, T("%s"),strTmp);
		case WEBSYS_DICOM_COMPATIBILITY:
			mvMethodDisp_GRABIMG_GetManufacturer(&data);
			return websWrite(wp, T("%d"),data);
		case WEBSYS_INPUT_CONFIG:
			mvMethodDisp_VIDROUT_GetInputConfig(&data);
			return websWrite(wp, T("%d"),data);

			
    }
  
    return 0;
}

static void SetCalibra(Webs *wp)
{
    int iRet;
   char *pData,*pEnd;
    stTIME_INFO stTime;
    
    // eg:  "04/20/2019 20:00:01"
	
    memset(&stTime,0,sizeof(stTIME_INFO));
    pData = websGetVar(wp, T("calibra"), T(""));
    if(strlen(pData) < 19){
        iRet = -20;
    }else{
        stTime.uiMon = strtol(pData,&pEnd,10);
        stTime.uiMday = strtol(*(pData+2),&pEnd,10);
        stTime.uiYear = strtol(*(pData+5),&pEnd,10);
        stTime.uiHour = strtol(*(pData+11),&pEnd,10);
        stTime.uiMin = strtol(*(pData+13),&pEnd,10);
        stTime.uiSec = strtol(*(pData+15),&pEnd,10);
                                      
        iRet =  mvMethodTime_SetCalibTime(&stTime);
    }
   
    //web
    websHeader(wp);
    websWrite(wp, T("ret:%d"), iRet);
    websFooter(wp);
    websDone(wp);
}

static stSET_UPDATE_SELECT gstUpdateSelect;

static void WebSys_SetSystem(Webs *wp)
{
    int iRet,type,data;
    char *pType,*pData,*pEnd;
    stTIME_INFO stTime;
    int uiLen;
	char strFile[MAX_CFGFILE_NAME_LEN];
	
	type = strtol(websGetVar(wp, T("type"), T("0")), NULL, 10 );
    pData = websGetVar(wp, T("data"), T("0"));

    switch(type){
		case WEBSYS_STIME:
			
			memset(&stTime,0,sizeof(stTIME_INFO));

			stTime.uiMon = strtol(pData,&pEnd,10);
			pData =pEnd +1;

			stTime.uiMday = strtol(pData,&pEnd,10);
			pData =pEnd +1;

			stTime.uiYear = strtol(pData,&pEnd,10);
			pData =pEnd +1;

			stTime.uiHour = strtol(pData,&pEnd,10);
			pData =pEnd +1;

			stTime.uiMin = strtol(pData,&pEnd,10);
			pData =pEnd +1;
			stTime.uiSec = strtol(pData,&pEnd,10);
				
			iRet =mvMethodTime_SetCalibTime(&stTime);
			break;
			
		case WEBSYS_WTIME:		
			break;

        case WEBSYS_RESET:
			if(strlen(pData) <1)
			{
				iRet = -2;
			}
			else
			{
				type = strtol(pData,NULL,10);
				iRet =  mvMethodDevStat_Reset(type);
			}
			break;

		case WEBSYS_FIRMVER:	

			
			break;

		case WEBSYS_MAC100:		
			break;

		case WEBSYS_MAC1000:		
			break;

		case WEBSYS_LOG:		
			

			
			break;
	
		case WEBSYS_REBOOT:			
			iRet =	mvMethodDevStat_Reset(eMVDEV_SYSTEM_REBOOT);		
			break;
			
		case WEBSYS_DOWNLOG: //upload and download file later
			// LOG_FILE_PATH
			memset(strFile,0,sizeof(strFile));
			
			sprintf(strFile,"%s/%s",LOG_FILE_DIR,LOG_FILE_FISTNAME);
		
			websSetStatus(wp, 200);
			websWriteHeaders(wp, -1, 0);
			websWriteEndHeaders(wp);  
            websWrite(wp, T("%s"),strFile);			
			websFlush(wp, 0);
			websDone(wp);
			return 0;

		case WEBSYS_DOWNCFG:	
			memset(strFile,0,sizeof(strFile));
			sprintf(strFile,"%s/%s",SYSCFG_FILE_DIR,SYSCFG_DOWNLOAD);
			system("cat /opt/cfg/SystemConfig.json | /opt/svm/jq > /opt/cfg/SystemConfig.svmcfg");
			system("sync");

			//do download ...
			websSetStatus(wp, 200);
			websWriteHeaders(wp, -1, 0);
			websWriteEndHeaders(wp);  
            websWrite(wp, T("%s"),strFile);			
			websFlush(wp, 0);
			websDone(wp);
			return 0;

		case WEBSYS_UPLOADCFG:		
			if(strlen(pData) <1){
				iRet = -2;
			}else{				
				memset(strFile,0,sizeof(strFile));
				strcpy(strFile,pData);
				iRet =  mvMethodDevStat_UpdateSysCfg(0,strFile);
			}

			break;
			
		case WEBSYS_FACTORY:	
			iRet =mvMethodDevStat_Restore(0);			
			break;

		case WEBSYS_SUBMIT:		
			break;
						
		case WEBSYS_UPDATE:	
			memset(&g_stDevVerUpdateInfo,0,sizeof(g_stDevVerUpdateInfo));
			memset(AllUpdateChipInfo, 0, 13);
			memset(AllUpdateStatInfo, 0, 13);
			mvMethodDevVer_UpdateVer(0,"upgrade", &gstUpdateSelect, NULL);
			memset(&gstUpdateSelect, 0, sizeof(stSET_UPDATE_SELECT));
			break;	

		case WEBSYS_UPDATE_ALL:
			gstUpdateSelect.update_all = strtol(pData,NULL,10);
			break;

		case WEBSYS_UPDATE_IMX7:
			gstUpdateSelect.update_imx7 = strtol(pData,NULL,10);
			break;
		case WEBSYS_UPDATE_2797:
			gstUpdateSelect.update_2797 = strtol(pData,NULL,10);
			break;
		case WEBSYS_UPDATE_2796:
			gstUpdateSelect.update_2796 = strtol(pData,NULL,10);
			break;
		case WEBSYS_UPDATE_32626:
			gstUpdateSelect.update_32626 = strtol(pData,NULL,10);
			break;
		case WEBSYS_UPDATE_MCU:
			gstUpdateSelect.update_mcu = strtol(pData,NULL,10);
			break;
		case WEBSYS_UPDATE_FPGAINPUT:
			gstUpdateSelect.update_fpga_input = strtol(pData,NULL,10);
			break;
		case WEBSYS_UPDATE_FPGAREF:
			gstUpdateSelect.update_fpga_ref = strtol(pData,NULL,10);
			break;
		case WEBSYS_UPDATE_FPGALIVE:
			gstUpdateSelect.update_fpga_live = strtol(pData,NULL,10);
			break;
		case WEBSYS_UPDATE_FPGAREFSDI:
			gstUpdateSelect.update_fpga_ref_sdi = strtol(pData,NULL,10);
			break;
		case WEBSYS_UPDATE_FPGALIVESDI:
			gstUpdateSelect.update_fpga_live_sdi = strtol(pData,NULL,10);
			break;

		case WEBSYS_DOWNDPLOG:
			memset(strFile,0,sizeof(strFile));
			sprintf(strFile,"%s/%s",LOG_FILE_DIR,"Dp_Debug_Info.log");

			websSetStatus(wp, 200);
			websWriteHeaders(wp, -1, 0);
			websWriteEndHeaders(wp);  
            websWrite(wp, T("%s"),strFile);			
			websFlush(wp, 0);
			websDone(wp);
			return 0;

		case WEBSYS_CIOS_CONFIG:
			
			data = strtol(pData, NULL, 10);
			iRet =mvMethodVidPort_Vout_SetCiosConfig(data);
			break;
		case WEBSYS_DownAnalog:	
			memset(strFile,0,sizeof(strFile));
			iRet = mvMethodDevStat_DownloadAnologCfg();
			sprintf(strFile,"%s/%s",SYSCFG_FILE_DIR, ANALOGCFG_FILE_FISTNAME);

			//do download ...
			websSetStatus(wp, 200);
			websWriteHeaders(wp, -1, 0);
			websWriteEndHeaders(wp);  
            websWrite(wp, T("%s"),strFile);			
			websFlush(wp, 0);
			websDone(wp);

			downloadflag = 1;
			return 0;

		case WEBSYS_UploadAnalog:
			iRet =  mvMethodDevStat_UpdateAnalogModeCfg();
			break;
			
		case WEBSYS_DownPreset:	
			memset(strFile,0,sizeof(strFile));
			iRet = mvMethodDevStat_DownloadPresetCfg();
		
			sprintf(strFile,"%s/%s",SYSCFG_FILE_DIR, PRESETCFG_FILE_FISTNAME);

			//do download ...
			websSetStatus(wp, 200);
			websWriteHeaders(wp, -1, 0);
			websWriteEndHeaders(wp);  
            websWrite(wp, T("%s"),strFile);			
			websFlush(wp, 0);
			websDone(wp);


			downloadflag = 1;
			return 0;

		case WEBSYS_UploadPreset:	
			iRet =  mvMethodDevStat_UpdatePresetModeCfg();
			break;

		case WEBSYS_SETSTARTPOINT:
			iRet = mvMethodDevStat_SetStartPoint();
			break;

		case WEBSYS_STARTDIAGNOSIS:
			iRet = mvMethodDevStat_StartDiagnosis();
			system("cat /opt/cfg/SystemDiagnosis_noform.json | /opt/svm/jq > /opt/cfg/SystemDiagnosis.json");
			system("sync");

			//do download ...
			websSetStatus(wp, 200);
			websWriteHeaders(wp, -1, 0);
			websWriteEndHeaders(wp);  
            websWrite(wp, T("%s"), "/opt/cfg/SystemDiagnosis.json");			
			websFlush(wp, 0);
			websDone(wp);
			break;
		case WEBSYS_DICOM_COMPATIBILITY:
			data = strtol(pData, NULL, 10);
			iRet = mvMethodDisp_GRABIMG_SetManufacturer(data);
			break;
		case WEBSYS_INPUT_CONFIG:
			data = strtol(pData, NULL, 10);
			iRet = mvMethodDisp_VIDROUT_SetInputConfig(data);
			break;
		default :
			return 0;


	}
    
    //web
    websHeader(wp);
    websWrite(wp, T("ret:%d"), iRet);
    websFooter(wp);
    websDone(wp);
}


static int WebSys_GetUpgradeInfo(Webs* wp)
{
	stUPGRADE_CHIP_STATE stUpgrade_ChipStat;
	memset(&stUpgrade_ChipStat, 0, sizeof(stUpgrade_ChipStat));
	mvMethodDevVer_GetUpdateInfo(&stUpgrade_ChipStat);

	char buff[512];
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);

	memset(buff,0,512);
    
	sprintf(buff, "{\"imx7\":\"%d\",\"s2797\":\"%d\",\"s2796\":\"%d\",\"s32626\":\"%d\",\"fpgaswitch\":\"%d\",\"fpgaref\":\"%d\",\"fpgalive\":\"%d\",\"mcu\":\"%d\"}",
	stUpgrade_ChipStat.state_Imx7, 
	stUpgrade_ChipStat.state_2797,
	stUpgrade_ChipStat.state_2796, 
	stUpgrade_ChipStat.state_32626,
	stUpgrade_ChipStat.state_fpgaswitch, 
	stUpgrade_ChipStat.state_fpgaref,
	stUpgrade_ChipStat.state_fpgalive, 
	stUpgrade_ChipStat.state_mcu);

	websWrite(wp,"[");
	websWrite(wp, buff);
	websWrite(wp, "]");
    websDone(wp);
	return 0;
}


void formDefineSystem(void)
{      
    int iRet,iValue;    
    websDefineJst(T("GetSystem"), WebSys_GetSystem);
    websDefineAction(T("SetSystem"), WebSys_SetSystem);
	websDefineAction(T("GetUpgradeInfo"), WebSys_GetUpgradeInfo);
}
/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */