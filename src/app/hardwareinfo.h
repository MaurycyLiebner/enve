#ifndef HARDWAREINFO_H
#define HARDWAREINFO_H

class HardwareInfo {
    HardwareInfo() = delete;
public:
    static void sUpdateInfo();
    static int sCpuThreads() { return mCpuThreads; }

    static long sRamBytes() { return mRamBytes; }
    static int sRamKB() { return mRamKB; }
private:
    static int mCpuThreads;

    static long mRamBytes;
    static int mRamKB;
};

#endif // HARDWAREINFO_H
