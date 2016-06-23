class Input {
	public:
		Input(const char*);
		~Input();
		bool is_open() const;
		void read_fifo(std::vector<int16_t>&, FFT&);
	private:
		int handle;
};
//extern void compute_fft();

Input::Input(const char* file_name){
	handle = open(file_name, O_RDONLY | O_NONBLOCK);
}

Input::~Input(){
	close(handle);
}

bool Input::is_open() const {
	return handle >= 0;
}

void Input::read_fifo(std::vector<int16_t>& vbuf, FFT& fft){
	int16_t buf[vbuf.size()];
	int64_t data = read(handle, buf, sizeof(buf));
	if(data > 0){
		int64_t samples_read = data/sizeof(int16_t);
                vbuf.erase(vbuf.begin(), vbuf.begin() + samples_read);
                vbuf.insert(vbuf.end(), buf, buf + samples_read);

		fft.calculate(vbuf);
	}
}
