// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefsimple/simple_handler.h"

#include <sstream>
#include <string>
#include <ole2.h>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "simple_app.h"

namespace {

	SimpleHandler* g_instance = NULL;

}  // namespace

SimpleHandler::SimpleHandler()
	: is_closing_(false) {
	DCHECK(!g_instance);
	g_instance = this;
}

SimpleHandler::~SimpleHandler() {
	g_instance = NULL;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
	return g_instance;
}

void GetDesktopResolutionDup(int& horizontal, int& vertical)
{
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	horizontal = desktop.right;
	vertical = desktop.bottom;
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Add to the list of existing browsers.
	browser_list_.push_back(browser);

	// Disable drag and drop
	HWND hWndHost = browser->GetHost()->GetWindowHandle();
	HWND hWndHostChild = GetWindow(hWndHost, GW_CHILD);
	RevokeDragDrop(hWndHostChild);

	// Remove border
	LONG lStyle = GetWindowLongPtr(hWndHost, GWL_STYLE);
	lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
	SetWindowLongPtr(hWndHost, GWL_STYLE, lStyle);

	LONG lExStyle = GetWindowLongPtr(hWndHost, GWL_EXSTYLE);
	lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	SetWindowLongPtr(hWndHost, GWL_EXSTYLE, lExStyle);

	SetWindowPos(hWndHost, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

	// Force 16:9 ratio

	CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();

	RECT desktop;
	GetWindowRect(hWndHost, &desktop);
	desktop.bottom = desktop.bottom;
	desktop.right = desktop.right;

	if (command_line->HasSwitch("window"))
	{
		GetWindowRect(hWndHost, &desktop);
		desktop.bottom = 0.9 * desktop.bottom;
		desktop.right = desktop.bottom * 16 / 9;
	}

	if (command_line->HasSwitch("fullscreen"))
	{
		GetWindowRect(hWndHost, &desktop);
		desktop.bottom = desktop.bottom;
		desktop.right = desktop.right;
	}
	// SetWindowPos(hWndHost, 0, 0, 0, desktop.right, desktop.bottom, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

	// Recenter
	int horizontal, vertical;
	GetDesktopResolutionDup(horizontal, vertical);
	int xPos = (horizontal - desktop.right) / 2;
	int yPos = (vertical - desktop.bottom) / 2;
	MoveWindow(hWndHost, xPos, yPos, desktop.right, desktop.bottom, true);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	ShowWindow(SimpleApp::gameWindow, SW_SHOW);

	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed destription of this
	// process.
	if (browser_list_.size() == 1) {
		// Set a flag to indicate that the window close should be allowed.
		is_closing_ = true;
	}

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Remove from the list of existing browsers.
	BrowserList::iterator bit = browser_list_.begin();
	for (; bit != browser_list_.end(); ++bit) {
		if ((*bit)->IsSame(browser)) {
			browser_list_.erase(bit);
			break;
		}
	}

	if (browser_list_.empty()) {
		// All browser windows have closed. Quit the application message loop.
		CefQuitMessageLoop();
	}
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	ErrorCode errorCode,
	const CefString& errorText,
	const CefString& failedUrl) {
	CEF_REQUIRE_UI_THREAD();

	// Don't display an error for downloaded files.
	if (errorCode == ERR_ABORTED)
		return;

	// Display a load error message.
	std::stringstream ss;
	ss << "<html><body bgcolor=\"white\">"
		"<h2>Failed to load URL " << std::string(failedUrl) <<
		" with error " << std::string(errorText) << " (" << errorCode <<
		").</h2></body></html>";
	frame->LoadString(ss.str(), failedUrl);
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
	if (!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI,
			base::Bind(&SimpleHandler::CloseAllBrowsers, this, force_close));
		return;
	}

	if (browser_list_.empty())
		return;

	BrowserList::const_iterator it = browser_list_.begin();
	for (; it != browser_list_.end(); ++it)
		(*it)->GetHost()->CloseBrowser(force_close);
}