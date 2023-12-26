#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define N 3

void det(int **matrix, int num, int p[2]){
    int determinant;
    close(p[0]);
    if(num == 0){
        determinant = matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1];
    } else if (num == 1){
        determinant = matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0];
    } else {
        determinant = matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0];
    }
    write(p[1], &determinant, sizeof(int));
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

    fill_matrix(A);
    printf("Сгенерирована матрица:\n");
    print_matrix(A);

    int fds[N][2];
    for (size_t i = 0; i < N; i++) {
        if (pipe(fds[i]) == -1) {
            perror("Ошибка вызова pipe");
            return 1;
        }
    }

    pid_t pids[N];
    int dets[N];

    for (int i = 0; i < N; ++i) {
        if ((pids[i] = fork()) < 0) {
            perror("Ошибка создания дочернего процесса.");
            abort();
        } else if (pids[i] == 0) {
            det(A, i, fds[i]);
            return 0;
        } else {
            // Закрываем write для родителя
            close(fds[i][1]);
            // читаем из pipe значения детерминантов подматриц
            read(fds[i][0], &dets[i], sizeof(int));

        }
    }
    // Ждём завершения дочерних процессов
    for(int i = 0; i < N; i++) waitpid(pids[i], NULL, 0);

    int det_total = 0;
    int pos = 1;
    for(int i = 0; i < N; i++){
        det_total += A[0][i] * dets[i] * pos;
        pos *= -1;
    }

    printf("Детерминант матрицы равен: %d\n", det_total);
    return 0;
}