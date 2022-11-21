/*
Felipe Munoz Mazur
Leopoldo Santos Silva

Infelizmente nao conseguimos resolver um erro que acontece no programa.
Ainda assim, esperamos que a logica utilizada tenha algum fundamento e possa servir para algo.
Obrigado.
*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string>


int main(int argc, char** argv) {

	int n,		// Número de elementos do vetor de entrada (com 0s)
		m,		// Número de elementos do vetor de saída (sem 0s)
		* vIn,	// Vetor de entrada de n elementos
		* VOut,
		i;
	FILE* arqIn,	// Arquivo texto de entrada
		* arqOut;	// Arquivo texto de saída

	/*MPI TAGS:
	0 - valor de n
	1 - valor de m a ser somado no processo 0
	2 - numeros na saida
	3 - tamanho da cadeia de caracteres na saida (para alocar no processo 0)
	*/

  // -------------------------------------------------------------------------
  // Inicialização

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
				printf("\nArquivo texto de entrada não encontrado\n");
			}
			MPI_Finalize();
			exit(1);
		}

		// Lê tamanho do vetor de entrada
		fscanf(arqIn, "%d", &n);

		// Aloca vetor de entrada
		vIn = (int*)malloc(n * sizeof(int));
		if (vIn == NULL)
		{
			if (pId == 0) {
				printf("\nErro na alocação de estruturas\n");
			}
			MPI_Finalize();
			exit(1);
		}

		// Lê vetor do arquivo de entrada
		for (i = 0; i < n; i++)
		{
			fscanf(arqIn, "%d", &(vIn[i]));
		}

		// Fecha arquivo de entrada
		fclose(arqIn);
	}

	// -------------------------------------------------------------------------
	// Corpo principal do programa

	if (pId == 0) {
		// Mede instante de tempo inicial
		struct timeval tIni, tFim;
		gettimeofday(&tIni, 0);

		//envia n para os outros processos fazerem os calculos
		for (int i = 1; i < pNumber; i++) {
			MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			printf("\nenviado");
		}
	}
	else {//recebe o valor de n do processo 0
		MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("\nrecebido");
	}


	//trecho baseado no codigo disponivel em https://gist.github.com/ehamberg/1263868/cae1d85dee821d45fb0cd58747aaf33370f3f1ed
	int sum = 0;
	int rem = n % (pNumber - 1);
	//envia os elementos a serem processados por cada processo
	int* sendcounts = (int*)malloc(sizeof(int) * pNumber);
	int* displs = (int*)malloc(sizeof(int) * pNumber);
	if (sendcounts == NULL || displs == NULL) {
		printf("\nErro na alocação de memoria no processo %d\n", pId);
		MPI_Finalize();
		exit(1);
	}

	int* recvBuf;

	//calcula sendcounts
	sendcounts[0] = 0;
	for (i = 1; i < pNumber; i++)
	{
		sendcounts[i] = n / (pNumber - 1);
		if (rem > 0) {
			sendcounts[i]++;
			rem--;
		}
		displs[i] = sum;
		sum += sendcounts[i];
		recvBuf = (int*)malloc(sizeof(int) * sendcounts[i]);
		if (recvBuf == NULL) {
			printf("\nErro na alocação de memoria para recvBuf no processo %d\n", pId);
		}
	}
	// distribui os elementos entre os processos
	MPI_Scatterv(&vIn, sendcounts, displs, MPI_INT, &recvBuf, NULL, MPI_CHAR, 0, MPI_COMM_WORLD);

	// processa os valores
	if (pId != 0) {

		//processa a entrada
		std::string numbers = "";
		for (int i = 0; i < sendcounts[pId]; i++) {
			if (recvBuf[i] = !0) {
				//adicionar valor ao resultado
				numbers = numbers + std::to_string(vIn[i]) + " ";
				//incrementar contador
				m++;
			}
		}
		//envia mensagem contendo o valor de m a ser somado
		MPI_Send(&m, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
		//envia mensagem contendo o tamanho da cadeia de caracteres
		int charSize = strlen(numbers.c_str());
		MPI_Send(&charSize, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
		//envia mensagem para o processo 0 contendo os elementos a serem escritos na saida
		MPI_Send(numbers.c_str(), strlen(numbers.c_str()) + 1, MPI_CHAR, 0, 2, MPI_COMM_WORLD);
	}
	else { //coleta os resultados no processo 0 e escreve no arquivo de saida
		std::string result = "";
		//recebe os valores de m
		for (i = 1; i < pNumber; i++)
		{
			// Recebe mensagem do processo i com o valor de m
			int aux;
			MPI_Recv(&aux, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			m += aux;
			// Recebe mensagem do processo i com o tamanho da cadeia de caracteres
			MPI_Recv(&aux, 1, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// Aloca memoria e recebe a cadeia a ser escrita
			char* msg = (char*)malloc(sizeof(char) * aux);
			MPI_Recv(msg, aux + 1, MPI_CHAR, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			result = result + std::string(msg);
			free(msg);
		}

		// Mede instante de tempo final
		gettimeofday(&tFim, 0);
		// Tempo de execução em milissegundos
		long segundos = tFim.tv_sec - tIni.tv_sec;
		long microsegundos = tFim.tv_usec - tIni.tv_usec;
		double tempo = (segundos * 1e3) + (microsegundos * 1e-3);
		printf("Tempo=%.2fms\n", tempo);

		// -------------------------------------------------------------------------
		// Escrita no arquivo de saida
		// Cria arquivo de saída
		arqOut = fopen(argv[2], "wt");

		// Escreve tamanho do vetor de saída
		fprintf(arqOut, "%d\n", m);

		// Escreve vetor do arquivo de saída
		for (i = 0; i < m; i++)
		{
			fprintf(arqOut, result.c_str());
		}
		fprintf(arqOut, "\n");

		// Fecha arquivo de saída
		fclose(arqOut);

		printf("%d\n", n);

		// Libera vetores de entrada
		free(vIn);
	}


	// Finalize the MPI environment.
	free(sendcounts);
	free(displs);
	MPI_Finalize();
}