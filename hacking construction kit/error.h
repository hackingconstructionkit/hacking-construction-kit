#pragma once

#include <vector>

#include <Windows.h>

// Static class for manage errors
class GlobalError {
public:
	enum err {
		Screen1 = 1,
		Screen2,
		Screen3,
		Screen4,
		Screen5,
		Screen6,
		Screen7,
		Screen8,
		Screen9,
		Screen10,//10
		Screen11,
		Screen12,
		Screen13,
		Screen14,
		sClient1,
		sClient2,
		sClient3,
		sClient4,
		sClient5,
		cam1,//20
		cam2,
		cam3,
		cam4,
		cam5,
		cam6,
		cam7,
		cam8,
		cam9,
		cam10,
		cam11,//30
		cam12,
		cam13,
		cam14,
		cam15,
		cam16,
		cam17,
		cam18,
		cam19,
		cam20,
		cam21,//40
		cam22,
		cam23,
		cam24,
		cam25

	};

	// Record an error
	static void saveError(unsigned long error);

	static void saveError(unsigned long error, unsigned long windowsError);

	// Get last error recorded
	static unsigned long getLastError();
	
private:
	GlobalError();

	static GlobalError globalError;

	static std::vector<unsigned long> errors;

	 static HANDLE mutex;

};