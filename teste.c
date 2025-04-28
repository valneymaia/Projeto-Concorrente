#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define THRESHOLD 1000  // Limite mínimo para paralelizar

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quicksort_sequential(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort_sequential(arr, low, pi - 1);
        quicksort_sequential(arr, pi + 1, high);
    }
}

// Função auxiliar: gera vetor aleatório
void gerarVetorAleatorio(int *arr, int n, int max_valor) {
    srand(42); // Semente fixa para resultados reproduzíveis
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % (max_valor + 1);
    }
}

// QuickSort MPI (com envio e recebimento de dados)
void quicksort_mpi(int *arr, int n, int rank, int size) {
    if (n <= THRESHOLD || size == 1) {
        quicksort_sequential(arr, 0, n - 1);
        return;
    }

    int pi = partition(arr, 0, n - 1);

    int left_size = pi;
    int right_size = n - pi - 1;

    int partner = rank + size / 2;

    if (partner < size) {
        // Envia a metade direita para o parceiro
        MPI_Send(&right_size, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
        MPI_Send(&arr[pi + 1], right_size, MPI_INT, partner, 0, MPI_COMM_WORLD);

        // Ordena a metade esquerda
        quicksort_mpi(arr, left_size, rank, size / 2);

        // Recebe o lado direito de volta
        MPI_Recv(&arr[pi + 1], right_size, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
        // Não há parceiro, faz sequencialmente
        quicksort_sequential(arr, 0, n - 1);
    }
}

int main(int argc, char *argv[]) {
    int *vet = NULL;
    int n = 1000000;  // Tamanho do vetor
    int max_valor = 1000000;
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time, end_time;

    if (rank == 0) {
        vet = (int *)malloc(n * sizeof(int));
        if (vet == NULL) {
            printf("Erro ao alocar memória!\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        gerarVetorAleatorio(vet, n, max_valor);

        start_time = MPI_Wtime();
    }

    // Distribui o vetor para o processo 0
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        vet = (int *)malloc(n * sizeof(int));
    }

    MPI_Bcast(vet, n, MPI_INT, 0, MPI_COMM_WORLD);

    quicksort_mpi(vet, n, rank, size);

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Tempo de execução (MPI): %.6f segundos\n", end_time - start_time);

        free(vet);
    } else {
        free(vet);
    }

    MPI_Finalize();
    return 0;
}
