// clang-format off

#ifndef UNICODE
#define UNICODE
#endif

#define TIMER_INTERVAL_MS 60000

#define WM_CREATE_PARAM 8000
#define WM_EXIT_PARAM 1
#define WM_TIMER_PARAM 2

#include <Windows.h>
#include <tlhelp32.h>
#include <shellapi.h>

#include <cstring>
#include <iostream>

#include <stdio.h>
#include <time.h>

// clang-format on

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void CreateIcon(NOTIFYICONDATA &nid, HWND &hWnd, LPCWSTR &szTip);
void CreateIconMenu(HWND &hWnd);
void SetAffinity(wchar_t *szName);

LPCWSTR lpszClass = L"__hidden__";
LPCWSTR szTip = L"CSGO Lasso";
wchar_t *exeName = L"csgo.exe";

// clang-format off
  
int main() {
  // Remove console window
  HWND hConsoleWnd = GetConsoleWindow();
  ShowWindow(hConsoleWnd, SW_HIDE);

  HINSTANCE hInstance = GetModuleHandle(nullptr);

  WNDCLASS wc;
  HWND hWnd;
  MSG msg;

  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hbrBackground = nullptr;
  wc.hCursor = nullptr;
  wc.hIcon = nullptr;
  wc.hInstance = hInstance;
  wc.lpfnWndProc = WndProc;
  wc.lpszClassName = lpszClass;
  wc.lpszMenuName = nullptr;
  wc.style = 0;

  RegisterClass(&wc);

  hWnd = CreateWindow(
    lpszClass, 
    lpszClass, 
    WS_OVERLAPPEDWINDOW, 
    CW_USEDEFAULT,
    CW_USEDEFAULT, 
    CW_USEDEFAULT, 
    CW_USEDEFAULT, 
    nullptr,
    nullptr, 
    hInstance,
    nullptr
  );

  SetTimer(
    hWnd, 
    WM_TIMER_PARAM, 
    TIMER_INTERVAL_MS, 
    NULL
  );

  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return static_cast<int>(msg.wParam);
}

// clang-format on

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
  static NOTIFYICONDATA nid;

  switch (iMsg) {
    case WM_CREATE:
      CreateIcon(nid, hWnd, szTip);

      // Set affinity on load
      SetAffinity(exeName);
      return 0;
    case WM_CREATE_PARAM:
      switch (lParam) {
        case WM_RBUTTONDOWN:
          CreateIconMenu(hWnd);
          break;
      }
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case WM_EXIT_PARAM:
          Shell_NotifyIcon(NIM_DELETE, &nid);
          PostQuitMessage(0);
          break;
      }
      break;
    case WM_TIMER:
      SetAffinity(exeName);
      break;
    case WM_DESTROY:
      Shell_NotifyIcon(NIM_DELETE, &nid);
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

void CreateIcon(NOTIFYICONDATA &nid, HWND &hWnd, LPCWSTR &szTip) {
  std::memset(&nid, 0, sizeof(nid));

  nid.cbSize = sizeof(nid);
  nid.hWnd = hWnd;
  nid.uID = 0;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  nid.uCallbackMessage = WM_CREATE_PARAM;
  nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

  lstrcpy(nid.szTip, szTip);

  Shell_NotifyIcon(NIM_ADD, &nid);
  Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

// clang-format off

void CreateIconMenu(HWND &hWnd) {
  HMENU hmenu;
  hmenu = CreatePopupMenu();

  POINT pt;
  GetCursorPos(&pt);

  InsertMenu(
    hmenu, 
    0, 
    MF_BYPOSITION | MF_STRING, WM_EXIT_PARAM,
    L"Exit"
  );
  
  TrackPopupMenu(
    hmenu,
    TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, 
    pt.x,
    pt.y, 
    0, 
    hWnd, 
    NULL
  );
}

// clang-format on

void SetAffinity(wchar_t *szName) {
  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 1);

  ULONG_PTR lpProcAffinity;
  ULONG_PTR lpSysAffinity;

  if (Process32First(snapshot, &entry) == TRUE) {
    while (Process32Next(snapshot, &entry) == TRUE) {
      if (wcscmp(entry.szExeFile, szName) == 0) {
        HANDLE hProcess =
            OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

        if (GetProcessAffinityMask(hProcess, &lpProcAffinity, &lpSysAffinity)) {
          // Ensures the EXE is running on all cores except core 0
          if (lpProcAffinity != lpSysAffinity - 1) {
            SetProcessAffinityMask(hProcess, lpSysAffinity - 1);
          }
        }

        CloseHandle(hProcess);
      }
    }
  }

  CloseHandle(snapshot);
}