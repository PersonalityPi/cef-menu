// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefsimple/simple_app.h"

#include <string>

#include "cefsimple/simple_handler.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"
#include <shlwapi.h>
#include <fstream>

HWND SimpleApp::gameWindow = 0;

SimpleApp::SimpleApp() {
}

void GetDesktopResolution(int& horizontal, int& vertical)
{
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	horizontal = desktop.right;
	vertical = desktop.bottom;
}

void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  // Information used when creating the native window.
  CefWindowInfo window_info;

#if defined(OS_WIN)
  // On Windows we need to specify certain flags that will be passed to
  // CreateWindowEx().
  window_info.SetAsPopup(NULL, "Custom Menu");
#endif

  int horizontal, vertical;
  GetDesktopResolution(horizontal, vertical);
  window_info.width = horizontal;
  window_info.height = vertical;

  int xPos = (horizontal - window_info.width) / 2;
  int yPos = (vertical - window_info.height) / 2;
  window_info.x = xPos;
  window_info.y = yPos;

  // SimpleHandler implements browser-level callbacks.
  CefRefPtr<SimpleHandler> handler(new SimpleHandler());

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;

  wchar_t pathToOurDirectory[260];
  GetModuleFileName(NULL, pathToOurDirectory, 260);
  PathRemoveFileSpec(pathToOurDirectory);

  std::wstring path(pathToOurDirectory);
  path.append(L"\\mods\\menus\\default");

  CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
  manager->SetStoragePath(path, true, NULL);

  CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();

  std::wstring url = L"http://no1dead.github.io/";

  std::wstring urlString = command_line->GetSwitchValue("url").ToWString();
  if (!urlString.empty())
  {
	  url = urlString;
  }

  if (command_line->HasSwitch("window"))
  {
	  window_info.width = horizontal - 100;
	  window_info.height = vertical - 100;
	  xPos = (horizontal - window_info.width) / 2;
	  yPos = (vertical - window_info.height) / 2;
	  window_info.x = xPos;
	  window_info.y = yPos;
  }

  if (command_line->HasSwitch("fullscreen"))
  {
	  window_info.width = horizontal;
	  window_info.height = vertical;
	  xPos = (horizontal - window_info.width) / 2;
	  yPos = (vertical - window_info.height) / 2;
	  window_info.x = xPos;
	  window_info.y = yPos;
  }

  std::string hwndString = command_line->GetSwitchValue("hwnd").ToString();
  if (!hwndString.empty())
  {
	  gameWindow = (HWND)stoi(hwndString);
	  ShowWindow(gameWindow, SW_HIDE);
  }
  else
  {
	  CefShutdown();
  }

  // Create the first browser window.
  CefBrowserHost::CreateBrowser(window_info, handler.get(), url,
	  browser_settings, NULL);
}