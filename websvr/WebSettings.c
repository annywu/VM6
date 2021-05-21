/*
    WebSetting.c --  Video Manager OSD handle , based on goahead

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


void showBigOsdOnOff(Webs* wp)
{
	WebsKey* s;
	char buff[32];
	int uiVal1 = 0;
	int uiVal2 = 0;
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteEndHeaders(wp);

	mvMethodDisp_OSD_GetOnOff(2,&uiVal1);
	mvMethodDisp_OSD_GetOnOff(3,&uiVal2);

	sprintf(buff, "{\"id1\":\"%d\",\"id2\":\"%d\"}",uiVal1,uiVal2);

	websWrite(wp, "[");
	websWrite(wp, buff);
	websWrite(wp, "]");

	websDone(wp);
}

static void WebSett_SetOsdOnOff(Webs *wp)
{
    int ret;
	int uiVal;
	int uiItem;	
   char *pData;
    
    pData = websGetVar(wp, T("data"), T("0"));  
	
	uiVal =strtol(pData,NULL,10);

	if (uiVal < 10)
	{
    	ret = mvMethodDisp_OSD_SetOnOff(2,uiVal);
	}
	else
	{
		ret = mvMethodDisp_OSD_SetOnOff(3,uiVal - 10);
	}
	
    //web   
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);  
    websWrite(wp, T("{ret:%d}"), ret); 
    websFlush(wp, 0);
    websDone(wp);
    
}


static int WebSett_GetSlider(Webs *wp){
    
    int ret;
	int uiItem;
	int uiVal;
    char *pType;
    
	uiItem =strtol(websGetVar(wp, T("slider"), T("0")), NULL, 10 );
	if(uiItem==2)
		mvMethodDisp_GRABIMG_GetFrozSpan(&uiVal);
	else
    	ret = mvMethodOsd_GetSpan(uiItem,&uiVal);	
	uiVal = uiVal < 0 ? 0 : uiVal;
	
	if (uiVal > 10)
		printf("[smvwebsvr print] : %s uiItem = %d uiVal = %d\n", __func__, uiItem, uiVal);

	websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);
	websWrite(wp, T("%d"),uiVal);
    websFlush(wp, 0);
    websDone(wp);    
}


static void WebSett_SetSlider(Webs *wp)
{
	int ret;
	int uiVal;
	int uiItem;	
   char *pType,*pData;
    
    pType = websGetVar(wp, T("type"), T("0")); 
    pData = websGetVar(wp, T("data"), T("0"));  

	uiItem =strtol(pType,NULL,10);	


	uiVal =strtol(pData,NULL,10);	

	if(uiItem==2)
		mvMethodDisp_GRABIMG_SetFrozSpan(uiVal);
	else
    	ret = mvMethodOsd_SetSpan(uiItem, uiVal); 
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);  
    websWrite(wp, T("{ret:%d}"), ret); 
    websFlush(wp, 0);
    websDone(wp);
}



static int WebSett_GetSelect(int eid, Webs *wp, int argc, char **argv)
{
	int ret;
	int uiVal;
	int uiItem;
    char *pType;
    
    if (jsArgs(argc, argv, T("%s"), &pType) < 1) {
        websError(wp, 400, T("GetSelect Insufficient args\n"));
        printf("WebSett_GetSelect : %s,get jsArgs failed\n",__func__);
        return -1;
    } 
	uiItem =strtol(pType,NULL,10);
	if (uiItem == 2)
		mvMethodDisp_GRABIMG_GetGrabIndicatSel(&uiVal);
	else if (uiItem == 5)
	{
		mvMethodDisp_OSD_GetOnOff(2,&uiVal);

	}
	else if (uiItem == 6)
	{
		mvMethodDisp_OSD_GetOnOff(3, &uiVal);

	}
	else
	{
    	ret = mvMethodOsd_GetSingleSel(uiItem,&uiVal);
		if (uiItem == 0)
			uiVal = uiVal - 2;

	}
	
	uiVal = uiVal <=0 ? 0:1;	
	uiVal = uiVal >=1 ? 1:0;

	websWrite(wp, T("%d"),uiVal);
    return uiVal;
}

static void WebSett_SetSelect(Webs *wp)
{
    int ret;
	int uiVal;
	int uiItem;	
	char *pType,*pData;
    
    pType = websGetVar(wp, T("type"), T("0")); 
    pData = websGetVar(wp, T("data"), T("0"));  

	uiItem =strtol(pType,NULL,10);	


	uiVal =strtol(pData,NULL,10);	

	if(uiItem==2)
		mvMethodDisp_GRABIMG_SetGrabIndicatSel(uiVal);
	else
    	ret = mvMethodOsd_SetSingleSel(uiItem, uiVal); 
		 
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);  
    websWrite(wp, T("{ret:%d}"), ret); 
    websFlush(wp, 0);
    websDone(wp);
}

void formDefineSettings(void)
{      
	websDefineAction(T("SetOsdOnOff"), WebSett_SetOsdOnOff); 
	websDefineAction(T("SetRemoteMouseOnOff"), WebSett_SetSelect); 
    websDefineAction(T("GetSlider"), WebSett_GetSlider);		
    websDefineAction(T("SetSlider"), WebSett_SetSlider);
    websDefineAction(T("SetSelect"), WebSett_SetSelect);
    websDefineJst(T("GetSelect"), WebSett_GetSelect);

	websDefineAction("showBigOsdOnOff", showBigOsdOnOff);
}
/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */