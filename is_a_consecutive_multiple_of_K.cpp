#include <iostream>
#include <string>

bool IsKPeriodic(const std::string& str, int K)
{
    int n = str.length();

    // Проверка на некорректные значения K
    if (K <= 0 || n % K != 0) {
        return false;
    }

    int subLength = n / K; // Длина каждой подстроки

    // Сравнение первой подстроки с остальными подстроками
    for (int i = 1; i < K; i++) {
        std::string sub = str.substr(i * subLength, subLength);
        if (sub != str.substr(0, subLength)) {
            return false;
        }
    }

    return true;
}

int main()
{
    std::string str = "abcabcabcabc";
    int K = 4;

    if (IsKPeriodic(str, K)) {
        std::cout << "String is k-periodic" << std::endl;
    } else {
        std::cout << "String is not k-periodic" << std::endl;
    }

    return 0;
}
