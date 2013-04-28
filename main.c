/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * main.c - Net dialog sample for connecting to an access point
 *
 * For OE firmwares, this sample must be run under the 3.xx kernel.
 *
 * Copyright (c) 2007 David Perry (Insert_Witty_Name)
 *
 */

#include <pspkernel.h>
#include <pspdisplay.h>
#include <string.h>
#include <math.h>
#include <psputility.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspsdk.h>
#include <psputility.h>
#include <psputility_netmodules.h>
#include <psputility_htmlviewer.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psphttp.h>
#include <pspssl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <sys/unistd.h>
#include "cencode.h"
#include "json.h"

//#if _PSP_FW_VERSION >= 200
PSP_MODULE_INFO("Net Dialog Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU); 
//#else
//PSP_MODULE_INFO("Net Dialog Sample", 0x1000, 1, 1);
//PSP_MAIN_THREAD_ATTR(0);
//#endif


extern uint32_t scanhash_c(int thr_id, const unsigned char *midstate, unsigned char *data,
	        unsigned char *hash, const unsigned char *target,
	        uint32_t max_nonce, unsigned long *hashes_done, uint32_t n, unsigned long stat_ctr);
		  
static int running = 1;

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;
	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
	
	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;
	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);

	if (thid >= 0)
		sceKernelStartThread(thid, 0, 0);

	return thid;
}

/* Graphics stuff, based on cube sample */
static unsigned int __attribute__((aligned(16))) list[262144];
static valid = 0;
/*struct Vertex
{
	unsigned int color;
	float x,y,z;
};

struct Vertex __attribute__((aligned(16))) vertices[12*3] =
{
		{0xff7f0000,-1,-1, 1}, // 0
    	{0xff7f0000,-1, 1, 1}, // 4
    	{0xff7f0000, 1, 1, 1}, // 5

    	{0xff7f0000,-1,-1, 1}, // 0
    	{0xff7f0000, 1, 1, 1}, // 5
    	{0xff7f0000, 1,-1, 1}, // 1

    	{0xff7f0000,-1,-1,-1}, // 3
    	{0xff7f0000, 1,-1,-1}, // 2
    	{0xff7f0000, 1, 1,-1}, // 6

    	{0xff7f0000,-1,-1,-1}, // 3
    	{0xff7f0000, 1, 1,-1}, // 6
    	{0xff7f0000,-1, 1,-1}, // 7

    	{0xff007f00, 1,-1,-1}, // 0
    	{0xff007f00, 1,-1, 1}, // 3
    	{0xff007f00, 1, 1, 1}, // 7

    	{0xff007f00, 1,-1,-1}, // 0
    	{0xff007f00, 1, 1, 1}, // 7
    	{0xff007f00, 1, 1,-1}, // 4

    	{0xff007f00,-1,-1,-1}, // 0
    	{0xff007f00,-1, 1,-1}, // 3
    	{0xff007f00,-1, 1, 1}, // 7

    	{0xff007f00,-1,-1,-1}, // 0
    	{0xff007f00,-1, 1, 1}, // 7
    	{0xff007f00,-1,-1, 1}, // 4

    	{0xff00007f,-1, 1,-1}, // 0
    	{0xff00007f, 1, 1,-1}, // 1
    	{0xff00007f, 1, 1, 1}, // 2

    	{0xff00007f,-1, 1,-1}, // 0
    	{0xff00007f, 1, 1, 1}, // 2
    	{0xff00007f,-1, 1, 1}, // 3

    	{0xff00007f,-1,-1,-1}, // 4
    	{0xff00007f,-1,-1, 1}, // 7
    	{0xff00007f, 1,-1, 1}, // 6

    	{0xff00007f,-1,-1,-1}, // 4
    	{0xff00007f, 1,-1, 1}, // 6
    	{0xff00007f, 1,-1,-1}, // 5
};*/

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4)
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2)

static void setupGu()
{
		sceGuInit();

    	sceGuStart(GU_DIRECT,list);
    	sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_WIDTH);
    	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,(void*)0x88000,BUF_WIDTH);
    	sceGuDepthBuffer((void*)0x110000,BUF_WIDTH);
    	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
    	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
    	sceGuDepthRange(0xc350,0x2710);
    	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
    	sceGuEnable(GU_SCISSOR_TEST);
    	sceGuDepthFunc(GU_GEQUAL);
    	sceGuEnable(GU_DEPTH_TEST);
    	sceGuFrontFace(GU_CW);
    	sceGuShadeModel(GU_SMOOTH);
    	sceGuEnable(GU_CULL_FACE);
    	sceGuEnable(GU_CLIP_PLANES);
    	sceGuFinish();
    	sceGuSync(0,0);

    	sceDisplayWaitVblankStart();
    	sceGuDisplay(GU_TRUE);
}

static void drawStuff(void)
{
	static int val = 0;

	sceGuStart(GU_DIRECT, list);

	sceGuClearColor(0xff554433);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

	//sceGumMatrixMode(GU_PROJECTION);
	//sceGumLoadIdentity();
	//sceGumPerspective(75.0f,16.0f/9.0f,0.5f,1000.0f);
	   
	//sceGumMatrixMode(GU_VIEW);
	//sceGumLoadIdentity();
	
	//sceGumMatrixMode(GU_MODEL);
	//sceGumLoadIdentity();
	    
	//ScePspFVector3 pos = { 0, 0, -5.0f };
	//ScePspFVector3 rot = { val * 0.79f * (M_PI/180.0f), val * 0.98f * (M_PI/180.0f), val * 1.32f * (M_PI/180.0f) };
	//sceGumTranslate(&pos);
	//sceGumRotateXYZ(&rot);

	//sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 12*3, 0, vertices);

	sceGuFinish();
	sceGuSync(0,0);
	
	//val++;
}

int netDialog()
{
	int done = 0;

   	pspUtilityNetconfData data;

	memset(&data, 0, sizeof(data));
	data.base.size = sizeof(data);
	data.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	data.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
	data.base.graphicsThread = 17;
	data.base.accessThread = 19;
	data.base.fontThread = 18;
	data.base.soundThread = 16;
	data.action = PSP_NETCONF_ACTION_CONNECTAP;
	
	struct pspUtilityNetconfAdhoc adhocparam;
	memset(&adhocparam, 0, sizeof(adhocparam));
	data.adhocparam = &adhocparam;

	sceUtilityNetconfInitStart(&data);
	
	while(running)
	{
		drawStuff();

		switch(sceUtilityNetconfGetStatus())
		{
			case PSP_UTILITY_DIALOG_NONE:
				break;

			case PSP_UTILITY_DIALOG_VISIBLE:
				sceUtilityNetconfUpdate(1);
				break;

			case PSP_UTILITY_DIALOG_QUIT:
				sceUtilityNetconfShutdownStart();
				break;
				
			case PSP_UTILITY_DIALOG_FINISHED:
				done = 1;
				break;

			default:
				break;
		}

		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
		
		if(done){
			sceUtilityNetconfShutdownStart();
			sceGuTerm();
			//free(data.adhocparam);
			//free(&data);
			break;
		}
	}
	
	return 1;
}

void netInit(void)
{
	sceNetInit(128*1024, 42, 4*1024, 42, 4*1024);
	
	sceNetInetInit();
	
	sceNetResolverInit();
	sceSslInit(0x28000);
	sceHttpInit(0x25800);
	sceHttpsInit(0, 0, 0, 0);
	sceHttpsLoadDefaultCert(0, 0);
	sceHttpLoadSystemCookie();
	sceNetApctlInit(0x8000, 48);
	
	
}

void netTerm(void)
{
	sceNetApctlTerm();
	
	sceNetInetTerm();
	
	sceNetTerm();
}

int user_thread(SceSize args, void *argp)
{
	netInit();
	
	SetupCallbacks();
	
	setupGu();

	netDialog();
	
	netTerm();
	
	return 0;
}

int hex2bin(unsigned char *p, const char *hexstr, size_t len)
{
	while (*hexstr && len) {
		char hex_byte[3];
		unsigned int v;

		if (!hexstr[1]) {
			return 0;
		}

		hex_byte[0] = hexstr[0];
		hex_byte[1] = hexstr[1];
		hex_byte[2] = 0;

		if (sscanf(hex_byte, "%x", &v) != 1) {
			return 0;
		}

		*p = (unsigned char) v;

		p++;
		hexstr += 2;
		len--;
	}

	return (len == 0 && *hexstr == 0) ? 1 : 0;
}
char *bin2hex(const unsigned char *p, size_t len)
{
	int i;
	char *s = malloc((len * 2) + 1);
	if (!s)
		return NULL;

	for (i = 0; i < len; i++)
		sprintf(s + (i * 2), "%02x", (unsigned int) p[i]);

	return s;
}
void sendHttp(int template, char *data, const char *url){
	int connection = sceHttpCreateConnectionWithURL(template, url, 0);
	int id = sceHttpCreateRequest(connection, PSP_HTTP_METHOD_POST, "/", strlen(data));
	sceHttpSendRequest(id, data, strlen(data));
	memset(data,'\0', 1024);
	sceHttpReadData(id, data, 1024);
	sceHttpDeleteRequest(id);
	sceHttpDeleteConnection(connection);
}

struct work {
	unsigned char	data[128];
	unsigned char	hash1[64];
	unsigned char	midstate[32];
	unsigned char	target[32];
	unsigned char	hash[32];
};
int
timeval_subtract (
     struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating Y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     `tv_usec' is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}
static void hashmeter(const struct timeval *diff,
		      unsigned long hashes_done)
{
	float khashes, secs;
	char buf[128];
	khashes = hashes_done / 1000.0;
	secs = (float)diff->tv_sec + ((float)diff->tv_usec / 1000000.0);
	sprintf(buf, "\n%.4f khash/sec\n", khashes / secs);
	pspDebugScreenPrintf(buf);
		//applog(LOG_INFO, "thread %d: %lu hashes, %.2f khash/sec",thr_id, hashes_done,khashes / secs);
}
int miner(struct work *work, int template);
int count = 1;
static u32 matrix(u32 word, unsigned int shift) { 
    u32 result; 
    __asm__ volatile ( 
	   "ROTR      %0,   %1, %2\n"	   
       : "=r"(result) : "r"(word), "r"(shift)); 
    return result; 
}
static void sendJSON(int template, char *data, struct work *work){
	cJSON *json;
	sendHttp(template, data, "http://pit.deepbit.net:8332");
	pspDebugScreenPrintf(data);
	json = cJSON_Parse(data);
	cJSON *resultJSON = cJSON_GetObjectItem(json,"result");
	cJSON *dataJSON;
	cJSON *midJSON;
	cJSON *hashJSON;
	cJSON *targetJSON;
	if(resultJSON != NULL){
		pspDebugScreenPrintf("\nJSON START %x %x\n", 0x01, 0x01);
		dataJSON = cJSON_GetObjectItem(resultJSON,"data");
		if(dataJSON == NULL){
			//Assume bad submit
			char s[256];
			sprintf(s, "{\"version\": 1.1,\"method\": \"getwork\", \"params\": [], \"id\":%d}",count);
			count++;
			cJSON_Delete(json);
			pspDebugScreenPrintf("\nValid hash = ");
			pspDebugScreenPrintf(valid);
			pspDebugScreenPrintf("\n");
			sendJSON(template, s, work);
		}
		midJSON = cJSON_GetObjectItem(resultJSON,"midstate");
		hashJSON = cJSON_GetObjectItem(resultJSON,"hash1");
		targetJSON = cJSON_GetObjectItem(resultJSON,"target");
		pspDebugScreenPrintf("");
		hex2bin(work->data,dataJSON->valuestring,strlen(dataJSON->valuestring));
		pspDebugScreenPrintf("");
		hex2bin(work->midstate,midJSON->valuestring,strlen(midJSON->valuestring));
		pspDebugScreenPrintf("");
		hex2bin(work->hash1,hashJSON->valuestring,strlen(hashJSON->valuestring));
		pspDebugScreenPrintf("");
		hex2bin(work->target,targetJSON->valuestring,strlen(targetJSON->valuestring));
		pspDebugScreenPrintf("");
		memset(work->hash, 0, sizeof(work->hash));
		if(dataJSON != NULL){
			//free(json);
			if(count==0){
				free(data);
			}
			pspDebugScreenPrintf("\n Starting Worker\n");
			cJSON_Delete(json);
			miner(work, template);
		} else{
			pspDebugScreenPrintf("\nERROR DATA\n");
		}
	}else{
		pspDebugScreenPrintf("\nERROR\n");
	}
}


static void submitWork(struct work *work, int template, unsigned long hashes_done)
{
	char *hexstr = NULL;
	char s[512];
	//char d[200];
	
	/* build hex string */
	hexstr = bin2hex(work->data, sizeof(work->data));
	//memcpy(d,hexstr,38);
	//d[39] = '\0';
	/* build JSON-RPC request */
	//From py original_data[:152] + nonce + original_data[160:256]
	
	sprintf(s, "{\"version\": 1.1,\"method\": \"getwork\", \"params\": [\"%s\"], \"id\":%d}",hexstr,count);
	count++;
	char *outs;cJSON *json;
	json=cJSON_Parse(s);
	if (!json) {
		pspDebugScreenPrintf("\nJSON ERROR\n");
		sceKernelDelayThread(5000 * 1000);
		sceKernelExitGame();
	}
	else
	{
		outs=cJSON_Print(json);
		cJSON_Delete(json);
	}
	memset(s,'\0',512);
	memcpy(s, outs, strlen(outs));
	free(outs);
	pspDebugScreenPrintf(s);
	pspDebugScreenPrintf("\nHex Submitted\n");
	
	
	sendJSON(template, s, work);
	/* issue JSON-RPC request */
	//val = json_rpc_call(curl, rpc_url, rpc_userpass, s, false, false);

	//res = json_object_get(val, "result");

	//applog(LOG_INFO, "PROOF OF WORK RESULT: %s",
	//       json_is_true(res) ? "true (yay!!!)" : "false (booooo)");

	//json_decref(val);

	//rc = true;

}
int miner(struct work *work, int template){
	uint32_t max_nonce = 2155020;
	unsigned long hashes_done;
	struct timeval tv_start, tv_end, diff;
	uint64_t max64;
	//int rc;
	hashes_done = 0;
	gettimeofday(&tv_start, NULL);
	pspDebugScreenPrintf("\n Started\n");
	//Need to speed up!!!
	uint32_t n = 0;
	
	while(n != 1){
		n = scanhash_c(0, work->midstate, work->data + 64, work->hash, work->target, max_nonce, &hashes_done, n, hashes_done);
		gettimeofday(&tv_end, NULL);
		timeval_subtract(&diff, &tv_end, &tv_start);
		hashmeter(&diff, hashes_done);
	}
	pspDebugScreenPrintf("\n HAS DONE\n");
	pspDebugScreenClear();
	gettimeofday(&tv_end, NULL);
		timeval_subtract(&diff, &tv_end, &tv_start);

		hashmeter(&diff, hashes_done);
		
	if (diff.tv_usec > 500000){
			diff.tv_sec++;
	}
	if (diff.tv_sec > 0) {
		max64 =
		   ((uint64_t)hashes_done * 5) / diff.tv_sec;
		if (max64 > 0xfffffffaULL)
			max64 = 0xfffffffaULL;
		max_nonce = max64;
	}
	submitWork(work, template, hashes_done);
	return 0;
}


/* main routine */
int main(int argc, char *argv[])
{
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEURI);
	sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEHTTP);
	sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);
	sceUtilityLoadNetModule(PSP_NET_MODULE_SSL);
	
	netInit();
	
	SetupCallbacks();
	
	setupGu();

	netDialog();
	
	pspDebugScreenInit();
	pspDebugScreenClear();
	int template = sceHttpCreateTemplate("PSPMINER", 1, 0);
	
	sceHttpSetConnectTimeOut(template, 10*1000*1000);
	sceHttpEnableKeepAlive(template);
	char *data = malloc(1024*sizeof(char));
	char *out = malloc(1024*sizeof(char));
	base64_encodestate *state = malloc(sizeof(base64_encodestate));
	base64_init_encodestate(state);
	char *auth = "username:pass";
	base64_encode_block(auth, strlen(auth), out, state);
	
	
	char text1[]="{\"version\": 1.1,\"method\": \"getwork\", \"params\": [], \"id\":0}";	
	char *outs;cJSON *json;
	json=cJSON_Parse(text1);
	if (!json) {
		pspDebugScreenPrintf("\nJSON ERROR\n");
	}
	else
	{
		outs=cJSON_Print(json);
		cJSON_Delete(json);
	}
	memcpy(data, outs, strlen(outs));
	free(outs);
	sceHttpAddExtraHeader(template, "Authorization", "Basic !!YOUR BASE64code", 0);
	sceHttpAddExtraHeader(template, "Content-type", "application/json", 0);
	free(out);
	
	
	//struct work *work = malloc(sizeof(struct work));
	struct work work __attribute__((aligned(128)));
	//struct work *work = 0x04002000;// __attribute__((aligned(128)));
	//sceKernelDelayThread(2000 * 1000);
	
	sendJSON(template, data, &work);
	
	pspDebugScreenPrintf("\nDONE\n");
	sceKernelDelayThread(5000 * 1000);
	netTerm();

	
	sceKernelExitGame();

	return 0;
}
