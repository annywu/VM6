/*
    WebNetwork.c -- show network information , based on goahead

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


typedef enum GET_NETWORK_TYE_tag{
   WEBNET_IP1=0, //ether
   WEBNET_MASK1,
   WEBNET_IP2,
   WEBNET_MASK2,
   WEBNET_PORT1,
   WEBNET_CMDIF,
   WEBNET_STORIP =6,//dicom
   WEBNET_STORPORT,
   WEBNET_STORAE,
   WEBNET_STORLAE,
   WEBNET_RISIP =10, //ris
   WEBNET_RISPORT,
   WEBNET_RISAE,
   WEBNET_RISLAE,
   WEBNET_CNT
}GET_NETWORK_TYE;
static int GetNetWork(int eid, Webs *wp, int argc, char **argv)
{
    int iRet,eth,type;
    char *pType;
    stDEVCOMM_NET stDevCommNet;
    stDICOMM_SVR_INFO stDicommSvrInfo;
    
    if (jsArgs(argc, argv, "%s", &pType) < 1) {
        websError(wp, 400, T("GetSlider Insufficient args\n"));
        printf("GetNetWork : %s,get jsArgs failed\n",__FUNCTION__);
        return -1;
    } 

	memset(&stDevCommNet,0,sizeof(stDEVCOMM_NET));
	memset(&stDicommSvrInfo,0,sizeof(stDICOMM_SVR_INFO));
    type = strtol(pType, NULL, 10);
		
    switch(type){
        case WEBNET_IP1: 
            iRet = mvMethodComm_Net_GetNet(eDEVCOMM_NET_ETH0,&stDevCommNet);  
            return websWrite(wp, T("%s"),stDevCommNet.strIpAddr);			
		case WEBNET_IP2: 
            iRet = mvMethodComm_Net_GetNet(eDEVCOMM_NET_ETH1,&stDevCommNet);  
            return websWrite(wp, T("%s"),stDevCommNet.strIpAddr);

        case WEBNET_MASK1: 
            iRet = mvMethodComm_Net_GetNet(eDEVCOMM_NET_ETH0,&stDevCommNet);			
            return websWrite(wp, T("%s"),stDevCommNet.strMask); 			
		case WEBNET_MASK2:
            iRet = mvMethodComm_Net_GetNet(eDEVCOMM_NET_ETH1,&stDevCommNet);			
            return websWrite(wp, T("%s"),stDevCommNet.strMask);      
        case WEBNET_PORT1:
            mvMethodComm_Net_GetCmdNetPort(&iRet);
            return websWrite(wp, T("%d"),iRet); 
        case WEBNET_CMDIF:
            mvMethodComm_Net_GetCmdNetSelect(&iRet);			
			iRet -=1;  //iNet index : 1,2 .
            return websWrite(wp, T("%d"),iRet);

		//dicom store
		case WEBNET_STORIP:
             mvMethodDisp_GRABIMG_GetDicomStorSvr(&stDicommSvrInfo); 			 
             return websWrite(wp, T("%s"),stDicommSvrInfo.strPeerHostName); 
		case WEBNET_STORPORT:
			 mvMethodDisp_GRABIMG_GetDicomStorSvr(&stDicommSvrInfo); 
             return websWrite(wp, T("%d"),stDicommSvrInfo.uiPeerPort); 
		case WEBNET_STORAE:
		 	mvMethodDisp_GRABIMG_GetDicomStorSvr(&stDicommSvrInfo);			 
		 	return websWrite(wp, T("%s"),stDicommSvrInfo.strPeerAETitle); 
	 	case WEBNET_STORLAE:
		 	mvMethodDisp_GRABIMG_GetDicomStorSvr(&stDicommSvrInfo); 
		 	return websWrite(wp, T("%s"),stDicommSvrInfo.strAETitle); 

		//dicom ris
		case WEBNET_RISIP:
             mvMethodDisp_GRABIMG_GetDicomRisSvr(&stDicommSvrInfo); 			 
             return websWrite(wp, T("%s"),stDicommSvrInfo.strPeerHostName); 
		case WEBNET_RISPORT:
			 mvMethodDisp_GRABIMG_GetDicomRisSvr(&stDicommSvrInfo); 
             return websWrite(wp, T("%d"),stDicommSvrInfo.uiPeerPort); 
		case WEBNET_RISAE:
		 	mvMethodDisp_GRABIMG_GetDicomRisSvr(&stDicommSvrInfo);			 
		 	return websWrite(wp, T("%s"),stDicommSvrInfo.strPeerAETitle); 
	 	case WEBNET_RISLAE:
		 	mvMethodDisp_GRABIMG_GetDicomRisSvr(&stDicommSvrInfo); 
		 	return websWrite(wp, T("%s"),stDicommSvrInfo.strAETitle); 
			
    }
    
    return 0;
}
#if 1 
static void SetNetWork(Webs *wp)
{
    int iRet;
    char *pType,*pData;
    stDEVCOMM_NET stDevCommNet;
    stDICOMM_SVR_INFO stDicommSvrInfo;
    int uiVal;

    //ether1
    memset(&stDevCommNet,0,sizeof(stDEVCOMM_NET));
    strncpy(stDevCommNet.strIpAddr, websGetVar(wp,  T("IP1"), T("11.88.0.2")), MAX_IPADDR_SIZE);
    strncpy(stDevCommNet.strMask, websGetVar(wp, "MASK1", "255.255.255.0"), MAX_IPADDR_SIZE);
    stDevCommNet.uiNetIndex =eDEVCOMM_NET_ETH0;	
    iRet = mvMethodComm_Net_SetNet(eDEVCOMM_NET_ETH0,&stDevCommNet);   
    
    //ether2 port and cmdInterface
    memset(&stDevCommNet,0,sizeof(stDEVCOMM_NET));
    strncpy(stDevCommNet.strIpAddr, websGetVar(wp, T("IP2"), T("10.1.1.111")), MAX_IPADDR_SIZE);
    strncpy(stDevCommNet.strMask, websGetVar(wp, T("MASK3"), T("255.255.255.0")), MAX_IPADDR_SIZE);
    stDevCommNet.uiNetIndex =eDEVCOMM_NET_ETH1;
    iRet = mvMethodComm_Net_SetNet(eDEVCOMM_NET_ETH1,&stDevCommNet);   

	uiVal =strtol( websGetVar(wp, T("PORT1"), T("33800")), NULL, 10);
    mvMethodComm_Net_SetCmdNetPort(uiVal);


    //dicom
    memset(&stDicommSvrInfo,0,sizeof(stDicommSvrInfo));
    strncpy(stDicommSvrInfo.strPeerHostName, websGetVar(wp, T("IP3"), T("11.88.0.10")), DICOMM_SVR_INFO_LEN);
    stDicommSvrInfo.uiPeerPort = strtol(websGetVar(wp, T("PORT2"), T("104")),NULL, 10);
    strncpy(stDicommSvrInfo.strPeerAETitle, websGetVar(wp, T("AE"), T("UROXXX_ST_SCP")), DICOMM_SVR_INFO_LEN);
    strncpy(stDicommSvrInfo.strAETitle, websGetVar(wp, T("LAE"), T("VIDEOMANAGER_AET")), DICOMM_SVR_INFO_LEN);
	
    mvMethodDisp_GRABIMG_SetDicomStorSvr(&stDicommSvrInfo); 
    
    
    //ris > copy dicom
    memset(&stDicommSvrInfo,0,sizeof(stDicommSvrInfo));
    strncpy(stDicommSvrInfo.strPeerHostName, websGetVar(wp, T("IP4"), T("11.88.0.10")), DICOMM_SVR_INFO_LEN);
    stDicommSvrInfo.uiPeerPort = strtol(websGetVar(wp, T("PORT3"), T("105")),NULL, 10);
    strncpy(stDicommSvrInfo.strPeerAETitle, websGetVar(wp, T("AERIS"), T("UROXXX_WK_SCP")), DICOMM_SVR_INFO_LEN);
    strncpy(stDicommSvrInfo.strAETitle, websGetVar(wp, T("LAERIS"), T("VIDEOMANAGER_AET")), DICOMM_SVR_INFO_LEN);

    mvMethodDisp_GRABIMG_SetDicomRisSvr(&stDicommSvrInfo); 
    
    //web   
	websRedirect(wp,"/Network.html");

}
#endif


static void SetCmdIFSel(Webs *wp)
{
    int iRet,type,data;
    char *pType,*pData;

    stDEVCOMM_NET stDevCommNet;
    stDICOMM_SVR_INFO stDicommSvrInfo;

	pType =websGetVar(wp, T("type"), T("0"));
    type = strtol(pType, NULL, 10 );

	switch(type)
    {
        case WEBNET_CMDIF: 
            pData =websGetVar(wp, T("data"), T("0"));	
            data = strtol(pData, NULL, 10 );

			if(data != 0 && data != 1)
                data =0;			
			iRet = mvMethodComm_Net_SetCmdNetSelect(data+1);//iNet index : 1,2 .
			break;
        case WEBNET_IP1:
            stDevCommNet.uiNetIndex =eDEVCOMM_NET_ETH0;	
            memset(&stDevCommNet,0,sizeof(stDEVCOMM_NET));
            strncpy(stDevCommNet.strIpAddr, websGetVar(wp,  T("ip"), T("11.88.0.2")), MAX_IPADDR_SIZE);
            strncpy(stDevCommNet.strMask, websGetVar(wp, "netmask", "255.255.255.0"), MAX_IPADDR_SIZE);
            iRet = mvMethodComm_Net_SetNet(eDEVCOMM_NET_ETH0,&stDevCommNet);   

            break;
        case WEBNET_IP2:
            stDevCommNet.uiNetIndex =eDEVCOMM_NET_ETH1;	
            memset(&stDevCommNet,0,sizeof(stDEVCOMM_NET));
            strncpy(stDevCommNet.strIpAddr, websGetVar(wp,  T("ip1g"), T("10.1.1.111")), MAX_IPADDR_SIZE);
            strncpy(stDevCommNet.strMask, websGetVar(wp, "netmask1gb", "255.255.255.0"), MAX_IPADDR_SIZE);
            iRet = mvMethodComm_Net_SetNet(eDEVCOMM_NET_ETH1,&stDevCommNet);   

            break;

        case WEBNET_PORT1:
            pData =websGetVar(wp, T("portnumber"), T("33800"));	
            data = strtol(pData, NULL, 10 );
            iRet = mvMethodComm_Net_SetCmdNetPort(data);
            break;
        case WEBNET_STORIP:
            memset(&stDicommSvrInfo,0,sizeof(stDicommSvrInfo));
            strncpy(stDicommSvrInfo.strPeerHostName, websGetVar(wp, T("dicomip"), T("11.88.0.10")), DICOMM_SVR_INFO_LEN);
            stDicommSvrInfo.uiPeerPort = strtol(websGetVar(wp, T("dicomportnumber"), T("104")),NULL, 10);
            strncpy(stDicommSvrInfo.strPeerAETitle, websGetVar(wp, T("remotetitle"), T("UROXXX_ST_SCP")), DICOMM_SVR_INFO_LEN);
            strncpy(stDicommSvrInfo.strAETitle, websGetVar(wp, T("localtitle"), T("VIDEOMANAGER_AET")), DICOMM_SVR_INFO_LEN);
	
            iRet = mvMethodDisp_GRABIMG_SetDicomStorSvr(&stDicommSvrInfo); 
            break;
        case WEBNET_RISIP:
            memset(&stDicommSvrInfo,0,sizeof(stDicommSvrInfo));
            strncpy(stDicommSvrInfo.strPeerHostName, websGetVar(wp, T("risip"), T("11.88.0.10")), DICOMM_SVR_INFO_LEN);
            stDicommSvrInfo.uiPeerPort = strtol(websGetVar(wp, T("rispn"), T("105")),NULL, 10);
            strncpy(stDicommSvrInfo.strPeerAETitle, websGetVar(wp, T("Risremote"), T("UROXXX_WK_SCP")), DICOMM_SVR_INFO_LEN);
            strncpy(stDicommSvrInfo.strAETitle, websGetVar(wp, T("Rislocal"), T("VIDEOMANAGER_AET")), DICOMM_SVR_INFO_LEN);
	
            iRet = mvMethodDisp_GRABIMG_SetDicomRisSvr(&stDicommSvrInfo); 
    
            break;
		default:
			break;
			
   	}
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);  
    websWrite(wp, T("{ret:%d}"), iRet); 
    websFlush(wp, 0);
    websDone(wp);
	return;
}

void formDefineNetWork(void)
{      
    int iRet,iValue;
    
    websDefineJst("GetNetWork", GetNetWork);
    websDefineAction("SetNetWork", SetNetWork);
    websDefineAction("SetCmdIFSel", SetCmdIFSel);
}
/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */