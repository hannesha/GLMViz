#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/pulseaudio.h>

class Pulse{
	public:
		Pulse(const std::string&, const size_t);
		~Pulse();
		static std::string get_default_sink();
		struct usr_data{
			std::string* device;
			pa_mainloop* mainloop;
		};
		void read(std::vector<int16_t>&);
	private:
		static void info_cb(pa_context*, const pa_server_info*, void*);
		static void state_cb(pa_context* , void*);

		pa_simple *stream;
		size_t samples;
};

Pulse::Pulse(const std::string& device, const size_t nsamples){
	samples = nsamples;
	const pa_sample_spec sample_spec = {
		format : PA_SAMPLE_S16LE,
		rate :  44100,
		channels : 1
	};

	pa_buffer_attr buffer_attr = {
		nsamples * 4, //maxlength
		(uint32_t)-1, 
		(uint32_t)-1, 
		(uint32_t)-1, 
		nsamples * 2     //fragsize
	};
	int error;
	stream = pa_simple_new(NULL, "GLViz", PA_STREAM_RECORD, device.c_str(), "GLViz monitor", &sample_spec, NULL, &buffer_attr, &error);
}

Pulse::~Pulse(){
	pa_simple_free(stream);
}

void Pulse::read(std::vector<int16_t>& vbuf){
	int16_t buf[samples];
	pa_simple_read(stream, buf, sizeof(buf),NULL);
	
	vbuf.erase(vbuf.begin(), vbuf.begin() + samples);
	vbuf.insert(vbuf.end(), buf, buf + samples);
}


std::string Pulse::get_default_sink(){	
	std::string device;
	pa_mainloop_api *mainloop_api;
	pa_context *context;
	pa_mainloop *mainloop;

	// Create a mainloop API and connection to the default server
	mainloop = pa_mainloop_new();

	mainloop_api = pa_mainloop_get_api(mainloop);
	context = pa_context_new(mainloop_api, "Viz device list");
	

	pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL);
	
	// init userdata struct
	struct Pulse::usr_data data {&device, mainloop};
	
	// Set callback and hand through the device string and mainloop
	pa_context_set_state_callback(context, Pulse::state_cb, &data);

	int err;
	//starting a mainloop to get default sink
	if (pa_mainloop_run(mainloop, &err) < 0){
		//printf("Could not open pulseaudio mainloop to find default device name: %d", err);
	}

	return device;
}

void Pulse::state_cb(pa_context* context, void* userdata){
	//make sure loop is ready	
	switch (pa_context_get_state(context)){
		case PA_CONTEXT_READY:
		// read server info
		pa_operation_unref(pa_context_get_server_info(
			context, Pulse::info_cb, userdata));
		default:
		break;
	}
}

void Pulse::info_cb(pa_context* context, const pa_server_info* info, void* userdata){
	// recast userdata struct
	struct Pulse::usr_data *data = static_cast<struct Pulse::usr_data*>(userdata);
	//get default sink name
	*(data->device) = std::string(info->default_sink_name);

	//append .monitor sufix
	data->device->append(".monitor");

	//quit mainloop
	pa_mainloop_quit(data->mainloop, 0);
}
