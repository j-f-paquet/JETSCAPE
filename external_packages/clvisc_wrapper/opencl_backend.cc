#include "include/opencl_backend.h"

#define __CL_ENABLE_EXCEPTIONS

CompileOption::CompileOption(){
    Define("USE_SINGLE_PRECISION");
};

CompileOption::CompileOption(bool use_single_precision, bool optimize) {
    if (use_single_precision) {
        Define("USE_SINGLE_PRECISION");
    }
    if (! optimize) {
        Define("-cl-opt-disable");
    }
}

void CompileOption::Define(std::string definition) {
    opt << "-D " << definition <<" ";
}

template <class ValueType>
void CompileOption::SetValue(std::string key, ValueType value) {
    opt << "-D " << key << "=" << value << " ";
}

OpenclBackend::OpenclBackend(std::string device_type, int device_id) {
    // select device type and device id (if there are multiple cpu/gpus)
    if (device_type == "cpu" || device_type == "CPU") {
        device_type_ = CL_DEVICE_TYPE_CPU;
    } else if (device_type == "gpu" || device_type == "GPU") {
        device_type_ = CL_DEVICE_TYPE_GPU;
    } else {
        device_type_ = CL_DEVICE_TYPE_ALL;
    };
    device_id_ = device_id;
    // create context for the designated device type
    context_ = CreateContext_(device_type_);
    // choose one device if there are many of the same kind
    devices_ = context_.getInfo<CL_CONTEXT_DEVICES>();
    auto num_of_devices = devices_.size();
    if (device_id_ < 0 || device_id_ > num_of_devices-1) {
        for( std::vector<cl::Device>::size_type i=0; i!=num_of_devices; i++){
            std::cerr << "#" << devices_[i].getInfo<CL_DEVICE_NAME>() << '\n';
            std::cerr << "#Max compute units ="
                << devices_[i].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << '\n';
        }
        throw std::out_of_range("device_id out of range");
    } else {
        device_ = devices_[device_id_];
    }
    queue_ = cl::CommandQueue(context_, device_, CL_QUEUE_PROFILING_ENABLE);
}



/** get the kernel excution time in units of seconds */
float OpenclBackend::ExcutionTime(cl::Event & event)
{
    cl_ulong tstart, tend;
    event.getProfilingInfo(CL_PROFILING_COMMAND_START, & tstart);
    event.getProfilingInfo(CL_PROFILING_COMMAND_END, & tend);
    //std::cout<<"#run time="<<(tend - tstart )/1000<<"ms\n";
    return (tend - tstart) * 1.0E-9 ;
}

cl::Context OpenclBackend::CreateContext_(const cl_int & device_type)
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.size() == 0) {
        std::cerr<<"No platform found, install CUDA or AMD SDK first\n";
        exit(-1);
    } else {
        for (int i=0; i < platforms.size(); i++) {
            std::vector<cl::Device> supportDevices;
            platforms.at(i).getDevices(CL_DEVICE_TYPE_ALL, &supportDevices);
            for (int j=0; j < supportDevices.size(); j++) {
                if (supportDevices.at(j).getInfo<CL_DEVICE_TYPE>() == device_type) {
                    //std::cout<<"#Found device "<<device_type<<" on platform "<<i<<std::endl;
                    cl_context_properties properties[] =
                    { CL_CONTEXT_PLATFORM, 
                        (cl_context_properties) (platforms.at(i))(),
                        0 };
                    return cl::Context(device_type, properties);
                }// Found supported device and platform
            }// End for devices
        }// End for platform
        //// if no platform support device_type, exit
        std::cerr<<"no platform support device type"<<device_type<<std::endl;
        exit(-1);
    }
}

cl::Program OpenclBackend::BuildProgram(std::string fname,
                                        const CompileOption & compile_option)
{ //// build programs and print the compile error if there is
    std::ifstream kernelFile(fname.c_str());
    if(!kernelFile.is_open()) std::cerr<<"Open "<<fname << " failed!"<<std::endl;
    std::string sprog(std::istreambuf_iterator<char> (kernelFile),
                      (std::istreambuf_iterator<char> ()));
    cl::Program::Sources prog(1, std::make_pair(sprog.c_str(), sprog.length()));
    auto program = cl::Program(context_, prog);
    //programs.push(program);
    try{
        program.build(devices_, compile_option.opt.str().c_str());
        kernelFile.close();
        return program;
    } catch(cl::Error & err) {
        std::cerr << err.what() << "(" << err.err() << ")\n" \
            << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device_);
    }
}


cl::Buffer OpenclBackend::CreateBuffer(size_t bytes_of_buffer) {
    return cl::Buffer(context_, CL_MEM_READ_WRITE, bytes_of_buffer);
}

template <class ValueType>
cl::Buffer OpenclBackend::CreateBufferByCopyVector(std::vector<ValueType> source_vector,
                                                   bool read_only) {
    //copy content from a source vector to global memory of device
    if (read_only) {
        return cl::Buffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                source_vector.size()*sizeof(ValueType), source_vector.data());
    } else {
        return cl::Buffer(context_, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                source_vector.size()*sizeof(ValueType), source_vector.data());
    }
}
