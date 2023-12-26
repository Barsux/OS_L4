#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define N 3

void det(int **matrix, int num, int * shmem){
    int determinant;

    if(num == 0){
        determinant = matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1];
    } else if (num == 1){
        determinant = matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0];
    } else {
        determinant = matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0];
    }
    shmem[num] = determinant;
}

void fill_matrix(int **matrix) {
    srand(time(NULL));
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand() % 10;
        }
    }
}

void print_matrix(int **matrix) {
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main() {
    // Выделение памяти под массив матрицы
    int **A = (int **)malloc(N * sizeof(int *));
    for (size_t i = 0; i < N; i++) {
        A[i] = (int *)malloc(N * sizeof(int));
    }

    //Генерируем рандомную 3x3 матрицу
    fill_matrix(A);
    printf("Сгенерирована матрица:\n");
    //Выводим на экран
    print_matrix(A);

    //Выделяем общую память, размером как три int, или же N*sizeof(int)
    int * shmem = mmap(NULL, sizeof(int) * N, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //Инициализируем N доч. процессов
    pid_t pids[N];

    //Запускаем N доч. процессов
    for (int i = 0; i < N; ++i) {
        if ((pids[i] = fork()) < 0) {
            perror("Ошибка создания дочернего процесса.");
            abort();
        } else if (pids[i] == 0) {
            det(A, i, shmem);
            return 0;
        } else {
        }
    }
    for(int i = 0; i < N; i++) waitpid(pids[i], NULL, 0);

    //Считаем из полученных определителей подматриц детерминант матрицы
    int det_total = 0;
    int pos = 1;
    for(int i = 0; i < N; i++){
        det_total += A[0][i] * shmem[i] * pos;
        pos *= -1;
    }
    printf("Детерминант матрицы равен: %d\n", det_total);
    return 0;
}