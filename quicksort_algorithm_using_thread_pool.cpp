#include <iostream>
#include <vector>
#include <future>
#include <algorithm>
#include <thread>
#include <exception>
#include <stdexcept>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

// класс пула потоков для управления потоками
class ThreadPool {
public:
    // создаем и инициализируем пул потоков и определяем его функцию
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) { // создаем заданное количество потоков
            // пишем лямбдо-выражение код которого будет выполнятся в каждом потоке
            threads.emplace_back([this] { // создаем новый поток
                while (true) { // цикл будет выполнятся до тех пор пока пул потоков не будет остановлен
                    std::function<void()> task; // объявляем функцию которая будет хранить задачу для выполнения
                    {   
                        // блок который будет использоваться для захвата мьютекса
                        std::unique_lock<std::mutex> lock(queueMutex); // мьютекс для синхронизации доступа к очереди задач
                        // поток будет ждать пока появится задача или не будет установлен флаг stop в true т.е. остановки пула
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) { // если флаг в true и очередь задач пуста то завершаем выполнение и выходим из цикла
                            return;
                        }
                        task = std::move(tasks.front()); // извлекаем задачу из очереди задач переместив ее и сохраняем ее
                        tasks.pop(); // удаляем задачу из очереди
                    }
                    try {
                        task(); // выполняем задачу которую извлекли из очереди
                    } catch (const std::exception& e) {
                        std::cerr << "Ошибка потока: " << e.what() << std::endl;
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all(); // сигнал о событии для потока condition
        for (std::thread& thread : threads) {
            thread.join(); // дожидаемся завершения всех потоков
        }
    }

    // объявляем шаблонную функцию enqueue - это std::future который представляет будущий результат выполнения функции func.
    template <typename Func>
    // функция позволяет добавлять задачи в очередь пула потоков для их асинхронного выполнения
    auto enqueue(Func&& func) -> std::future<decltype(func())> {
        // делаем обертку над функцией func что-бы его асинхронно выполнить и получить результат
        // общий указатель: std::make_shared<std::packaged_task позволит передать
        // обертку(func) в другой поток и сохранить ее(func) в памяти до завершения выполнения задачи
        auto task = std::make_shared<std::packaged_task<decltype(func())()>>(std::forward<Func>(func));
        std::future<decltype(func())> result = task->get_future(); // создаем будущий результат выполнения func
        {
            std::unique_lock<std::mutex> lock(queueMutex); // захват мьютекса
            if (stop) { // если пул потоков остановлен
                throw std::runtime_error("поставить в очередь остановленный ThreadPool");
            }
            tasks.emplace([task]() { (*task)(); }); // добавляем функцию(задачу) в очередь
        }
        condition.notify_one(); // сигнал для одного из потоков о наличии новой задачи в очереди
        return result; // возвращаем объект std::future как будущий результат выполнения
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

// функция разделения массива
template <typename T>
int partition(std::vector<T>& array, int left, int right) {
    T pivot = array[left];
    int i = left - 1;
    int j = right + 1;
    while (true) {
        do {
            i++;
        } while (array[i] < pivot);
        do {
            j--;
        } while (array[j] > pivot);
        if (i >= j) {
            return j; // возврат индекса опорного элемента
        }
        std::swap(array[i], array[j]); // меняем элементы местами
    }
}

// функция быстрой сортировки
template <typename T>
std::future<void> quicksort(std::vector<T>& array, int left, int right, ThreadPool& pool) {
    if (left < right) {
        int pivot = partition(array, left, right); // находим опорный элемент
        if (right - left > 10000) { // если размер левой части массива больше порогового значения, то...
            // асинхронно вызываем рекурсию для правой части сортировки
            auto right_future = pool.enqueue([&array, left, right, &pool]() {
                quicksort(array, left, right, pool);
            });

            quicksort(array, pivot + 1, right, pool); // сортируем левую часть синхронно
            right_future.wait(); // // ожидаем завершения сортировки правой части
            return right_future; // возвращаем фьючерс как сигнал о завершении сортировки правой части
        } else { // иначе, если размер левой части массива меньше порогового значения, то...
            // сортируем обе части синхронно
            quicksort(array, left, pivot, pool);
            quicksort(array, pivot + 1, right, pool);

            // возвращаем завершенный фьючерс так как правая часть сортируется синхронно
            return std::async(std::launch::deferred, [] {});
        }
    }
    return std::async(std::launch::deferred, [] {}); // возвращаем завершенный фьючерс
}

int main() {
    std::vector<int> array = {3, 6, 1, 8, 2, 7, 4, 5, 1, 8, 6, 3, 4};
    ThreadPool pool(std::thread::hardware_concurrency()); // hardware_concurrency() для того что-бы использовать ядра процессора

    try {
        auto start_time = std::chrono::high_resolution_clock::now(); // начальное время

        auto result_future = quicksort(array, 0, array.size() - 1, pool); // запуск сортировки
        result_future.wait(); // ждем завершения всей сортировки

        auto end_time = std::chrono::high_resolution_clock::now(); // конечное время
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        std::cout << "Время выполнения сортировки: " << duration.count() << " миллисекунд" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Исключение в main: " << e.what() << std::endl;
    }

    for (int num : array) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}
