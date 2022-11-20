#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>

#pragma warning(disable : 4996)

int main(int argc, char** argv) {

	int n,		// N�mero de elementos do vetor de entrada (com 0s)
		m,		// N�mero de elementos do vetor de sa�da (sem 0s)
		* vIn,	// Vetor de entrada de n elementos
		i;
	FILE* arqIn,	// Arquivo texto de entrada
		* arqOut;	// Arquivo texto de sa�da

  // -------------------------------------------------------------------------
  // Inicializa��o

	// Initialize the MPI environment
	MPI_Init(&argc, &argv);

	// Get the rank of the process
	int pId, pNumber;
	m = 0;
	MPI_Comm_rank(MPI_COMM_WORLD, &pId);
	MPI_Comm_size(MPI_COMM_WORLD, &pNumber);

	if (argc != 3)
	{
		if (pId == 0) {
			printf("O programa foi executado com argumentos incorretos.\n");
			printf("Uso: ./remove0_seq arquivo_entrada arquivo_saida\n");
		}
		MPI_Finalize();
		exit(1);
	}

	if (pId == 0) {
		// Abre arquivo de entrada
		arqIn = fopen(argv[1], "rt");

		if (arqIn == NULL)
		{
			if (pId == 0) {
				printf("\nArquivo texto de entrada n�o encontrado\n");
			}
			MPI_Finalize();
			exit(1);
		}

		// L� tamanho do vetor de entrada
		fscanf(arqIn, "%d", &n);

		// Aloca vetor de entrada
		vIn = (int*)malloc(n * sizeof(int));
		if (vIn == NULL)
		{
			if (pId == 0) {
				printf("\nErro na aloca��o de estruturas\n");
			}
			MPI_Finalize();
			exit(1);
		}

		// L� vetor do arquivo de entrada
		for (i = 0; i < n; i++)
		{
			fscanf(arqIn, "%d", &(vIn[i]));
		}

		// Fecha arquivo de entrada
		fclose(arqIn);
	}

	// -------------------------------------------------------------------------
	// Corpo principal do programa



	// Remove 0s do vetor de entrada, produzindo vetor de sa�da
	if (pId == 0) {
		// TODO: Medir tempo inicial
		std::string result = "";
		
		//envia os elementos a serem processados por cada processo
		for (i = 1; i < pNumber; i++)
		{
			//envia os elementos a serem processados
			//TODO
		}

		//recebe os resultados
		for (i = 1; i < pNumber; i++)
		{
			char msg[100];
			// Recebe mensagem do processo i com os valores do vetor
			//MPI_Recv(msg, , MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);
			//result = result + std::string(msg);

			// Recebe mensagem do processo i com o valor de m
			int aux;
			MPI_Recv(&aux, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("%d ", i);
			m += aux;

			// TODO: Medir tempo final
			// TODO: calcular tempo e printar
			//printf("Tempo=%.2fms\n", tempo);
		}

		// -------------------------------------------------------------------------
		// Escrita no arquivo de saida
		// Cria arquivo de sa�da
		arqOut = fopen(argv[2], "wt");

		// Escreve tamanho do vetor de sa�da
		fprintf(arqOut, "%d\n", m);

		// Escreve vetor do arquivo de sa�da
		for (i = 0; i < m; i++)
		{
			fprintf(arqOut, result.c_str());
		}
		fprintf(arqOut, "\n");

		// Fecha arquivo de sa�da
		fclose(arqOut);

		printf("%d\n", n);
	}
	else {
		//allocate memory for arrays
		vIn = (int*)malloc((1+(n/pNumber))* sizeof(int));
		if (vIn == NULL)
		{
			printf("\nErro na aloca��o de memoria no processo %d\n", pId);
			MPI_Finalize();
			exit(1);
		}
		//set all values to 0
		for (int i = 0; i < 1 + (n / pNumber); i++) {
			vIn[i]=0;
		}

		//recebe os valores do processo 0
		// TODO

		//processa a entrada
		std::string numbers = "";
		for (int i = 0; i < 1 + (n / pNumber); i++) {
			if (vIn[i] =! 0) {
				//adicionar valor ao resultado
				numbers = numbers + std::to_string(vIn[i]) + " ";
				//incrementar contador
				m++;
			}
		}
		//envia mensagem para o processo 0 contendo os elementos a serem escritos na saida
		//MPI_Send(numbers.c_str(), strlen(numbers.c_str()) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		//envia mensagem contendo o valor de m a ser somado
		MPI_Send(&m, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

	}

	// -------------------------------------------------------------------------
	// Finaliza��o

	// Libera vetores de entrada
	free(vIn);
	// Finalize the MPI environment.
	MPI_Finalize();
}