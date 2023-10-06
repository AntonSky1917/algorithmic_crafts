#include <iostream>
#include <vector>
#include <future>

const int MAX_THREADS = 4;
std::mutex mtx; // мьютекс для доступа
int active_threads = 0; // счетчик активных потоков

void merge(int* arr, int l, int m, int r) {
    int nl = m - l + 1;
    int nr = r - m;

    // создаем временные массивы
    int left[nl], right[nr];

    // копируем данные во временные массивы
    for (int i = 0; i < nl; i++)
        left[i] = arr[l + i];
    for (int j = 0; j < nr; j++)
        right[j] = arr[m + 1 + j];

    int i = 0, j = 0;
    int k = l;  // начало левой части

    while (i < nl && j < nr) {
        // записываем минимальные элементы обратно во входной массив
        if (left[i] <= right[j]) {
            arr[k] = left[i];
            i++;
        }
        else {
            arr[k] = right[j];
            j++;
        }
        k++;
    }
    // записываем оставшиеся элементы левой части
    while (i < nl) {
        arr[k] = left[i];
        i++;
        k++;
    }
    // записываем оставшиеся элементы правой части
    while (j < nr) {
        arr[k] = right[j];
        j++;
        k++;
    }
}

// функция для сортировки слиянием
void mergeSort(int* arr, int l, int r) {
    if (l < r) { // проверям, можно ли разделить массив на более мелкие части
        int m = l + (r - l) / 2; // вычисление средней позиции в массиве для разделения

        // вектор для слежки за потоками
        std::vector<std::future<void>> futures; // ограничиваем колличество одновременно запускаемых потоков
        if (active_threads < MAX_THREADS) { // проверяем, если количество потоков не превышено, то
            futures.push_back(std::async(std::launch::async, [arr, l, m]() { // запускаем

                // далее блок кода счетчика активных потоков
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    active_threads++; // либо увеличиваем счетчик активных потоков
                }
                mergeSort(arr, l, m); // рекурсия для левой части массива
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    active_threads--; // либо уменьшаем
                }
            }));
        } else {
            mergeSort(arr, l, m); // если макс.кол.пот превышено то, выполняем сортировку последовательно
        }

        mergeSort(arr, m + 1, r); // рекурсия для правой части массива

        // ожидание завершения всех запущенных потоков
        for (auto& future : futures) {
            future.get();
        }

        // слияние двух подмассивов
        merge(arr, l, m, r);
    }
}

int main() {
    int arr[] = {12, 11, 13, 5, 6, 7, 1, 4, 3, 2, 8, 9, 10};
    int arr_size = sizeof(arr) / sizeof(arr[0]);

    mergeSort(arr, 0, arr_size - 1);

    std::cout << "Отсортированный массив: \n";
    for (int i = 0; i < arr_size; i++) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}
