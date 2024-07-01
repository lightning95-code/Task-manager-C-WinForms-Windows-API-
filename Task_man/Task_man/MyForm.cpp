#include "MyForm.h"
#include <Windows.h>
#include <Psapi.h>
#include <processthreadsapi.h>
#include <handleapi.h>
#include <tchar.h> //для підтримки формату виводу
#include <string>
#include <iostream>
#include <iomanip>


using namespace System;
using namespace System::Windows::Forms;


[STAThreadAttribute]
void main(array<String^>^ args) {
    Application::SetCompatibleTextRenderingDefault(false);
    Application::EnableVisualStyles();
    Taskman::MyForm form;
    Application::Run(% form);
}


void Taskman::MyForm::Start_analysis() {
    DWORD aProcesses[1024], cbNeeded = 0, cProcesses = 0;
    EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded);
    cProcesses = cbNeeded / sizeof(DWORD);
    int GlPrNum = 1;

    // Починаю проходити по кожному процесу:
    for (DWORD i = 0; i < cProcesses; i++) {
        std::string status;

        TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

        // Get a handle to the process.
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
        if (hProcess != NULL) {
            HMODULE hMod;
            DWORD cbNeeded;
            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
            }

            PROCESS_MEMORY_COUNTERS_EX pmc;
            if (GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
                // Вивод 
                int moduleCount = 0;
                HANDLE hProcessModules = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
                if (hProcessModules != NULL) {
                    HMODULE hMods[1024];
                    DWORD cbNeeded;
                    if (EnumProcessModulesEx(hProcessModules, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
                        moduleCount = cbNeeded / sizeof(HMODULE);
                    }
                    CloseHandle(hProcessModules);
                }

                DWORD exitCode;
                if (GetExitCodeProcess(hProcess, &exitCode)) {
                    if (exitCode == STILL_ACTIVE) {
                        status = "running";
                    }
                    else {
                        status = "exited";
                    }
                }

                

                int rowIndex = dataGridView1->Rows->Add();
                dataGridView1->Rows[rowIndex]->Cells[0]->Value = gcnew String(szProcessName); //назва процесу
                dataGridView1->Rows[rowIndex]->Cells[1]->Value = GlPrNum; // Номер процеса по порядку
                dataGridView1->Rows[rowIndex]->Cells[2]->Value = aProcesses[i]; // id
                dataGridView1->Rows[rowIndex]->Cells[3]->Value = moduleCount; // кількість модулів
                dataGridView1->Rows[rowIndex]->Cells[4]->Value = pmc.WorkingSetSize / (static_cast<unsigned long long>(1024) * 1024); // занята пам'ять в МБ
                dataGridView1->Rows[rowIndex]->Cells[5]->Value = gcnew String(status.c_str()); // статус процесса

                GlPrNum++;
            }

            // Release the handle to the process.
            CloseHandle(hProcess);
        }
    }
}



System::Void Taskman::MyForm::ExitButton_Click(System::Object^ sender, System::EventArgs^ e) {
    Application::Exit();
}

System::Void Taskman::MyForm::Update_Button_Click(System::Object^ sender, System::EventArgs^ e) {
    dataGridView1->Rows->Clear(); 
    Start_analysis(); 
}

System::Void Taskman::MyForm::StartButton_Click(System::Object^ sender, System::EventArgs^ e) {
    dataGridView1->Rows->Clear();
    Start_analysis();
    StartButton->Enabled = false;
}

System::Void Taskman::MyForm::DeleteButton_Click(System::Object^ sender, System::EventArgs^ e) {
        int selectedRow = dataGridView1->CurrentRow->Index;
        int processId = Convert::ToInt32(dataGridView1->Rows[selectedRow]->Cells[2]->Value);
        dataGridView1->Rows[selectedRow]->Cells[5]->Value = gcnew String("terminated");
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
}

System::Void Taskman::MyForm::CreateButton_Click(System::Object^ sender, System::EventArgs^ e) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    LPWSTR applicationName = L"C:\\Windows\\System32\\notepad.exe"; //шлях до Notepad
    LPWSTR commandLineArgs = L"D:\\vs_pr\\Task Manager\\Task_man\\myfile.txt"; //шлях до файлу

    if (!CreateProcess(applicationName, commandLineArgs, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        MessageBox::Show("Failed to create process!");
    }
}



