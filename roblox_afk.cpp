#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

// 键盘扫描码定义
#define SCANCODE_SPACE 0x39
#define SCANCODE_W     0x11
#define SCANCODE_A     0x1E
#define SCANCODE_S     0x1F
#define SCANCODE_D     0x20

// 函数声明
void simulateHardwareKeyPress(BYTE scancode);
void attachToRobloxWindow();
void forceForegroundWindow(HWND hwnd);
void simulateMouseMovement();

HWND robloxHwnd = NULL;
DWORD robloxThreadId = 0;

int main() {
	// 设置控制台为UTF-8
	SetConsoleOutputCP(CP_UTF8);
	
	// 初始化随机数
	srand(GetTickCount());
	
	std::cout << "Roblox Hardware Input Simulator" << std::endl;
	std::cout << "================================" << std::endl;
	
	// 查找并附加到Roblox窗口
	robloxHwnd = FindWindow(NULL, "Roblox");
	if (!robloxHwnd) {
		std::cout << "Looking for Roblox window..." << std::endl;
		// 尝试查找活动窗口
		robloxHwnd = GetForegroundWindow();
	}
	
	if (robloxHwnd) {
		robloxThreadId = GetWindowThreadProcessId(robloxHwnd, NULL);
		attachToRobloxWindow();
		forceForegroundWindow(robloxHwnd);
		std::cout << "Attached to window: " << robloxHwnd << std::endl;
	} else {
		std::cout << "Please focus Roblox window and press any key..." << std::endl;
		std::cin.get();
		robloxHwnd = GetForegroundWindow();
		robloxThreadId = GetWindowThreadProcessId(robloxHwnd, NULL);
		attachToRobloxWindow();
	}
	
	// 设置间隔
	int interval;
	std::cout << "\nEnter interval (seconds): ";
	std::cin >> interval;
	
	std::cout << "\nRunning - Press Ctrl+C to stop\n" << std::endl;
	
	// 按键扫描码列表
	BYTE scancodes[] = {SCANCODE_SPACE, SCANCODE_W, SCANCODE_A, SCANCODE_S, SCANCODE_D};
	const char* keynames[] = {"SPACE", "W", "A", "S", "D"};
	const int keycount = sizeof(scancodes)/sizeof(scancodes[0]);
	
	try {
		while (true) {
			// 确保窗口激活
			if (GetForegroundWindow() != robloxHwnd) {
				forceForegroundWindow(robloxHwnd);
				attachToRobloxWindow();
			}
			
			// 随机选择按键
			int randomKey = rand() % keycount;
			BYTE scancode = scancodes[randomKey];
			
			// 硬件级按键模拟
			simulateHardwareKeyPress(scancode);
			
			// 输出信息
			std::cout << "Pressed: " << keynames[randomKey] << std::endl;
			
			// 模拟鼠标移动防止AFK检测
			if (rand() % 3 == 0) {
				simulateMouseMovement();
			}
			
			// 随机间隔
			int delay = interval * 1000 + (rand() % 3000 - 1500);
			if (delay < 1000) delay = 1000;
			
			// 分段等待
			for (int i = 0; i < 5; i++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(delay/5));
			}
		}
	}
	catch (...) {
		std::cout << "\nStopped" << std::endl;
	}
	
	return 0;
}

// 硬件级扫描码模拟（游戏专用）
void simulateHardwareKeyPress(BYTE scancode) {
	INPUT input = {0};
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = scancode;          // 使用扫描码
	input.ki.dwFlags = KEYEVENTF_SCANCODE; // 硬件扫描码标志
	
	// 按下
	SendInput(1, &input, sizeof(INPUT));
	std::this_thread::sleep_for(std::chrono::milliseconds(80 + rand() % 40));
	
	// 释放
	input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
}

// 附加到目标窗口的输入队列
void attachToRobloxWindow() {
	if (robloxHwnd && robloxThreadId) {
		DWORD currentThreadId = GetCurrentThreadId();
		AttachThreadInput(currentThreadId, robloxThreadId, TRUE);
	}
}

// 强制窗口到前台（绕过系统限制）
void forceForegroundWindow(HWND hwnd) {
	if (!hwnd) return;
	
	// 特殊方法强制激活窗口
	DWORD currentPID = GetCurrentProcessId();
	DWORD windowPID = 0;
	GetWindowThreadProcessId(GetForegroundWindow(), &windowPID);
	
	if (windowPID != currentPID) {
		AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(), NULL), 
			GetCurrentThreadId(), TRUE);
	}
	
	ShowWindow(hwnd, SW_RESTORE);
	SetForegroundWindow(hwnd);
	
	if (windowPID != currentPID) {
		AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(), NULL),
			GetCurrentThreadId(), FALSE);
	}
}

// 模拟真实鼠标移动
void simulateMouseMovement() {
	// 获取窗口位置
	RECT rect;
	GetWindowRect(robloxHwnd, &rect);
	
	// 窗口中心位置
	int centerX = rect.left + (rect.right - rect.left) / 2;
	int centerY = rect.top + (rect.bottom - rect.top) / 2;
	
	// 随机小范围移动
	int moveX = centerX + (rand() % 50 - 25);
	int moveY = centerY + (rand() % 50 - 25);
	
	// 设置鼠标位置并小幅度移动
	SetCursorPos(moveX, moveY);
	
	// 微小移动
	mouse_event(MOUSEEVENTF_MOVE, rand() % 10 - 5, rand() % 10 - 5, 0, 0);
}
