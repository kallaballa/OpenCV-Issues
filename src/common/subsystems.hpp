#ifndef SRC_SUBSYSTEMS_HPP_
#define SRC_SUBSYSTEMS_HPP_

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <filesystem>
#include <va/va.h>
#include <va/va_drm.h>
#include <va/va_backend.h>
#include <opencv2/opencv.hpp>
#include "opencv2/core/va_intel.hpp"
#include <opencv2/videoio.hpp>

using std::cout;
using std::cerr;
using std::endl;

namespace kb {

namespace va {
//code in the kb::va namespace adapted from https://github.com/opencv/opencv/blob/4.x/samples/va_intel/display.cpp.inc
cv::ocl::OpenCLExecutionContext context;
VADisplay display = NULL;
bool initialized = false;

bool open_display();
void close_display();

#define VA_INTEL_PCI_DIR "/sys/bus/pci/devices"
#define VA_INTEL_DRI_DIR "/dev/dri/"
#define VA_INTEL_PCI_DISPLAY_CONTROLLER_CLASS 0x03

static unsigned read_id(const char *devName, const char *idName);
static int find_adapter(unsigned desiredVendorId);

int drmfd = -1;

class Directory {
    typedef int (*fsort)(const struct dirent**, const struct dirent**);
public:
    Directory(const char *path) {
        dirEntries_ = 0;
        numEntries_ = scandir(path, &dirEntries_, filterFunc, (fsort) alphasort);
    }
    ~Directory() {
        if (numEntries_ && dirEntries_) {
            for (int i = 0; i < numEntries_; ++i)
                free(dirEntries_[i]);
            free(dirEntries_);
        }
    }
    int count() const {
        return numEntries_;
    }
    const struct dirent* operator[](int index) const {
        return ((dirEntries_ != 0) && (index >= 0) && (index < numEntries_)) ? dirEntries_[index] : 0;
    }
protected:
    static int filterFunc(const struct dirent *dir) {
        if (!dir)
            return 0;
        if (!strcmp(dir->d_name, "."))
            return 0;
        if (!strcmp(dir->d_name, ".."))
            return 0;
        return 1;
    }
private:
    int numEntries_;
    struct dirent **dirEntries_;
};

static unsigned read_id(const char *devName, const char *idName) {
    long int id = 0;

    std::string fileName = cv::format("%s/%s/%s", VA_INTEL_PCI_DIR, devName, idName);

    FILE *file = fopen(fileName.c_str(), "r");
    if (file) {
        char str[16] = "";
        if (fgets(str, sizeof(str), file))
            id = strtol(str, NULL, 16);
        fclose(file);
    }
    return (unsigned) id;
}

static int find_adapter(unsigned desiredVendorId) {
    int adapterIndex = -1;

    Directory dir(VA_INTEL_PCI_DIR);

    for (int i = 0; i < dir.count(); ++i) {
        const char *name = dir[i]->d_name;

        unsigned classId = read_id(name, "class");
        if ((classId >> 16) == VA_INTEL_PCI_DISPLAY_CONTROLLER_CLASS) {
            unsigned vendorId = read_id(name, "vendor");
            if (vendorId == desiredVendorId) {
                std::string subdirName = cv::format("%s/%s/%s", VA_INTEL_PCI_DIR, name, "drm");
                Directory subdir(subdirName.c_str());
                for (int j = 0; j < subdir.count(); ++j) {
                    if (!strncmp(subdir[j]->d_name, "card", 4)) {
                        adapterIndex = strtoul(subdir[j]->d_name + 4, NULL, 10);
                    }
                }
                break;
            }
        }
    }

    return adapterIndex;
}

class NodeInfo {
    enum {
        NUM_NODES = 2
    };
public:
    NodeInfo(int adapterIndex) {
        const char *names[NUM_NODES] = { "renderD", "card" };
        int numbers[NUM_NODES];
        numbers[0] = adapterIndex + 128;
        numbers[1] = adapterIndex;
        for (int i = 0; i < NUM_NODES; ++i) {
            paths_[i] = cv::format("%s%s%d", VA_INTEL_DRI_DIR, names[i], numbers[i]);
        }
    }
    ~NodeInfo() {
        // nothing
    }
    int count() const {
        return NUM_NODES;
    }
    const char* path(int index) const {
        return ((index >= 0) && (index < NUM_NODES)) ? paths_[index].c_str() : 0;
    }
private:
    std::string paths_[NUM_NODES];
};

static bool open_device_intel();
static bool open_device_generic();

static bool open_device_intel() {
    const unsigned IntelVendorID = 0x8086;

    int adapterIndex = find_adapter(IntelVendorID);
    if (adapterIndex >= 0) {
        NodeInfo nodes(adapterIndex);

        for (int i = 0; i < nodes.count(); ++i) {
            drmfd = open(nodes.path(i), O_RDWR);
            if (drmfd >= 0) {
                display = vaGetDisplayDRM(drmfd);
                vaSetInfoCallback(display, nullptr, nullptr);
                if (display)
                    return true;
                close(drmfd);
                drmfd = -1;
            }
        }
    }
    return false;
}

static bool open_device_generic() {
    static const char *device_paths[] = { "/dev/dri/renderD128", "/dev/dri/card0" };
    static const int num_devices = sizeof(device_paths) / sizeof(device_paths[0]);

    for (int i = 0; i < num_devices; ++i) {
        drmfd = open(device_paths[i], O_RDWR);
        if (drmfd >= 0) {
            display = vaGetDisplayDRM(drmfd);
            vaSetInfoCallback(display, nullptr, nullptr);
            if (display)
                return true;
            close(drmfd);
            drmfd = -1;
        }
    }
    return false;
}

bool open_display() {
    if (!initialized) {
        drmfd = -1;
        display = 0;

        if (open_device_intel() || open_device_generic()) {
            int majorVersion = 0, minorVersion = 0;
            if (vaInitialize(display, &majorVersion, &minorVersion) == VA_STATUS_SUCCESS) {
                initialized = true;
                return true;
            }
            close(drmfd);
            display = 0;
            drmfd = -1;
        }
        return false; // Can't open VA display
    }
    return true;
}

void close_display() {
    if (initialized) {
        if (display)
            vaTerminate(display);
        if (drmfd >= 0)
            close(drmfd);
        display = 0;
        drmfd = -1;
        initialized = false;
    }
}

void check_if_YUV420_available() {
    VAEntrypoint entrypoints[5];
    int num_entrypoints, vld_entrypoint;
    VAConfigAttrib attrib;
    VAStatus status;

    status = vaQueryConfigEntrypoints(va::display, VAProfileVP9Profile0, entrypoints, &num_entrypoints);
    assert(status == VA_STATUS_SUCCESS);

    for (vld_entrypoint = 0; vld_entrypoint < num_entrypoints; ++vld_entrypoint) {
        if (entrypoints[vld_entrypoint] == VAEntrypointVLD)
            break;
    }
    if (vld_entrypoint == num_entrypoints)
        throw std::runtime_error("Failed to find VLD entry point");

    attrib.type = VAConfigAttribRTFormat;
    vaGetConfigAttributes(va::display, VAProfileVP9Profile0, VAEntrypointVLD, &attrib, 1);
    if ((attrib.value & VA_RT_FORMAT_YUV420) == 0)
        throw std::runtime_error("Desired YUV420 RT format not found");
}

void init() {
    if (!va::open_display())
        throw std::runtime_error("Failed to open VA display for CL-VA interoperability");

    va::check_if_YUV420_available();

    cv::va_intel::ocl::initializeContextFromVA(va::display, true);

    va::context = cv::ocl::OpenCLExecutionContext::getCurrent();
}

std::string get_info() {
    std::stringstream ss;
    ss << VA_VERSION_S << " (" << vaQueryVendorString(display) << ")";
    return ss.str();
}

void bind() {
    context.bind();
}
} // namespace va
}

#endif /* SRC_SUBSYSTEMS_HPP_ */
