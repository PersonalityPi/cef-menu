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

SimpleApp::SimpleApp() {}

void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  // Information used when creating the native window.
  CefWindowInfo window_info;

  CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();

  std::string hwndString = command_line->GetSwitchValue("hwnd").ToString();
  if (!hwndString.empty())
  {
    gameWindow = (HWND)stoi(hwndString);
	RECT windowRect;
	GetClientRect(gameWindow, &windowRect);
    window_info.SetAsChild(gameWindow, windowRect);
  }
  else
  {
    CefShutdown();
  }

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

  std::wstring url = L"http://no1dead.github.io/";

  std::wstring urlString = command_line->GetSwitchValue("url").ToWString();
  if (!urlString.empty())
  {
	  url = urlString;
  }

  // Create the first browser window.
  CefBrowserHost::CreateBrowser(window_info, handler.get(), url,
	  browser_settings, NULL);
}