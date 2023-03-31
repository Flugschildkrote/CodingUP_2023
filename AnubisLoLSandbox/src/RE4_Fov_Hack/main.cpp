#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>
#include <thread>
#include <array>
#include <string>
#include <future>
#include <cassert>

template <typename ...T_ARGS>
void aprint(T_ARGS&&... args)
{
    static std::mutex printLock;

    std::lock_guard<std::mutex> l(printLock);
    (std::cout << ... << args) << std::flush;
}

template <typename ...T_ARGS>
void aprintln(T_ARGS&&... args)
{
    aprint(std::forward<T_ARGS>(args)..., "\n");
}

namespace
{
    class FovUserReader
    {
    public:

        FovUserReader(void)
            :
            m_LastValue(0),
            m_ValueAvailable(false),
            m_Thread([this] { exec(); })
        {

        }

        void notifyRe4Closed(void) {

            {
                std::lock_guard lk(m_Mutex);
                m_LastValue = 0;
                m_ValueAvailable = true;
            }
            m_ConditionVar.notify_one();
        }

        /*
         Must not block
         */
        bool getValue(DWORD& out_fov)
        {
            std::unique_lock lk(m_Mutex);
            m_ConditionVar.wait(lk, [this] {return m_ValueAvailable; });

            if (m_ValueAvailable) {
                out_fov = m_LastValue;
            }
            return m_ValueAvailable;
        }
    private:

        void exec(void) noexcept
        {
            do {
                std::string line;
                std::getline(std::cin, line);
                bool error = false;

                DWORD newFov = 0;
                for (size_t i = 0; i < line.size() && !error; ++i) {
                    char num = line[i];
                    if (num < '0' || num > '9') {
                        error = true;
                    }

                    newFov *= 10;
                    newFov += (num - '0');
                }

                newFov = error ? 0 : newFov;
                newFov = (newFov < 90 || newFov > 280) ? 0 : newFov;

                {
                    std::lock_guard lk(m_Mutex);
                    m_LastValue = newFov;

                }
                m_ConditionVar.notify_one();

            } while (true);
        }

        DWORD m_LastValue;
        bool m_ValueAvailable;
        std::condition_variable m_ConditionVar;
        std::thread m_Thread;
        std::mutex m_Mutex;
    };


    struct ProcessInfo {
        HANDLE h;
        HANDLE hCallback;
        DWORD_PTR baseAddress;
        std::mutex m;
        std::atomic_bool processClosed;
        FovUserReader* fovReader;

        operator bool(void) const noexcept {
            return h != INVALID_HANDLE_VALUE;
        }

        static VOID CALLBACK ProcessExitedCallback(
            _In_  PVOID lpParameter,
            _In_  BOOLEAN TimerOrWaitFired
        )
        {
            ProcessInfo& processInfo = *reinterpret_cast<ProcessInfo*>(lpParameter);
            processInfo.m.lock();
            CloseHandle(processInfo.h);
           // CloseHandle(processInfo.hCallback);
            processInfo.h = INVALID_HANDLE_VALUE;
            processInfo.baseAddress = 0;
            processInfo.m.unlock();

            aprintln("[ProcessManager] re4.exe n'est plus ouvert.");
            processInfo.processClosed = true;
            processInfo.processClosed.notify_one();
            processInfo.fovReader->notifyRe4Closed();
        }
    };

    HANDLE GetProcessHandle(const std::wstring name)
    {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

        if (Process32First(snapshot, &entry) == TRUE)
        {
            do {
                //std::wcout << entry.szExeFile << std::endl;
                std::wstring processName(entry.szExeFile);
                const std::wstring& referenceName = name;
                if (processName == referenceName)
                {
                    CloseHandle(snapshot);
                    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
                }
            } while (Process32Next(snapshot, &entry) == TRUE);
        }

        return INVALID_HANDLE_VALUE;
    }

    DWORD_PTR GetProcessBaseAddress(HANDLE hProcess)
    {
        HMODULE lpModule[1024];
        DWORD lpcbNeeded;

        if (!EnumProcessModules(hProcess, lpModule, sizeof(lpModule), &lpcbNeeded))
        {
            return DWORD_PTR(0);
        }

        return reinterpret_cast<DWORD_PTR>(lpModule[0]);
    }

    bool ScanForProcess(const std::wstring& processName, ProcessInfo& procInfo)
    {
        HANDLE h = GetProcessHandle(processName);
        if (h == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        DWORD_PTR base = GetProcessBaseAddress(h);
        if (base == 0) 
        {
            return false;
        }

        procInfo.h = h;
        procInfo.baseAddress = base;
        return true;
    }

    bool ReadAddressFromOffsetChain(const ProcessInfo& p, DWORD startAddress, const std::vector<DWORD>& offsetsChain, DWORD_PTR& outAddress)
    {
        DWORD_PTR address = p.baseAddress + startAddress;
        SIZE_T numBytes;

        for (DWORD offset : offsetsChain)
        {
            if (!ReadProcessMemory(p.h, (void*)address, &address, sizeof(DWORD_PTR), &numBytes))
            {
                return false;
            }
            address += offset;
        }

        outAddress = address;
        return true;
    }

 

    enum FG_COLORS
    {
        FG_BLACK = 0,
        FG_BLUE = 1,
        FG_GREEN = 2,
        FG_CYAN = 3,
        FG_RED = 4,
        FG_MAGENTA = 5,
        FG_BROWN = 6,
        FG_LIGHTGRAY = 7,
        FG_GRAY = 8,
        FG_LIGHTBLUE = 9,
        FG_LIGHTGREEN = 10,
        FG_LIGHTCYAN = 11,
        FG_LIGHTRED = 12,
        FG_LIGHTMAGENTA = 13,
        FG_YELLOW = 14,
        FG_WHITE = 15
    };

    /*Enum to store Background colors*/
    enum BG_COLORS
    {
        BG_NAVYBLUE = 16,
        BG_GREEN = 32,
        BG_TEAL = 48,
        BG_MAROON = 64,
        BG_PURPLE = 80,
        BG_OLIVE = 96,
        BG_SILVER = 112,
        BG_GRAY = 128,
        BG_BLUE = 144,
        BG_LIME = 160,
        BG_CYAN = 176,
        BG_RED = 192,
        BG_MAGENTA = 208,
        BG_YELLOW = 224,
        BG_WHITE = 240
    };

   

}

int main(int argc, char** argv) 
{
    
    const DWORD addressMainOffset = 0x0D22CFE8;
    const std::vector<DWORD> offsetsChain = {
        0x80, 
        0x40, 
        0x160, 
        0x14
    };



    FovUserReader reader;
    ProcessInfo re4;
    re4.h = INVALID_HANDLE_VALUE;
    re4.baseAddress = 0;
    re4.processClosed = false;
    re4.fovReader = &reader;

    //HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    //SetConsoleTextAttribute(hStdOut, FOREGROUND_RED);

 

    std::cout <<
        "--------------------------------\n"
        "-----[Resident Evil 4, FOV]-----\n"
        "--------------------------------\n"
        << std::endl;

    goto loop;

error:

    std::cout << "\nErreur lors d'une operation. Verifiez que re4.exe est ouvert" << std::endl;
loop:


    do {
        aprint("[ProcessManager] En attente du lancement de re4.exe...");

        re4.processClosed = false;
        re4.m.lock();
        while (!ScanForProcess(L"re4.exe", re4))
        {
            re4.m.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            re4.m.lock();
        }

        re4.m.unlock();
        RegisterWaitForSingleObject(&re4.hCallback, re4.h, ProcessInfo::ProcessExitedCallback, &re4, INFINITE, WT_EXECUTEONLYONCE);
        aprintln("[OK]");

        SIZE_T numBytes;
        DWORD_PTR fovAddress;
        DWORD fov = 0;

        std::cout << "Lecture de la FOV actuelle : ";
        // Read the FOV address
        if (!ReadAddressFromOffsetChain(re4, addressMainOffset, offsetsChain, fovAddress))
            goto error;

        if (!ReadProcessMemory(re4.h, (void*)(fovAddress), &fov, sizeof(DWORD), &numBytes))
            goto error;

         std::cout << fov << std::endl;

    read_again:
        std::cout << "Entrer la nouvelle Fov (entre 90 et 280) : ";
        if (!reader.getValue(fov)) {
            if (re4.processClosed)
                goto error;
            else {
                std::cout << "Valeur invalide" << std::endl;
                goto read_again;
            }
        }
             
        if (WriteProcessMemory(re4.h, (void*)(fovAddress), &fov, sizeof(DWORD), &numBytes)) {
            std::cout << "Valeur changee avec succes";
        }
        else {
            goto error;
        }

    } while (true);

    return EXIT_SUCCESS;
}