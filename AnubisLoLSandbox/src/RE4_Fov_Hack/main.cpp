#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>
#include <thread>
#include <array>
#include <string>

namespace
{

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

        throw std::runtime_error("Impossible de trouver re4.exe. Verifiez que le jeu est ouvert.");
    }

    DWORD_PTR GetProcessBaseAddress(HANDLE hProcess)
    {
        HMODULE lpModule[1024];
        DWORD lpcbNeeded;

        if (!EnumProcessModules(hProcess, lpModule, sizeof(lpModule), &lpcbNeeded))
        {
            throw std::runtime_error("Failed to get process base address");
        }

        return reinterpret_cast<DWORD_PTR>(lpModule[0]);
    }
}

HANDLE hProcess = INVALID_HANDLE_VALUE;
DWORD_PTR moduleBaseAddr = 0x00;

int main(int argc, char** argv)
{
    try {
        hProcess = GetProcessHandle(L"re4.exe");
        moduleBaseAddr = GetProcessBaseAddress(hProcess);

        //std::cout << "Module base address= 0x" << std::hex << moduleBaseAddr << std::endl;

        std::cout <<
            "--------------------------------\n"
            "-----[Resident Evil 4, FOV]-----\n"
            "--------------------------------\n"
            << std::endl;

        SIZE_T numBytes;

        DWORD addressMainOffset = 0x0D22CFE8;

        DWORD_PTR address;
        DWORD fov = 0;
        ReadProcessMemory(hProcess, (void*)(moduleBaseAddr + addressMainOffset), &address, sizeof(DWORD_PTR), &numBytes);
        ReadProcessMemory(hProcess, (void*)(address + 0x80), &address, sizeof(DWORD_PTR), &numBytes);
        ReadProcessMemory(hProcess, (void*)(address + 0x40), &address, sizeof(DWORD_PTR), &numBytes);
        ReadProcessMemory(hProcess, (void*)(address + 0x160), &address, sizeof(DWORD_PTR), &numBytes);

        DWORD_PTR fov_address = address + 0x14;
        ReadProcessMemory(hProcess, (void*)(fov_address), &fov, sizeof(DWORD), &numBytes);


        do {
            std::cout << std::dec << "FOV Actuelle : " << fov << "\n";
            std::cout << "Entrer une nouvelle FOV (ou q pour quitter) :"; 

            std::string line;
            std::getline(std::cin, line);

            if (line == "q" || line == "Q") {
                break;
            }
            else {

                bool error = false;
                DWORD newFov = 0;
                for (size_t i = 0; i < line.size() && !error; ++i) {
                    char num = line[i];
                    if (num < '0' || num > '9') {
                        std::cerr << "Nombre invalide. Entrer un nombre entre 90 et 280" << std::endl;
                        error = true;
                    }

                    newFov *= 10;
                    newFov += (num - '0');
                }

                if (!error) {
                    if (newFov < 90 || newFov > 280) {
                        std::cerr << "Nombre invalide. Entrer un nombre entre 90 et 280" << std::endl;
                    }
                    else {
                        fov = newFov;
                    }
                }
            }

            WriteProcessMemory(hProcess, (void*)(fov_address), &fov, sizeof(DWORD), &numBytes);
        } while (true);

        CloseHandle(hProcess);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        system("Pause");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}