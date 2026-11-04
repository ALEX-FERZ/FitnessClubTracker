#include <cctype>
#include <ctime>

#include "services/Services.h"

namespace DateTime {

namespace {
std::string format(const char* pattern) {
    std::time_t t = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), pattern, &local);
    return buf;
}

bool digitsAt(const std::string& s, std::size_t from, std::size_t count) {
    for (std::size_t i = from; i < from + count; ++i)
        if (i >= s.size() || !std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    return true;
}

int num(const std::string& s, std::size_t from, std::size_t count) {
    return std::stoi(s.substr(from, count));
}

bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

int daysInMonth(int y, int m) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && isLeap(y)) ? 29 : days[m - 1];
}

// Количество дней от 1970-01-01 (алгоритм days_from_civil).
long long daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long long>(doe) - 719468;
}
}  // namespace

std::string now() { return format("%Y-%m-%d %H:%M"); }
std::string today() { return format("%Y-%m-%d"); }

bool isValidDate(const std::string& s) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    if (!digitsAt(s, 0, 4) || !digitsAt(s, 5, 2) || !digitsAt(s, 8, 2)) return false;
    int y = num(s, 0, 4), m = num(s, 5, 2), d = num(s, 8, 2);
    return y >= 2000 && y <= 2100 && m >= 1 && m <= 12 && d >= 1 && d <= daysInMonth(y, m);
}

bool isValidDateTime(const std::string& s) {
    if (s.size() != 16 || s[10] != ' ' || s[13] != ':') return false;
    if (!isValidDate(s.substr(0, 10)) || !digitsAt(s, 11, 2) || !digitsAt(s, 14, 2)) return false;
    return num(s, 11, 2) < 24 && num(s, 14, 2) < 60;
}

long long toMinutes(const std::string& dt) {
    long long days = daysFromCivil(num(dt, 0, 4), num(dt, 5, 2), num(dt, 8, 2));
    int h = dt.size() >= 16 ? num(dt, 11, 2) : 0;
    int mi = dt.size() >= 16 ? num(dt, 14, 2) : 0;
    return days * 24 * 60 + h * 60 + mi;
}

}  // namespace DateTime
