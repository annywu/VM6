/*
    test.c -- Unit test program for GoAhead

    Usage: goahead-test [options] [documents] [endpoints...]
        Options:
        --auth authFile        # User and role configuration
        --home directory       # Change to directory to run
        --log logFile:level    # Log to file file at verbosity level
        --route routeFile      # Route configuration file
        --verbose              # Same as --log stderr:2
        --version              # Output version information

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "goahead.h"
#include    "js.h"

/********************************* Defines ************************************/

static int finished = 0;

#undef ME_GOAHEAD_LISTEN
/*
    These must match TOP.es.set
 */
#if TEST_IPV6
#if ME_COM_SSL
    #define ME_GOAHEAD_LISTEN "http://127.0.0.1:18080, https://127.0.0.1:14443, http://[::1]:18090, https://[::1]:14453"
#else
    #define ME_GOAHEAD_LISTEN "http://127.0.0.1:18080, http://[::1]:18090"
#endif
#else
#if ME_COM_SSL
    #define ME_GOAHEAD_LISTEN "http://127.0.0.1:18080, https://127.0.0.1:14443"
#else
    #define ME_GOAHEAD_LISTEN "http://127.0.0.1:80"
#endif
#endif

#include <sys/ioctl.h>
#include <net/if.h>

#include <locale.h>
#include <glib.h>
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

#define ETHERNET_WIRE0 "eth0"
#define ETHERNET_WIRE1 "eth1"

/*********************************** Locals ***********************************/
/*
 *  Change configuration here
 */

static void initPlatform(void);
static void logHeader(void);
static void usage(void);

static bool testHandler(Webs *wp);
#if ME_GOAHEAD_JAVASCRIPT
static int aspTest(int eid, Webs *wp, int argc, char **argv);
static int bigTest(int eid, Webs *wp, int argc, char **argv);
#endif
static void actionTest(Webs *wp);
static void sessionTest(Webs *wp);
static void showTest(Webs *wp);
#if ME_GOAHEAD_UPLOAD && !ME_ROM
static void upload(Webs *wp);
#endif
#if ME_GOAHEAD_LEGACY
static int legacyTest(Webs *wp, char *prefix, char *dir, int flags);
#endif
#if ME_UNIX_LIKE
static void sigHandler(int signo);
#endif
static void exitProc(void *data, int id);
static void actionDownLoad(Webs *wp, char *path, char *query);

extern  void fileWriteEvent(Webs *wp);
char g_DownloadFileName[128]={0};
char g_UploadFileName[128]={0};
/*********************************** Code *************************************/
extern GMutex*  g_scan_mutex;
extern stVIDSIGIN_STATUS  g_stSigInStat;
stVIDSIGIN_STATUS AllSigInStat[20];
int g_AbortFlag = 0;
int g_IsInShowScan = 0;
void showScan(Webs *wp)
{
    WebsKey *s;
    g_IsInShowScan = 1;
	char buff[1024];
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);

	memset(buff, 0, 1024);

	g_mutex_lock(g_scan_mutex);
    
    if(g_stSigInStat.uiSigInId == 0)
    {
        memset(AllSigInStat, 0, sizeof(stVIDSIGIN_STATUS)*20);
    }


    // AllSigInStat[g_stSigInStat.uiSigInId].uiSigInId = g_stSigInStat.uiSigInId;
    // AllSigInStat[g_stSigInStat.uiSigInId].uiFrameHActive = g_stSigInStat.uiFrameHActive;

    // if(g_stSigInStat.bIsInterLace != 1)
    // {
    //     AllSigInStat[g_stSigInStat.uiSigInId].uiFrameVActive = g_stSigInStat.uiFrameVActive;  
    // }
    // else
    // {
    //    AllSigInStat[g_stSigInStat.uiSigInId].uiFrameVActive = g_stSigInStat.uiFrameVActive*2;       
    // }

    if (!g_AbortFlag)
    {
        printf("[smvwebsvr print3] : %s AllSigInStat[g_stSigInStat.uiSigInId].uiSigInId = %d %d\n", __func__, AllSigInStat[g_stSigInStat.uiSigInId].uiSigInId, AllSigInStat[g_stSigInStat.uiSigInId].uiFrameHActive);
        sprintf(buff, "{\"id\":\"%d\",\"hactive_1\":\"%d\",\"vactive_1\":\"%d\",\"hactive_2\":\"%d\",\"vactive_2\":\"%d\",\"hactive_3\":\"%d\",\"vactive_3\":\"%d\",\"hactive_4\":\"%d\",\"vactive_4\":\"%d\",\"hactive_5\":\"%d\",\"vactive_5\":\"%d\",\"hactive_6\":\"%d\",\"vactive_6\":\"%d\",\"hactive_7\":\"%d\",\"vactive_7\":\"%d\",\"hactive_8\":\"%d\",\"vactive_8\":\"%d\",\"hactive_9\":\"%d\",\"vactive_9\":\"%d\",\"hactive_10\":\"%d\",\"vactive_10\":\"%d\",\"hactive_11\":\"%d\",\"vactive_11\":\"%d\",\"hactive_12\":\"%d\",\"vactive_12\":\"%d\",\"hactive_13\":\"%d\",\"vactive_13\":\"%d\",\"hactive_14\":\"%d\",\"vactive_14\":\"%d\"}",
                AllSigInStat[g_stSigInStat.uiSigInId].uiSigInId,
                AllSigInStat[1].uiFrameHActive, AllSigInStat[1].uiFrameVActive,
                AllSigInStat[2].uiFrameHActive, AllSigInStat[2].uiFrameVActive,
                AllSigInStat[3].uiFrameHActive, AllSigInStat[3].uiFrameVActive,
                AllSigInStat[4].uiFrameHActive, AllSigInStat[4].uiFrameVActive,
                AllSigInStat[5].uiFrameHActive, AllSigInStat[5].uiFrameVActive,
                AllSigInStat[6].uiFrameHActive, AllSigInStat[6].uiFrameVActive,
                AllSigInStat[7].uiFrameHActive, AllSigInStat[7].uiFrameVActive,
                AllSigInStat[8].uiFrameHActive, AllSigInStat[8].uiFrameVActive,
                AllSigInStat[9].uiFrameHActive, AllSigInStat[9].uiFrameVActive,
                AllSigInStat[10].uiFrameHActive, AllSigInStat[10].uiFrameVActive,                                                                                                                  
                AllSigInStat[11].uiFrameHActive, AllSigInStat[11].uiFrameVActive,
                AllSigInStat[12].uiFrameHActive, AllSigInStat[12].uiFrameVActive, 
                AllSigInStat[13].uiFrameHActive, AllSigInStat[13].uiFrameVActive,
                AllSigInStat[14].uiFrameHActive, AllSigInStat[14].uiFrameVActive);

        websWrite(wp,"[");
	    websWrite(wp, buff);
	    websWrite(wp, "]");
        websDone(wp);
        g_mutex_unlock(g_scan_mutex);
    }
    else
    {
        websWrite(wp, T("%d"), -1);
        websDone(wp);
        g_mutex_unlock(g_scan_mutex);
        sleep(3);
        memset(&g_stSigInStat, 0, sizeof(g_stSigInStat));
        g_AbortFlag = 0;
        g_IsInShowScan = 0;
        printf("[smvwebsvr print] : %s end webserver scanning\n", __func__);
    }
    
}

extern DEVVER_UPDATE_INFO g_stDevVerUpdateInfo;
extern GMutex*  g_update_mutex;
char AllUpdateChipInfo[13] = {0};
char AllUpdateStatInfo[13] = {0};

void showUpdate(Webs *wp)
{
    WebsKey     *s;
	char buff[512];
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);

	memset(buff,0,512);
    

	g_mutex_lock(g_update_mutex);

    AllUpdateChipInfo[g_stDevVerUpdateInfo.uiChipId] = g_stDevVerUpdateInfo.uiChipId;
    AllUpdateStatInfo[g_stDevVerUpdateInfo.uiChipId] = g_stDevVerUpdateInfo.uiStatId; 

	sprintf(buff, "{\"uiUpgradeIndex\":\"%d\",\"uiUpgradeNum\":\"%d\",\"uiProcessVal\":\"%d\",\"uiChipId1\":\"%d\",\"uiStatId1\":\"%d\",\"uiChipId2\":\"%d\",\"uiStatId2\":\"%d\",\"uiChipId3\":\"%d\",\"uiStatId3\":\"%d\",\"uiChipId4\":\"%d\",\"uiStatId4\":\"%d\",\"uiChipId5\":\"%d\",\"uiStatId5\":\"%d\",\"uiChipId6\":\"%d\",\"uiStatId6\":\"%d\",\"uiChipId7\":\"%d\",\"uiStatId7\":\"%d\",\"uiChipId8\":\"%d\",\"uiStatId8\":\"%d\",\"uiChipId9\":\"%d\",\"uiStatId9\":\"%d\",\"uiChipId10\":\"%d\",\"uiStatId10\":\"%d\",\"uiChipId11\":\"%d\",\"uiStatId11\":\"%d\",\"uiChipId12\":\"%d\",\"uiStatId12\":\"%d\"}",
	g_stDevVerUpdateInfo.uiUpgradeIndex, g_stDevVerUpdateInfo.uiUpgradeNum,g_stDevVerUpdateInfo.uiProcessVal, 
    AllUpdateChipInfo[1], AllUpdateStatInfo[1],
    AllUpdateChipInfo[2], AllUpdateStatInfo[2],
    AllUpdateChipInfo[3], AllUpdateStatInfo[3],
    AllUpdateChipInfo[4], AllUpdateStatInfo[4],
    AllUpdateChipInfo[5], AllUpdateStatInfo[5],
    AllUpdateChipInfo[6], AllUpdateStatInfo[6],
    AllUpdateChipInfo[7], AllUpdateStatInfo[7],
    AllUpdateChipInfo[8], AllUpdateStatInfo[8],
    AllUpdateChipInfo[9], AllUpdateStatInfo[9],
    AllUpdateChipInfo[10], AllUpdateStatInfo[10],
    AllUpdateChipInfo[11], AllUpdateStatInfo[11],
    AllUpdateChipInfo[12], AllUpdateStatInfo[12]);

	g_mutex_unlock(g_update_mutex);

	websWrite(wp,"[");
	websWrite(wp, buff);
	websWrite(wp, "]");
		
    websDone(wp);
}


//main(goahead, int argcc, char **argvv, char **envp)
GThreadFunc WebStart(gpointer data)
{
    char    *argp, *auth, *home, *documents, *endpoints, *endpoint, *route, *tok, *lspec;
    int     argind, duration;

    route = "route.txt";
    auth = "auth.txt";
    duration = 0;

    int  argc = 5;
    char arg0[] = "webs";
    char arg1[] = "--home";
    char arg2[] ="/etc/goahead";
    char arg3[] ="/opt/svm/www";
    char arg4[] ="80";
    
    char *argv[] ={ arg0 , arg1,arg2,arg3,arg4 } ;
    for (argind = 1; argind < argc; argind++) {
        argp = argv[argind];
        if (*argp != '-') {
            break;

        } else if (smatch(argp, "--auth") || smatch(argp, "-a")) {
            if (argind >= argc) usage();
            auth = argv[++argind];

#if ME_UNIX_LIKE && !MACOSX
        } else if (smatch(argp, "--background") || smatch(argp, "-b")) {
            websSetBackground(1);
#endif

        } else if (smatch(argp, "--debugger") || smatch(argp, "-d") || smatch(argp, "-D")) {
            websSetDebug(1);

        } else if (smatch(argp, "--duration")) {
            if (argind >= argc) usage();
            duration = atoi(argv[++argind]);

        } else if (smatch(argp, "--home")) {
            if (argind >= argc) usage();
            home = argv[++argind];
            if (chdir(home) < 0) {
                error("Cannot change directory to %s", home);
                exit(-1);
            }
        } else if (smatch(argp, "--log") || smatch(argp, "-l")) {
            if (argind >= argc) usage();
            logSetPath(argv[++argind]);

        } else if (smatch(argp, "--verbose") || smatch(argp, "-v")) {
            logSetPath("stdout:2");

        } else if (smatch(argp, "--route") || smatch(argp, "-r")) {
            route = argv[++argind];

        } else if (smatch(argp, "--version") || smatch(argp, "-V")) {
            printf("%s\n", ME_VERSION);
            exit(0);

        } else if (*argp == '-' && isdigit((uchar) argp[1])) {
            lspec = sfmt("stdout:%s", &argp[1]);
            logSetPath(lspec);
            wfree(lspec);

        } else {
            usage();
        }
    }
    documents = ME_GOAHEAD_DOCUMENTS;
    if (argc > argind) {
        documents = argv[argind++];
    }
#if 1
    initPlatform();
#endif
    if (websOpen(documents, route) < 0) {
        error("Cannot initialize server. Exiting.");
        return -1;
    }
    logHeader();
    if (websLoad(auth) < 0) {
        error("Cannot load %s", auth);
        return -1;
    }
    if (argind < argc) {
        while (argind < argc) {
            endpoint = argv[argind++];
            if (websListen(endpoint) < 0) {
                return -1;
            }
        }
    } else {
        endpoints = sclone(ME_GOAHEAD_LISTEN);
        for (endpoint = stok(endpoints, ", \t", &tok); endpoint; endpoint = stok(NULL, ", \t,", &tok)) {
            if (getenv("TRAVIS")) {
                if (strstr(endpoint, "::1") != 0) {
                    /* Travis CI does not support IPv6 */
                    continue;
                }
            }
            if (websListen(endpoint) < 0) {
                return -1;
            }
        }
        wfree(endpoints);
    }
   

    websDefineHandler("test", testHandler, 0, 0, 0);
    websAddRoute("/test", "test", 0);
#if ME_GOAHEAD_LEGACY
    websUrlHandlerDefine("/legacy/", 0, 0, legacyTest, 0);
#endif
#if ME_GOAHEAD_JAVASCRIPT
    websDefineJst("aspTest", aspTest);
    websDefineJst("bigTest", bigTest);
#endif
    websDefineAction("test", actionTest);
    websDefineAction("sessionTest", sessionTest);
#if ME_GOAHEAD_UPLOAD && !ME_ROM
    websDefineAction("upload", upload);
#endif
    websDefineAction("down", actionDownLoad);

	websDefineAction("showScan", showScan);
    websDefineAction("showUpdate", showUpdate);

//	websFormDefine();  //echo user c input

    formDefineNetWork();
	formDefineSettings();
	formDefineSystem();
	formDefineVideoCfg();
	formDefineAjust();
#if ME_UNIX_LIKE && !MACOSX
    /*
        Service events till terminated
    */
    if (websGetBackground()) {
        if (daemon(0, 0) < 0) {
            error("Cannot run as daemon");
            return -1;
        }
    }
#endif
	printf("WebStart duration =%d \n",duration);
    if (duration) {
        printf("Running for %d secs\n", duration);
        websStartEvent(duration * 1000, (WebsEventProc) exitProc, 0);
    }
    websServiceEvents(&finished);
    logmsg(1, "Instructed to exit\n");
    websClose();
    return 0;

}


static void exitProc(void *data, int id)
{
    websStopEvent(id);
    finished = 1;
}


static void logHeader(void)
{
    char    home[ME_GOAHEAD_LIMIT_STRING];

    getcwd(home, sizeof(home));
    logmsg(2, "Configuration for %s", ME_TITLE);
    logmsg(2, "---------------------------------------------");
    logmsg(2, "Version:            %s", ME_VERSION);
    logmsg(2, "BuildType:          %s", ME_DEBUG ? "Debug" : "Release");
    logmsg(2, "CPU:                %s", ME_CPU);
    logmsg(2, "OS:                 %s", ME_OS);
    logmsg(2, "Host:               %s", websGetServer());
    logmsg(2, "Directory:          %s", home);
    logmsg(2, "Documents:          %s", websGetDocuments());
    logmsg(2, "Configure:          %s", ME_CONFIG_CMD);
    logmsg(2, "---------------------------------------------");
}


static void usage(void) {
    fprintf(stderr, "\n%s Usage:\n\n"
        "  %s [options] [documents] [IPaddress][:port]...\n\n"
        "  Options:\n"
        "    --auth authFile        # User and role configuration\n"
#if ME_UNIX_LIKE && !MACOSX
        "    --background           # Run as a Unix daemon\n"
#endif
        "    --debugger             # Run in debug mode\n"
        "    --home directory       # Change to directory to run\n"
        "    --log logFile:level    # Log to file file at verbosity level\n"
        "    --route routeFile      # Route configuration file\n"
        "    --verbose              # Same as --log stderr:2\n"
        "    --version              # Output version information\n\n",
        ME_TITLE, ME_NAME);
    exit(-1);
}


void initPlatform(void)
{
#if ME_UNIX_LIKE
    //signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    signal(SIGKILL, sigHandler);
    signal(SIGINT, SIG_IGN); //edit on 4th Jan. 2021.
    #ifdef SIGPIPE
        signal(SIGPIPE, SIG_IGN);
    #endif
#elif ME_WIN_LIKE
    _fmode=_O_BINARY;
#endif
}


#if ME_UNIX_LIKE
static void sigHandler(int signo)
{
    finished = 1;
}
#endif


/*
    Simple handler and route test
    Note: Accesses to "/" are normally remapped automatically to /index.html
 */
static bool testHandler(Webs *wp)
{
    if (smatch(wp->path, "/")) {
        websRewriteRequest(wp, "/index.html");
        /* Fall through */
    }
    return 0;
}


#if ME_GOAHEAD_JAVASCRIPT
/*
    Parse the form variables: name, address and echo back
 */
static int aspTest(int eid, Webs *wp, int argc, char **argv)
{
    char    *name, *address;
    
    printf("%s\n",__func__); 
    if (jsArgs(argc, argv, "%s %s", &name, &address) < 2) {
        websError(wp, 400, "Insufficient args\n");
        return -1;
    }
    return (int) websWrite(wp, "Name: %s, Address %s", name, address);
}


/*
    Generate a large response
 */
static int bigTest(int eid, Webs *wp, int argc, char **argv)
{
    int     i;

    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);
    websWrite(wp, "<html>\n");
    for (i = 0; i < 800; i++) {
        websWrite(wp, " Line: %05d %s", i, "aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccccddddddd<br/>\r\n");
    }
    websWrite(wp, "</html>\n");
    websDone(wp);
    return 0;
}
#endif



/*
    Implement /action/actionTest. Parse the form variables: name, address and echo back.
 */
static void actionTest(Webs *wp)
{
    cchar   *name, *address;

    name = websGetVar(wp, "name", NULL);
    address = websGetVar(wp, "address", NULL);
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);
    websWrite(wp, "<html><body><h2>name: %s, address: %s</h2></body></html>\n", name, address);
    websFlush(wp, 0);
    websDone(wp);
}


static void sessionTest(Webs *wp)
{
    cchar   *number;

    if (scaselessmatch(wp->method, "POST")) {
        number = websGetVar(wp, "number", 0);
        websSetSessionVar(wp, "number", number);
    } else {
        number = websGetSessionVar(wp, "number", 0);
    }
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);
    websWrite(wp, "<html><body><p>Number %s</p></body></html>\n", number);
    websDone(wp);
}

extern void websvrEchoScanVinSigInfo(Webs *wp);
static void showTest(Webs *wp)
{
    WebsKey     *s;

    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);
    websWrite(wp, "<html><body><pre>\n");
    for (s = hashFirst(wp->vars); s; s = hashNext(wp->vars, s)) {
        websWrite(wp, "%s=%s\n", s->name.value.string, s->content.value.string);
    }
    websWrite(wp, "</pre></body></html>\n");
    websDone(wp);
}


#if ME_GOAHEAD_UPLOAD && !ME_ROM
/*
    Dump the file upload details. Don't actually do anything with the uploaded file.
 */
static void upload(Webs *wp)
{
    WebsKey         *s;
    WebsUpload      *up;
    char            *upfile;

	printf("waring: upload,please mkdir www/tmp \n");
#if 1	
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
//    websWriteHeader(wp, "Content-Type", "text/plain");
    websWriteEndHeaders(wp);
#endif
    if (scaselessmatch(wp->method, "POST")) {

		printf("upload begin\n");
		int cnt;
		cnt =0;
        for (s = hashFirst(wp->files); s; s = hashNext(wp->files, s)) {
            up = s->content.value.symbol;
			
#if 1			
            websWrite(wp, "FILE: %s\r\n", s->name.value.string);
            websWrite(wp, "FILENAME=%s\r\n", up->filename);
            websWrite(wp, "CLIENT=%s\r\n", up->clientFilename);
            websWrite(wp, "TYPE=%s\r\n", up->contentType);
            websWrite(wp, "SIZE=%d\r\n", up->size);
#endif

			cnt++;
			printf("upload cnt=%d f:%s %s size:%d\n",cnt,up->filename,up->clientFilename,up->size);

            upfile = sfmt("%s/tmp/%s", websGetDocuments(), up->clientFilename);
            if (rename(up->filename, upfile) < 0) {
                error("Cannot rename uploaded file: %s to %s, errno %d", up->filename, upfile, errno);
            }
            wfree(upfile);
        }
		websSetStatus(wp, 100);
#if 0		
//        websWrite(wp, "\r\nVARS:\r\n");
 //       for (s = hashFirst(wp->vars); s; s = hashNext(wp->vars, s)) {
 //           websWrite(wp, "%s=%s\r\n", s->name.value.string, s->content.value.string);
 //       }
#endif
    }
	printf("upload ok:%s\n", up->clientFilename);
   websDone(wp);
   websSetStatus(wp, 200);

   websRedirect(wp,"/System.html");
}
#endif


#if ME_GOAHEAD_LEGACY
/*
    Legacy handler with old parameter sequence
 */
static int legacyTest(Webs *wp, char *prefix, char *dir, int flags)
{
    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteHeader(wp, "Content-Type", "text/plain");
    websWriteEndHeaders(wp);
    websWrite(wp, "Hello Legacy World\n");
    websDone(wp);
    return 1;
}

#endif

static void avolfileClose(){
	//wfree(websIndex);
	//websIndex = NULL;
	//wfree(websDocuments);
	//websDocuments = NULL;
}
char * getUrlLastSplit(char * str, char c)
{
    unsigned char i=0,offset;
    while(*(str+i) != NULL){
        if(*(str+i) ==  c){
            offset = i;
        }
        i++;
	}
	return (str+offset+1);
}
static int avolfileHandler(Webs *wp){
	WebsFileInfo    info;
	char *tmp, *date;
	ssize nchars;
	int code;

	char* pathfilename; 
	char* filenameExt; 
	char* filename; 
	char* disposition; 
	
	assert(websValid(wp));
	assert(wp->method);
	assert(wp->filename && wp->filename[0]);

	// http://127.0.0.1:8080/action/down?name=/opt/svm/www/upload/run.sh
	pathfilename = websGetVar(wp, "name", NULL);
	if (pathfilename==NULL)
		return 1;

	//ȡ�ļ�������չ��
	filename =sclone(getUrlLastSplit(sclone(pathfilename),'/'));
	filenameExt =sclone(getUrlLastSplit(sclone(filename),'.'));
	strcpy(g_DownloadFileName,filename); //update api
	if (wp->ext) wfree(wp->ext);
	
	wp->ext=(char*)walloc(1+strlen(filenameExt)+1);
	sprintf(wp->ext,".%s",sclone(filenameExt));
	free(filenameExt);
	filenameExt=NULL;

	if (wp->filename) {
		wfree(wp->filename);
	}
	wp->filename=sclone(pathfilename);

	if (wp->path) {
		wfree(wp->path);
	}
	wp->path=sclone(pathfilename);
    
	//If the file is a directory, redirect using the nominated default page
	if (websPageIsDirectory(wp)) {
		nchars = strlen(wp->path);
		if (wp->path[nchars - 1] == '/' || wp->path[nchars - 1] == '\\') {
			wp->path[--nchars] = '\0';
		}
	    char* websIndex = "testdownload";
		tmp = sfmt("%s/%s", wp->path, websIndex);
		websRedirect(wp, tmp);
		wfree(tmp);
		return 1;
	}
	if (websPageOpen(wp, O_RDONLY | O_BINARY, 0666) < 0) {
		websError(wp, HTTP_CODE_NOT_FOUND, "Cannot open document for: %s", wp->path);
		return 1;
	}
	if (websPageStat(wp, &info) < 0) {
		websError(wp, HTTP_CODE_NOT_FOUND, "Cannot stat page for URL");
		return 1;
	}
	code = 200;
	if (wp->since && info.mtime <= wp->since) {
		code = 304;
	}
	websSetStatus(wp, code);
	websWriteHeaders(wp, info.size, 0);

	//����������ļ�ʱ���ļ��� 
	disposition = (char*)walloc(20+strlen(filename)+1);
	sprintf(disposition,"attachment;filename=%s",sclone(filename));
	websWriteHeader(wp, "Content-Disposition", sclone(disposition));
	free(filename);
	free(disposition);
	filename=NULL;
	disposition=NULL;

	if ((date = websGetDateString(&info)) != NULL) {
		websWriteHeader(wp, "Last-modified", "%s", date);
		wfree(date);
	}
	websWriteEndHeaders(wp);

	//All done if the browser did a HEAD request
	if (smatch(wp->method, "HEAD")) {
		websDone(wp);
		return 1;
	}
	websSetBackgroundWriter(wp, fileWriteEvent);
	return 1;
}


static void actionDownLoad(Webs *wp, char *path, char *query){
	//(*wp).route->handler->close = (*avolfileClose);
	//(*wp).route->handler->service = (*avolfileHandler);
	//(*wp).route->handler->service(wp);
	
	WebsHandlerProc service = (*wp).route->handler->service; 
	(*wp).route->handler->close = (*avolfileClose);
	(*wp).route->handler->service =(*avolfileHandler); 
	(*wp).route->handler->service(wp); 
	(*wp).route->handler->service= service; 
}

