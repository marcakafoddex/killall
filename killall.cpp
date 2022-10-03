#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <optional>

void showUsage() {
	std::cout << "Usage: killall [OPTIONS] basename1 basename2 ... basenameN" << std::endl;
	std::cout << "  -l, --list     List processes that match given basename(s)" << std::endl;
	std::cout << "  -v             Be extra verbose" << std::endl;
	std::cout << "  -h, --help     Show this help and quit" << std::endl;
	std::cout << std::endl;
	std::cout << "Examples:" << std::endl;
	std::cout << "  Lists all processes named git but doesn't kill them:" << std::endl;
	std::cout << "    killall -l git" << std::endl;
	std::cout << "  Kills all processes named code or devenv:" << std::endl;
	std::cout << "    killall code devenv" << std::endl;
}

std::optional<std::string> getBaseName(DWORD processID) {
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
	if (NULL == processHandle)
		return {};

	HMODULE moduleHandle;
	DWORD actualNumberOfModules;
	char processName[MAX_PATH] = { 0 };
	if (EnumProcessModules(processHandle, &moduleHandle, sizeof(moduleHandle), &actualNumberOfModules))
		GetModuleBaseName(processHandle, moduleHandle, processName, sizeof(processName) / sizeof(char));
	CloseHandle(processHandle);
	return processName;
}

bool handleProcess(DWORD processID, bool verbose, bool doList, const std::vector<std::string>& matches) {
	if (!processID)
		return true;

	// try to get the basename, may fail if process is gone again
	auto data = getBaseName(processID);
	if (!data.has_value()) {
		if (verbose)
			std::cerr << "killall [" << processID << "]: failed to get basename" << std::endl;
		return true;
	}
	auto& baseName = *data;

	// strip common extensions
	for (const char* strippable : { ".exe", ".com", ".dll" }) {
		const size_t pos = baseName.find(strippable);
		if (baseName.size() >= 4 && pos == baseName.size() - 4)
			baseName.resize(pos);
	}
	
	// see if match
	bool isMatch = matches.empty();
	for (const std::string& match : matches) {
		if (match == baseName) {
			isMatch = true;
			break;
		}
	}

	if (isMatch) {
		if (doList) {
			std::cout << "killall [" << processID << "]: would kill '" << baseName << "'" << std::endl;
		}
		else {
			const HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, processID);
			const bool killed = hProcess && TerminateProcess(hProcess, 9);
			if (!killed)
				std::cerr << "killall [" << processID << "]: failed to terminate process" << std::endl;
			else if (verbose)
				std::cout << "killall [" << processID << "]: killed '" << baseName << "'" << std::endl;
			CloseHandle(hProcess);
			return killed;
		}
	}
	return true;
}

int main(int argc, const char** argv) {
	bool doList = false;
	bool verbose = false;
	std::vector<std::string> basenames;
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			showUsage();
			return 1;
		}
		if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list"))
			doList = true;
		else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose"))
			verbose = true;
		else
			basenames.push_back(argv[i]);
	}

	if (basenames.empty()) {
		showUsage();
		return 1;
	}

	DWORD processHandles[4096], actualNum;
	if (!EnumProcesses(processHandles, sizeof(processHandles), &actualNum)) {
		std::cerr << "Failed to enumerate all processes" << std::endl;
		return 1;
	}

	int result = 0;
	for (uint32_t i = 0; i < actualNum / sizeof(DWORD); i++)
		if (!handleProcess(processHandles[i], verbose, doList, basenames))
			result = 1;
	return result;
}
