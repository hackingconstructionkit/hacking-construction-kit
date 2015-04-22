/*
 * Author: thirdstormofcythraul@outlook.com
 */
#include "plugin_loader.h"

#include <vector>
#include <algorithm>

#include <Windows.h>

#include "register.h"
#include "macro.h"
#include "plugin.h"

#include "memory_debug.h"

PluginLoader::PluginLoader(){

}

PluginLoader::~PluginLoader(){

}

PluginLoader PluginLoader::instance = PluginLoader();
std::vector<Plugin*> PluginLoader::plugs = std::vector<Plugin*>();

PluginLoader& PluginLoader::getInstance(){
	return instance;
}

void PluginLoader::loadAllPlugins(){
	wchar_t* path = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\");
	wchar_t* key = TEXT("Plugs");
	Register reg;
	std::wstring plugs = reg.getKey(HKEY_LOCAL_MACHINE, path, key);


	std::vector<std::wstring> theStringVector;
	const std::wstring theDelimiter = L",";
	size_t start = 0, end = 0;

	while (end != std::string::npos) {
		end = plugs.find(theDelimiter, start);
		// If at end, use length=maxLength.  Else use length=end-start.
		theStringVector.push_back(plugs.substr(start, (end == std::string::npos) ? std::string::npos : end - start));
		// If at end, use start=maxSize.  Else use start=end+delimiter.
		start = ((end > (std::string::npos - theDelimiter.size())) ?  std::string::npos : end + theDelimiter.size());
	}

	for(std::size_t i = 0; i < theStringVector.size(); ++i) {
		std::wstring plug = theStringVector[i];
		if (!plug.empty()){
			MYPRINTF("try to load plugin %w\n", plug.c_str());
			PluginLoader::getInstance().load(plug.c_str());
		}
	}

}

void PluginLoader::stop(){
	std::vector<Plugin*> temp;
	for(std::size_t i = 0; i < PluginLoader::plugs.size(); ++i) {
		temp.push_back(PluginLoader::plugs[i]);
	}

	for(std::size_t i = 0; i < temp.size(); ++i) {
		Plugin* plug = temp[i];
		if (plug){
			plug->stop();
			Sleep(1000);
		}
	}
}

void PluginLoader::load(const wchar_t *plugin){

	wchar_t *p = new wchar_t[wcslen(plugin) + 1];
	wcscpy_s(p, wcslen(plugin) + 1, plugin);

	HANDLE handle = CreateThread(NULL, 0, startThread, p, 0, 0);	

	WaitForSingleObject(handle, 1000);

	m_handles.push_back(handle);
}

DWORD WINAPI PluginLoader::startThread(LPVOID params) {
	wchar_t *pluginname = (wchar_t *)params;

	HINSTANCE hLibrary = (HINSTANCE)::LoadLibrary(pluginname);
	if (!hLibrary){
		MYPRINTF("unable to load %w\n", pluginname);
		return 1;
	}

	typedef void (*GETPLUGIN)(Plugin**);
	GETPLUGIN pfnLoad = (GETPLUGIN)::GetProcAddress(hLibrary, "GetPlugin");
	if (!pfnLoad){
		MYPRINTF("unable to find proccess %s\n", "GetPlugin");
		return 1;
	}

	Plugin* plugged = 0;
	pfnLoad(&plugged);

	if (!plugged){
		MYPRINTF("unable to create class\n");
		return 1;
	} else {
		PluginLoader::plugs.push_back(plugged);
		plugged->start();
	}

	PluginLoader::plugs.erase(std::remove(PluginLoader::plugs.begin(), PluginLoader::plugs.end(), plugged), PluginLoader::plugs.end());
	plugged->destroy();
	::FreeLibrary(hLibrary);
	delete[] pluginname;
	return 0;
}