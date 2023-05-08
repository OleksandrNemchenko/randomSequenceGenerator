#ifndef EASYCL_HPP_
#define EASYCL_HPP_

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace ecl{
    const std::size_t MAX_PLATFORMS_COUNT = 64;
    const std::size_t MAX_DEVICES_COUNT = 64;
    const std::size_t MAX_INFO_SIZE = 1024;

    enum ACCESS{READ = CL_MEM_READ_ONLY, WRITE = CL_MEM_WRITE_ONLY, READ_WRITE = CL_MEM_READ_WRITE};
    enum DEVICE{CPU = CL_DEVICE_TYPE_CPU, GPU = CL_DEVICE_TYPE_GPU, ACCEL = CL_DEVICE_TYPE_ACCELERATOR};
    enum FREE{AUTO, MANUALLY};
    enum EXEC {SYNC, ASYNC};

///////////////////////////////////////////////////////////////////////////////
// Error Class Declaration
///////////////////////////////////////////////////////////////////////////////
    class Error{
    protected:
        static int error;
        static std::string getErrorString();

    public:
        static void checkError(const std::string&);
    };

///////////////////////////////////////////////////////////////////////////////
// Platform Class Declaration
///////////////////////////////////////////////////////////////////////////////
    class Platform : public Error{
    private:
        cl_platform_id platform = nullptr;
        std::string name = "";

        std::vector<cl_device_id> cpus;
        std::vector<cl_device_id> gpus;
        std::vector<cl_device_id> accs;

        void initDevices(std::vector<cl_device_id>&, cl_device_type);
        void releaseDevices(std::vector<cl_device_id>&);
    public:
        Platform() = default;
        Platform(cl_platform_id);

        cl_device_id getDevice(std::size_t, DEVICE) const;
        const std::string& getName() const;

        std::string getPlatformInfo(cl_platform_info) const;
        std::string getDeviceInfo(std::size_t, DEVICE, cl_device_info) const;

        friend std::ostream& operator<<(std::ostream&, const Platform&);
        ~Platform();
    };

///////////////////////////////////////////////////////////////////////////////
// System Class Declaration
///////////////////////////////////////////////////////////////////////////////
    class System : public Error{
    private:
        static std::vector<const Platform*> platforms;
        static bool initialized;
    public:
		System() = delete;
        static void init();
        static std::vector<const Platform*> getPlatforms() noexcept { return platforms; }
        static const Platform& getPlatform(std::size_t);
        static void release();
    };

///////////////////////////////////////////////////////////////////////////////
// Program Class Declaration
///////////////////////////////////////////////////////////////////////////////
    class Program : public Error{
    private:
        std::map<cl_context, cl_program> program;
        std::string source;

        std::string getBuildError(cl_context, cl_device_id);

		void copy(const Program&);
		void move(Program&);
    public:
        Program(const char*);
        Program(const std::string&);

        Program(const Program&);
        Program& operator=(const Program&);

        Program(Program&&);
        Program& operator=(Program&&);

        static Program load(const std::string&);

        cl_program getProgram(cl_context) const;
        const std::string& getSource() const;

        Program& operator=(const std::string&);
        Program& operator=(const char*);

        Program& operator+=(const std::string&);
        Program& operator+=(const char*);
        Program& operator+=(const Program&);

        Program operator+(const std::string&);
        Program operator+(const char*);
        Program operator+(const Program&);
        friend std::ostream& operator<<(std::ostream&, const Program&);

        void setSource(const std::string&);

        bool checkProgram(cl_context, cl_device_id);

        void clear();
        ~Program();
    };

///////////////////////////////////////////////////////////////////////////////
// Kernel Class Declaration
///////////////////////////////////////////////////////////////////////////////
    class Kernel: public Error{
    private:
        std::map<cl_program, cl_kernel> kernel; // карта ядер по программам
        std::string name;

		void copy(const Kernel&);
		void move(Kernel&);
    public:
        Kernel(const char*);
        Kernel(const std::string&);

        Kernel(const Kernel&);
        Kernel& operator=(const Kernel&);

        Kernel(Kernel&&);
        Kernel& operator=(Kernel&&);

        Kernel& operator=(const std::string&);
        Kernel& operator=(const char*);

        Kernel& operator+=(const std::string&);
        Kernel& operator+=(const char*);
        Kernel& operator+=(const Kernel&);

        Kernel operator+(const std::string&);
        Kernel operator+(const char*);
        Kernel operator+(const Kernel&);
        friend std::ostream& operator<<(std::ostream&, const Kernel&);

        void setName(const std::string&);
        const std::string& getName() const;

        cl_kernel getKernel(cl_program) const;
        bool checkKernel(cl_program);

        void clear();
        ~Kernel();
    };

///////////////////////////////////////////////////////////////////////////////
// Buffer Class Declaration
///////////////////////////////////////////////////////////////////////////////
    class Buffer : public Error{
    protected:
        std::map<cl_context, cl_mem> buffer; // buffers map by context
        void* ptr = nullptr; // pointer to data
        std::size_t size = 0; // sizeof data
        ACCESS access = READ; // memory access

		void copy(const Buffer&);
		void move(Buffer&);
    public:
        Buffer(void*, std::size_t, ACCESS);

		Buffer(const Buffer&);
		Buffer& operator=(const Buffer&);

		Buffer(Buffer&&);
		Buffer& operator=(Buffer&&);

		cl_mem getBuffer(cl_context) const;
		virtual void* getPtr();
		std::size_t getSize() const;
		ACCESS getAccess() const;

		void setPtr(void*);

		bool checkBuffer(cl_context) const;
		void createBuffer(cl_context);
		void releaseBuffer(cl_context);

		void clear();

		~Buffer();
    };

///////////////////////////////////////////////////////////////////////////////
// var Container Declaration
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class var : public Buffer {
private:
	T value;

	void copy(const var<T>&);
	void move(var<T>&);
public:
	var();
	var(const T&);
	var(ACCESS);
	var(const T&, ACCESS);

	var(const var<T>&);
	var<T>& operator=(const var<T>&);

	var(var<T>&&);
	var<T>& operator=(var<T>&&);

	const T& getValue() const;
	void* getPtr() override;

	void setValue(const T&);

	var<T>& operator++(int);
	var<T>& operator--(int);
	var<T>& operator=(const T&);
	var<T>& operator+=(const T&);
	var<T>& operator-=(const T&);
	var<T>& operator*=(const T&);
	var<T>& operator/=(const T&);
	var<T> operator+(const T&);
	var<T> operator-(const T&);
	var<T> operator*(const T&);
	var<T> operator/(const T&);

	operator T&();
	operator const T&() const;

	void clear();

	~var();
};

///////////////////////////////////////////////////////////////////////////////
// array Class Declaration
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class array : public Buffer {
private:
	T* arr = nullptr;
	std::size_t arr_size = 0;
	FREE manage = MANUALLY;

	void copy(const array<T>&);
	void move(array<T>&);
public:
	array();
	explicit array(std::size_t, ACCESS access = READ_WRITE);
	array(const T*, std::size_t, FREE manage = MANUALLY);
	array(T*, std::size_t, ACCESS, FREE manage = MANUALLY);

	array(const array<T>&);
	array<T>& operator=(const array<T>&);

	array(array<T>&&);
	array<T>& operator=(array<T>&&);

	T* getArray();
	const T* getConstArray() const;
	void* getPtr() override;
	std::size_t getArraySize() const;

	T& operator[](std::size_t);
	operator T*();
	operator const T*() const;

	void clear();
	~array();
};

///////////////////////////////////////////////////////////////////////////////
// Frame Struct Declaration
///////////////////////////////////////////////////////////////////////////////
    struct Frame{
        Program& prog;
        Kernel& kern;
        std::vector<const Buffer*> args;

		Frame(Program* program, Kernel* kernel, std::vector<const Buffer*>&& arguments):
			prog(*program),
			kern(*kernel),
			args(std::move(arguments))
		{}
    };
///////////////////////////////////////////////////////////////////////////////
// Computer Class Declaration
///////////////////////////////////////////////////////////////////////////////
    class Computer : public Error{
        private:
            cl_device_id device = nullptr;
            std::string name = "";

            cl_context context = nullptr;
            cl_command_queue queue = nullptr;

        public:
			Computer() = delete;
            Computer(std::size_t, const Platform&, DEVICE);

			cl_device_id getDevice() const;
			cl_context getContext() const;
			cl_command_queue getQueue() const;
            const std::string& getName() const;

			void send(Buffer&, EXEC sync = SYNC);
			void receive(Buffer&, EXEC sync = SYNC);
			void release(Buffer&, EXEC sync = SYNC);
			void grab(Buffer&, EXEC sync = SYNC);

            void send(const std::vector<Buffer*>&, EXEC sync = SYNC);
			void receive(const std::vector<Buffer*>&, EXEC sync = SYNC);
			void release(const std::vector<Buffer*>&, EXEC sync = SYNC);
			void grab(const std::vector<Buffer*>&, EXEC sync = SYNC);

            void grid(const Frame&, const std::vector<std::size_t>&, const std::vector<std::size_t>&, EXEC sync = SYNC);
            void grid(const Frame&, const std::vector<std::size_t>&, EXEC sync = SYNC);
            void task(const Frame&, EXEC sync = SYNC);

            void await();

            friend std::ostream& operator<<(std::ostream&, const Computer&);
			friend Computer& operator<<(Computer&, Buffer&);
			friend Computer& operator>>(Computer&, Buffer&);

            ~Computer();
    };
}

///////////////////////////////////////////////////////////////////////////////
// var Container Definition
///////////////////////////////////////////////////////////////////////////////
template<typename T>
void ecl::var<T>::copy(const var<T>& other) {
	clear();

	Buffer::copy(other);
	value = other.value;
	setPtr(&value);
}
template<typename T>
void ecl::var<T>::move(var<T>& other) {
	clear();

	Buffer::move(other);
	value = other.value;
	setPtr(&value);

	other.clear();
}

template<typename T>
ecl::var<T>::var() : Buffer(nullptr, sizeof(T), ACCESS::READ_WRITE) {
	value = T();
}
template<typename T>
ecl::var<T>::var(const T& value) : var(){
	this->value = value;
	setPtr(&this->value);
}
template<typename T>
ecl::var<T>::var(ACCESS access) : Buffer(nullptr, sizeof(T), access) {
	value = T();
}
template<typename T>
ecl::var<T>::var(const T& value, ACCESS access) : var(access) {
	this->value = value;
	setPtr(&this->value);
}

template<typename T>
ecl::var<T>::var(const var<T>& other) : Buffer(nullptr, 0, READ_WRITE) {
	copy(other);
}
template<typename T>
ecl::var<T>& ecl::var<T>::operator=(const var<T>& other) {
	copy(other);

	return *this;
}

template<typename T>
ecl::var<T>::var(var<T>&& other) : Buffer(nullptr, 0, READ_WRITE){
	move(other);
}
template<typename T>
ecl::var<T>& ecl::var<T>::operator=(var<T>&& other) {
	move(other);

	return *this;
}


template<typename T>
const T& ecl::var<T>::getValue() const {
	return value;
}
template<typename T>
void* ecl::var<T>::getPtr() {
	return &value;
}

template<typename T>
void ecl::var<T>::setValue(const T& value) {
	this->value = value;
}

template<typename T>
ecl::var<T>& ecl::var<T>::operator++(int) {
	setValue(value + 1);
	return *this;
}
template<typename T>
ecl::var<T>& ecl::var<T>::operator--(int) {
	setValue(value - 1);
	return *this;
}
template<typename T>
ecl::var<T>& ecl::var<T>::operator=(const T& v) {
	setValue(v);
	return *this;
}
template<typename T>
ecl::var<T>& ecl::var<T>::operator+=(const T& v) {
	setValue(value + v);
	return *this;
}
template<typename T>
ecl::var<T>& ecl::var<T>::operator-=(const T& v) {
	setValue(value - v);
	return *this;
}
template<typename T>
ecl::var<T>& ecl::var<T>::operator*=(const T& v) {
	setValue(value * v);
	return *this;
}
template<typename T>
ecl::var<T>& ecl::var<T>::operator/=(const T& v) {
	setValue(value / v);
	return *this;
}

template<typename T>
ecl::var<T> ecl::var<T>::operator+(const T& v) {
	var<T> result = value + v;
	return result;
}
template<typename T>
ecl::var<T> ecl::var<T>::operator-(const T& v) {
	var<T> result = value - v;
	return result;
}
template<typename T>
ecl::var<T> ecl::var<T>::operator*(const T& v) {
	var<T> result = value * v;
	return result;
}
template<typename T>
ecl::var<T> ecl::var<T>::operator/(const T& v) {
	var<T> result = value / v;
	return result;
}

template<typename T>
ecl::var<T>::operator T&() {
	return value;
}
template<typename T>
ecl::var<T>::operator const T&() const {
	return value;
}

template<typename T>
void ecl::var<T>::clear() {
	Buffer::clear();
	value.~T();
}

template<typename T>
ecl::var<T>::~var() {
	clear();
}

///////////////////////////////////////////////////////////////////////////////
// array Container Definition
///////////////////////////////////////////////////////////////////////////////
template<typename T>
void ecl::array<T>::copy(const array<T>& other) {
	clear();

	Buffer::copy(other);

	arr_size = other.arr_size;
	arr = new T[arr_size];
	std::copy(arr, arr + arr_size, other.arr);

	setPtr(arr);
	manage = AUTO;
}
template<typename T>
void ecl::array<T>::move(array<T>& other) {
	clear();

	Buffer::move(other);
	arr = other.arr;
	arr_size = other.arr_size;
	setPtr(arr);
	manage = other.manage;

	other.clear();
}

template<typename T>
ecl::array<T>::array() : Buffer(nullptr, 0, READ_WRITE) {
	arr = nullptr;
	arr_size = 0;
	manage = MANUALLY;
}
template<typename T>
ecl::array<T>::array(std::size_t size, ACCESS access) : Buffer(nullptr, size * sizeof(T), access) {
	arr = new T[size];
	setPtr(arr);
	arr_size = size;
	manage = AUTO;
}
template<typename T>
ecl::array<T>::array(const T* arr, std::size_t size, FREE manage) : Buffer(nullptr, size * sizeof(T), READ){
	this->arr = const_cast<T*>(arr);
	setPtr(this->arr);
	arr_size = size;
	this->manage = manage;
}
template<typename T>
ecl::array<T>::array(T* arr, std::size_t size, ACCESS access, FREE manage) : Buffer(nullptr, size * sizeof(T), access) {
	this->arr = arr;
	setPtr(this->arr);
	arr_size = size;
	this->manage = manage;
}

template<typename T>
ecl::array<T>::array(const array<T>& other) : Buffer(nullptr, 0, READ_WRITE) {
	copy(other);
}
template<typename T>
ecl::array<T>& ecl::array<T>::operator=(const array<T>& other) {
	copy(other);
	return *this;
}

template<typename T>
ecl::array<T>::array(array<T>&& other) : Buffer(nullptr, 0, READ_WRITE) {
	move(other);
}
template<typename T>
ecl::array<T>& ecl::array<T>::operator=(array<T>&& other) {
	move(other);
	return *this;
}

template<typename T>
T* ecl::array<T>::getArray() {
	return arr;
}
template<typename T>
const T* ecl::array<T>::getConstArray() const {
	return arr;
}
template<typename T>
void* ecl::array<T>::getPtr() {
	return arr;
}
template<typename T>
std::size_t ecl::array<T>::getArraySize() const {
	return arr_size;
}

template<typename T>
T& ecl::array<T>::operator[](std::size_t index) {
	return arr[index];
}
template<typename T>
ecl::array<T>::operator T*() {
	return arr;
}
template<typename T>
ecl::array<T>::operator const T*() const {
	return arr;
}

template<typename T>
void ecl::array<T>::clear() {
	Buffer::clear();
	if (manage == AUTO) delete[] arr;

	arr = nullptr;
	arr_size = 0;
	manage = MANUALLY;
}
template<typename T>
ecl::array<T>::~array() {
	clear();
}

#endif // EASYCL_HPP_
