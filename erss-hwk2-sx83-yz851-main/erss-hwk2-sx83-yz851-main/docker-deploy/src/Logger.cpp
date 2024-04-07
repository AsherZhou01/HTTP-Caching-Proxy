#include <system_error>
#include <pthread.h>
#include "Logger.hpp"

std::ofstream file;
pthread_mutex_t thread_mutex;

std::string formatCurrentTime()
{
    // 获取当前时间，返回的是一个std::time_t类型的时间戳，表示从1970年1月1日00:00:00起至当前时间的总秒数。
    std::time_t currentTime = std::time(0);
    // 将时间戳转换为本地时间，得到一个std::tm类型的结构体指针，该结构体包含了年、月、日等时间成分
    struct std::tm *timeinfo = std::localtime(&currentTime);
    // 大小设为80，格式化后的字符串长度一般不会超过这个值
    std::vector<char> buffer(80);
    // 将std::tm类型的时间格式化为字符串，格式为"%a %b %d %H:%M:%S %Y"
    std::strftime(&buffer[0], buffer.size(), "%a %b %d %H:%M:%S %Y", timeinfo);
    // 返回格式化后的时间字符串
    return std::string(&buffer[0]);
}
void writeLog(int id, std::string message)
{
    std::ostringstream logStream;
    logStream << id << ":" << message << std::endl;
    std::string messageStr = logStream.str();
    // std::cout << "HERE" << endl;

    pthread_mutex_lock(&thread_mutex);
    // 将构造好的日志消息通过std::ofstream file写入到文件中，并立即调用file.flush()方法确保消息被立即写入文件
    file << messageStr;
    file.flush();
    pthread_mutex_unlock(&thread_mutex);
}