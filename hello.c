#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gst/gst.h>
#include <unistd.h>
#include <string.h>
#include "../libdatachannel/include/rtc/rtc.h"

#define MTU 1000

typedef struct {
	rtcState state;
	rtcGatheringState gatheringState;
	int pc;
	int dc;
	int track;
	bool connected;
} Peer;


Peer *peer;
GstElement *gst_element;
char *g_sdp = NULL;
static GCond g_cond;
static GMutex g_mutex;

//const char PIPE_LINE[] = "v4l2src device=/dev/video0 ! videorate ! video/x-raw,width=800,height=600,framerate=30/1 ! omxh264enc target-bitrate=1000000 control-rate=variable ! video/x-h264,profile=high ! queue max-size-bytes=1000000 ! h264parse ! queue max-size-bytes=1000000 ! rtph264pay config-interval=-1 ssrc=111 pt=102 seqnum-offset=0 timestamp-offset=0 mtu=800 ! appsink max_buffers=2 drop=true  name=pear-sink";

const char PIPE_LINE[] = "v4l2src device=/dev/video0 ! videorate ! video/x-raw,width=640,height=480,framerate=30/1 ! omxh264enc target-bitrate=2000000 control-rate=variable ! video/x-h264,profile=high ! queue max-size-bytes=1000000 ! h264parse ! queue max-size-bytes=1000000 ! rtph264pay config-interval=-1 ssrc=111 pt=102 seqnum-offset=0 timestamp-offset=0 mtu=1000 ! appsink max_buffers=2 drop=true  name=pear-sink";



static GstFlowReturn new_sample(GstElement *sink, void *data) {

  static uint8_t rtp_packet[MTU] = {0};
  int bytes;
  static int temp;
  

  GstSample *sample;
  GstBuffer *buffer;
  GstMapInfo info;

  

  g_signal_emit_by_name (sink, "pull-sample", &sample);
  if(sample) {
    int ret=0;
    buffer = gst_sample_get_buffer(sample);
    gst_buffer_map(buffer, &info, GST_MAP_READ);

    memset(rtp_packet, 0, sizeof(rtp_packet));
    memcpy(rtp_packet, info.data, info.size);
    bytes = info.size;
   
	if (temp==0)
	{
 		int ret=rtcSendMessage(peer->track,rtp_packet,bytes);

		 printf("sent sample result %d\n",ret);
		temp=1;
	}
   
    	ret=rtcSendMessage(peer->track,rtp_packet,bytes);
    	if (ret<0)
	{
		printf("negative value %d",ret);
	}

    gst_sample_unref(sample);
    gst_buffer_unmap(buffer, &info);
    return GST_FLOW_OK;
  }
  return GST_FLOW_ERROR;
}



static void RTC_API descriptionCallback(int pc, const char *sdp, const char *type, void *ptr);
static void RTC_API candidateCallback(int pc, const char *cand, const char *mid, void *ptr);
static void RTC_API stateChangeCallback(int pc, rtcState state, void *ptr);
static void RTC_API gatheringStateCallback(int pc, rtcGatheringState state, void *ptr);
static void RTC_API openCallback(int id, void *ptr);
static void RTC_API closedCallback(int id, void *ptr);
static void RTC_API messageCallback(int id, const char *message, int size, void *ptr);
static void RTC_API trackCallbackFunc(int pc, int tr, void *ptr);
static void RTC_API dcCallback(int id, const char *message, int size, void *user_ptr);
static void deletePeer(Peer *peer);

char *state_print(rtcState state);
char *rtcGatheringState_print(rtcGatheringState state);

int all_space(const char *str);
char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char* read_stdin()
{
	char *line = NULL;
	size_t len = 0;
	size_t read = 0;
	char *buffer= (char *)malloc(sizeof(char)*2);
	buffer[0]='a';
	buffer[0]='\0';
	while ((read = getline(&line, &len, stdin)) != -1 && !all_space(line)) {
		buffer = (char *)realloc(buffer, (strlen(buffer) +1) + strlen(line)+1 );
		//printf("--%s",buffer);
		strcat(buffer, line);
	}
	
	return ltrim(buffer);
}


int main(int argc, char **argv) 
{

	
	//initialize GStreamer
	GstElement *pear_sink;
	gst_init(&argc, &argv);
	gst_element = gst_parse_launch(PIPE_LINE, NULL);
  	pear_sink = gst_bin_get_by_name(GST_BIN(gst_element), "pear-sink");
  	g_signal_connect(pear_sink, "new-sample", G_CALLBACK(new_sample), NULL);
  	g_object_set(pear_sink, "emit-signals", TRUE, NULL);

	gst_element_set_state(gst_element, GST_STATE_PAUSED);
	
	

	rtcInitLogger(RTC_LOG_DEBUG, NULL);
	rtcConfiguration config;
	memset(&config, 0, sizeof(config));
	config.disableAutoNegotiation=TRUE;
	//config.iceServers="stun:stun.l.google.com:19302";

	printf("ice servers -%s",config.iceServers);

	peer = (Peer *)malloc(sizeof(Peer));
	if (!peer) {
		fprintf(stderr, "Error allocating memory for peer\n");
		return -1;
	}
	memset(peer, 0, sizeof(Peer));

	
	// Create peer connection and callbacks
	peer->pc = rtcCreatePeerConnection(&config);

	rtcSetUserPointer(peer->pc, peer);
	rtcSetLocalDescriptionCallback(peer->pc, descriptionCallback);
	rtcSetLocalCandidateCallback(peer->pc, candidateCallback);
	rtcSetStateChangeCallback(peer->pc, stateChangeCallback);
	rtcSetGatheringStateChangeCallback(peer->pc, gatheringStateCallback);
	rtcSetTrackCallback(peer->pc, trackCallbackFunc);
	
	sleep(1);

	printf("\n\npeer connection number %d\n\n",peer->pc);
	
	
	//add track
	
	rtcTrackInit trackInit;
	memset(&trackInit, 0, sizeof(trackInit));
	trackInit.direction=RTC_DIRECTION_SENDONLY;
	trackInit.codec=RTC_CODEC_H264;
	trackInit.payloadType=102;
	trackInit.ssrc=111;
	trackInit.mid="video";
	trackInit.name="bobo-track";
	trackInit.trackId="track1";

	peer->track=rtcAddTrackEx(peer->pc, &trackInit);
	printf("\n\ntrack result %d\n\n",peer->track);
	char desc[1000];
	rtcGetTrackDescription(peer->track,desc,sizeof(desc));
	printf("track description============== \n%s\n\n===========",desc);


	//add datachannel
	peer->dc=rtcCreateDataChannel(peer->pc, "data-channel");
	printf("\n\nData Channel %d created",peer->dc);
	rtcSetMessageCallback(peer->pc, dcCallback);
	
	sleep(1);
	rtcSetLocalDescription(peer->pc,"offer");

	//wait here 
 	g_mutex_lock(&g_mutex);
	g_cond_wait (&g_cond, &g_mutex);
	g_mutex_unlock(&g_mutex);
	
	printf("Enter the answer..\n");
	char* sdp=read_stdin();
	rtcSetRemoteDescription(peer->pc, sdp, "answer");	

	
	//wait here 
 	g_mutex_lock(&g_mutex);
	g_cond_wait (&g_cond, &g_mutex);
	g_mutex_unlock(&g_mutex);
	
	gst_element_set_state(gst_element, GST_STATE_NULL);
  	gst_object_unref(gst_element);
	free(sdp);

}

static void RTC_API trackCallbackFunc(int pc, int tr, void *ptr)
{
	Peer *peer = (Peer *)ptr;
	//peer->track=tr;
	//char desc[10000];
	//rtcGetTrackDescription(tr, desc, sizeof(desc));
	//printf("got a track\n %s\n\n",desc);
	
}

static void RTC_API descriptionCallback(int pc, const char *sdp, const char *type, void *ptr) {
	//printf("\nRTC_API descriptionCallback type %s\n sdp:\n%s\n", type, sdp);
}

static void RTC_API candidateCallback(int pc, const char *cand, const char *mid, void *ptr) {
	//printf("\nRTC_API candidateCallback mid:%s\ncandidate:%s\n",mid, cand);
}

static void RTC_API stateChangeCallback(int pc, rtcState state, void *ptr) {
	Peer *peer = (Peer *)ptr;
	peer->state = state;
	printf("State %s: %s\n", "answerer", state_print(state));
	
	if (state==RTC_CONNECTED)
	{
		printf("/n/nConnected!!/n");
		gst_element_set_state(gst_element, GST_STATE_PLAYING);

	}
}

static void RTC_API gatheringStateCallback(int pc, rtcGatheringState state, void *ptr) {
	Peer *peer = (Peer *)ptr;
	peer->gatheringState = state;
	printf("Gathering state %s: %s\n", "answerer", rtcGatheringState_print(state));
	if (state==RTC_GATHERING_COMPLETE)
	{
		printf("\ngenerating offer\n");
		char type[100];
		rtcGetLocalDescriptionType(peer->pc, type, sizeof(type));
		printf("type - %s\n",type);
		char offer[10000];
		rtcGetLocalDescription(peer->pc, offer, sizeof(offer));
		printf("============\n\n%s\n",offer);
		g_cond_signal(&g_cond);
	}

}

static void RTC_API openCallback(int id, void *ptr) {
	
}

static void RTC_API dcCallback(int id, const char *message, int size, void *ptr)
{
	Peer *peer = (Peer *)ptr;

	printf("messaged received from track %d -%c",peer->dc,message);
}


static void RTC_API closedCallback(int id, void *ptr) {
	Peer *peer = (Peer *)ptr;
	peer->connected = false;
}

static void RTC_API messageCallback(int id, const char *message, int size, void *ptr) {
	if (size < 0) { // negative size indicates a null-terminated string
		printf("Message %s: %s\n", "offerer", message);
	} else {
		printf("Message %s: [binary of size %d]\n", "offerer", size);
	}
}

static void deletePeer(Peer *peer) {
	if (peer) {
		if (peer->dc)
			rtcDeleteDataChannel(peer->dc);
		if (peer->pc)
			rtcDeletePeerConnection(peer->pc);
		free(peer);
	}
}

char *state_print(rtcState state) {
	char *str = NULL;
	switch (state) {
	case RTC_NEW:
		str = "RTC_NEW";
		break;
	case RTC_CONNECTING:
		str = "RTC_CONNECTING";
		break;
	case RTC_CONNECTED:
		str = "RTC_CONNECTED";
		break;
	case RTC_DISCONNECTED:
		str = "RTC_DISCONNECTED";
		break;
	case RTC_FAILED:
		str = "RTC_FAILED";
		break;
	case RTC_CLOSED:
		str = "RTC_CLOSED";
		break;
	default:
		break;
	}

	return str;
}

char *rtcGatheringState_print(rtcGatheringState state) {
	char *str = NULL;
	switch (state) {
	case RTC_GATHERING_NEW:
		str = "RTC_GATHERING_NEW";
		break;
	case RTC_GATHERING_INPROGRESS:
		str = "RTC_GATHERING_INPROGRESS";
		break;
	case RTC_GATHERING_COMPLETE:
		str = "RTC_GATHERING_COMPLETE";
		break;
	default:
		break;
	}

	return str;
}

int all_space(const char *str) {
	while (*str) {
		if (!isspace(*str++)) {
			return 0;
		}
	}

	return 1;
}
