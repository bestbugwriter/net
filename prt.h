/********************************************************
*@brief: 定义调试打印宏
********************************************************/
#define DEBUG_MSG 3

#if 1==DEBUG_MSG	//普通打印
	#define PRT printf
#elif 2==DEBUG_MSG	//打印文件名、行号
	#define PRT(fmt,...) printf("[%s][%d]: "fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#elif 3==DEBUG_MSG	//打印文件名、行号、函数名
	#define PRT(fmt,...) printf("[%s][%s][%d]: "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
	#define PRT
#endif //DEBUG_MSG
