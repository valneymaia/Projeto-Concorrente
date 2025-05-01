#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>  // Incluir OpenMP para paralelização

#define THRESHOLD 1000  // Limite mínimo para paralelismo


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

// QuickSort Sequencial
void quicksort_sequential(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort_sequential(arr, low, pi - 1);
        quicksort_sequential(arr, pi + 1, high);
    }
}

// QuickSort Paralelo com OpenMP
void quicksort_parallel(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        if ((high - low) > THRESHOLD) {  
            #pragma omp parallel num_threads(4)
            {
                #pragma omp single nowait
                {
                    #pragma omp task
                    quicksort_parallel(arr, low, pi - 1);

                    #pragma omp task
                    quicksort_parallel(arr, pi + 1, high);

                    #pragma omp taskwait  // Garante que todas as tarefas terminem
                }
            }
        } else {
            quicksort_sequential(arr, low, high);  
        }
    }
}

// Função para imprimir o vetor
void printArray(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void gerarVetorAleatorio(int arr[], int n, int max_valor) {
    srand(time(NULL));

    for (int i = 0; i < n; i++) {
        arr[i] = rand() % (max_valor + 1);
    }
}

int main() {

    int a = 1000000;  // Teste com tamanho grande para notar diferença
    int *vet_seq = (int *)malloc(sizeof(int) * a);
    int *vet_par = (int *)malloc(sizeof(int) * a);
    int m = a;

    // Verifica se a alocação de memória foi bem-sucedida
    if (vet_seq == NULL || vet_par == NULL) {
        printf("Erro ao alocar memória!\n");
        return 1;
    }

    srand(time(NULL));

    // Preenchendo os vetores com números aleatórios dentro de um intervalo menor
    for (int i = 0; i < a; i++) {
        int num = rand() % 1000000;  // Evita números muito grandes
        vet_seq[i] = num;
        vet_par[i] = num; // Cópia para manter os mesmos valores nos dois testes
    }


      // Execução Sequencial
    double start_seq = omp_get_wtime();
    quicksort_sequential(vet_seq, 0, m - 1);
    double end_seq = omp_get_wtime();
    printf("Tempo de execução (Sequencial): %.6f segundos\n", end_seq - start_seq);

    // Execução Paralela
    double start_par = omp_get_wtime();
    quicksort_parallel(vet_par, 0, m - 1);
    double end_par = omp_get_wtime();
    printf("Tempo de execução (Paralelo): %.6f segundos\n", end_par - start_par);

    // Liberação de memória
    free(vet_seq);
    free(vet_par);


    return 0;
}





