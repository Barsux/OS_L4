#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define N 3
#define PORT 3386

typedef struct {
    int num;
    int det;
}pckt;


int buffer_size;

int det(int **matrix, int num){
    int determinant;

    if(num == 0){
        determinant = matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1];
    } else if (num == 1){
        determinant = matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0];
    } else {
        determinant = matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0];
    }
    return determinant;
}
//Функция для отправки информации с дочернего процесса серверу по протоколу UDP
void send_pckt(pckt packet){
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Ошибка вызова socket");
        return;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sendto(sock, &packet, buffer_size, 0, (struct sockaddr *)&addr, sizeof(addr));
    close(sock);
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
    buffer_size = sizeof(pckt);

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

    //Инициализируем UDP-сервер
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Ошибка вызова socket");
        return 1;
    }
    //UDP сервер будет принимать клиентов со всех адресов
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("Ошибка вызова bind");
        return 1;
    }

    //Инициализируем N доч. процессов
    pid_t pids[N];
    int dets[N];
    int numRecv = 0;

    //Запускаем N доч. процессов
    for (int i = 0; i < N; ++i) {
        if ((pids[i] = fork()) < 0) {
            perror("Ошибка создания дочернего процесса.");
            abort();
        } else if (pids[i] == 0) {
            pckt pack;
            pack.num = i;
            pack.det = det(A, i);
            send_pckt(pack);
            return 0;
        } else {
        }
    }
    // Сервер ждёт прихода трёх пакетов, в одном потоке, хоть и recvfrom запирающая.
    while(numRecv < 3){
        pckt p;
        recvfrom(sock, &p, buffer_size, 0, NULL, NULL);
        printf("Получен пакет с номером %d\n", p.num);
        dets[p.num] = p.det;
        numRecv++;
    }
    // Ждём завершения дочерних процессов
    for(int i = 0; i < N; i++) waitpid(pids[i], NULL, 0);
    close(sock);

    //Считаем из полученных определителей подматриц детерминант матрицы
    int det_total = 0;
    int pos = 1;
    for(int i = 0; i < N; i++){
        det_total += A[0][i] * dets[i] * pos;
        pos *= -1;
    }
    printf("Детерминант матрицы равен: %d\n", det_total);
    return 0;
}