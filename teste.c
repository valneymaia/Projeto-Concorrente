#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
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
    return i + 1;
}

void quicksort(int *arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

void gerarVetorAleatorio(int *arr, int n, int max_valor) {
    srand(42);  // Semente fixa para consistência
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % (max_valor + 1);
    }
}

int main(int argc, char *argv[]) {
    int *data = NULL;
    int *local_data;
    int n = 100000;  // Tamanho do vetor principal
    int max_valor = 1000000;

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start_time = 0, end_time;

    int local_n = n / size;

    // Alocação do vetor local
    local_data = (int *)malloc(local_n * sizeof(int));
    if (local_data == NULL) {
        printf("Erro na alocação local.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0) {
        data = (int *)malloc(n * sizeof(int));
        if (data == NULL) {
            printf("Erro na alocação global.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        gerarVetorAleatorio(data, n, max_valor);
        start_time = MPI_Wtime();
    }

    // Distribui partes do vetor para cada processo
    MPI_Scatter(data, local_n, MPI_INT, local_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Cada processo ordena sua parte
    quicksort(local_data, 0, local_n - 1);

    // Recolhe as partes ordenadas no processo 0
    MPI_Gather(local_data, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Tempo de execução MPI (parcialmente ordenado): %.6f segundos\n", end_time - start_time);

        // Observação: neste ponto, o vetor ainda não está totalmente ordenado globalmente
        // pois cada processo ordenou apenas sua parte local
        free(data);
    }

    free(local_data);
    MPI_Finalize();
    return 0;
}
