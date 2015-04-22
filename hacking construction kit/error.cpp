#include "error.h"

using namespace std;

HANDLE GlobalError::mutex = 0;
vector<unsigned long> GlobalError::errors;

GlobalError::GlobalError(){
	mutex = CreateMutex(NULL, FALSE, NULL);
}

void GlobalError::saveError(unsigned long error){
	WaitForSingleObject(mutex, INFINITE);

	errors.push_back(error * 100000);

	ReleaseMutex(mutex);
}

void GlobalError::saveError(unsigned long error, unsigned long windowsError){
	WaitForSingleObject(mutex, INFINITE);

	errors.push_back(error * 100000 + windowsError);

	ReleaseMutex(mutex);
}

unsigned long GlobalError::getLastError(){
	WaitForSingleObject(mutex, INFINITE);

	if (errors.size() == 0){
		return 0;
	}
	int lastError = errors.back();
	errors.pop_back();

	ReleaseMutex(mutex);

	return lastError;
}