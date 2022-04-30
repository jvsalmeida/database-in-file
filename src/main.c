#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../lib/cs50.h"
#include "../lib/btree.h"

int numInsert = 0;


// Ordem das informacoes: chave primaria(5), nome(30), marca(30), descricao(111), ano(4), preco(6), @`s(6)
// primaria: 3 primeiras do marca e 2 primeiras do preco
void insertData(BTree *tree, char *nome,  char *marca, char *preco, char *descricao, char *ano) {

	char *chavePrimaria = (char*)malloc(sizeof(char) * 6);

	strncat(chavePrimaria, nome, 3);
	strncat(chavePrimaria, ano, 2);

	FILE *file = fopen("produtos.dat", "a");
	if(file == NULL) {
		free(chavePrimaria);
		return;
	}

	char *asterisco = (char*)malloc(sizeof(char) * 193);
	char *lixo = (char*)malloc(sizeof(char) * 193);
	for(int w = 0; w < 192; ++w) {
		lixo[ w ] = '*';
	}
	lixo[ 192 ] = '\0';

	strncat(asterisco, chavePrimaria, 5);
	strncat(asterisco, "@", 2);	

	strncat(asterisco, nome, 30);
	strncat(asterisco, "@", 2);	

	strncat(asterisco, marca, 30);
	strncat(asterisco, "@", 2);	

	strncat(asterisco, descricao, 111);
	strncat(asterisco, "@", 2);	

	strncat(asterisco, ano, 4);
	strncat(asterisco, "@", 2);	

	strncat(asterisco, preco, 6);
	strncat(asterisco, "@", 2);

	strncat(asterisco, lixo, 192 - strlen(asterisco));
	asterisco[ 192 ] = '\0';

	fseek(file, 0, SEEK_SET);
	fwrite(asterisco , sizeof(char), 192, file);
	fclose(file);
	
	tree->root = insert(tree, chavePrimaria, numInsert++);

	printf("CHAVE PRIMÁRIA: %s\n", chavePrimaria);

	free(chavePrimaria);
	free(asterisco);
}


void atualizar(int offset) {
	FILE *file = fopen("produtos.dat", "r+");
	if(file == NULL)
		return;

	char *newPreco;
	newPreco = get_string("Digite o novo preço: ");
	fseek(file, 192 * offset, SEEK_SET);

	char *str = (char*)malloc(sizeof(char) * 193);
	
	char *newStr = (char*)malloc(sizeof(char) * 193);
	fread(newStr, 1, 192, file);

	char *pch = strtok (newStr, "@");
	strcat(str, pch);
	strncat(str, "@", 2);;

	for(int i = 0; i < 4; ++i) {
  		pch = strtok(NULL, "@");
  		strcat(str, pch);
  		strncat(str, "@", 2);
	}
	strncat(str, newPreco, 6);
  	strncat(str, "@", 2);

  	char *lixo = (char*)malloc(sizeof(char) * 193);
	for(int w = 0; w < 192; ++w) {
		lixo[ w ] = '*';
	}
	lixo[ 192 ] = '\0';

  	strncat(str, lixo, 192 - strlen(str));

	newStr[ 192 ] = '\0';

	fseek(file, 192 * offset, SEEK_SET);
	fwrite(str , sizeof(char), 192, file);
	fclose(file);

	free(str);
	free(newStr);
	free(lixo);
}


char *dados[] = {"chavePrimaria", "nome", "marca", "descricao", "ano", "preco"};
void impressao(int offset) {
	FILE *file = fopen("produtos.dat", "r+");
	if(file == NULL)
		return;

	fseek(file, 192 * offset, SEEK_SET);

	char *str = (char*)malloc(sizeof(char) * 193);
	fread(str, 1, 192, file);
	char *pch = strtok (str, "@");

	if(pch[0] == '#')
		return;

	for(int i = 0; i < 6; ++i) {
		printf("%s: %s\n", dados[i], pch);
  		pch = strtok(NULL, "@");
	}

	fclose(file);
	free(str);
}


void delecao(BTree *tree, char *primary, int offset) {
	FILE *file = fopen("produtos.dat", "r+");
	if(file == NULL)
		return;

	fseek(file, 192 * offset, SEEK_SET);

	char *str = (char*)malloc(sizeof(char) * 193);
	fread(str, 1, 192, file);

	str[ 0 ] = '#';
	str[ 1 ] = '-';

	fseek(file, 192 * offset, SEEK_SET);
	fwrite(str , sizeof(char), 192, file);
	

	fclose(file);
	free(str);

	delete(tree, primary);
}


void arquivoIndice(NODE *newNode) {
	int i = 0;
	if (newNode) {
		if(newNode->link[ 0 ] == NULL) {
			FILE *file = fopen("ibtree.idx", "a+");
			if(file == NULL)
				return;
			
			fprintf(file, "-1");

			for (i = 0; i < newNode->count; ++i)
				fprintf(file, " <%s, %d> -1", newNode->key[ i ], newNode->rrn[ i ]);
			
			while( (i++) < NODE_MAX )
				fprintf(file, " <*****, -1> -1");

			fprintf(file, " ");
			fclose(file);

		} else {
			for (i = 0; i < NODE_MAX; ++i) {
				arquivoIndice(newNode->link[ i ]);

				if(i == 0) {
					FILE *file = fopen("ibtree.idx", "a+");
					if(file == NULL)
						return;

					if(newNode->link[ i ] == NULL) 
						fprintf(file, "-1");
					else
						fprintf(file, "%d", i);
					
					int j;
					for (j = 0; j < newNode->count; ++j)
						fprintf(file, " <%s, %d> %d", newNode->key[ j ], newNode->rrn[ j ], j + 1);
			
					while( (j++) < NODE_MAX )
						fprintf(file, " <*****, -1> -1");

					fprintf(file, " ");					
					fclose(file);
				}
			}
			arquivoIndice(newNode->link[ i ]);
		}
	}
}


// Ordem das informacoes: chave primaria(5), nome(30), marca(30), descricao(111), ano(4), preco(6), @`s(6)
void console(BTree *tree, bool *loop) {
	char *nome;			// = (char*)malloc(sizeof(char) * 31);
	char *marca;		// = (char*)malloc(sizeof(char) * 31);
	char *descricao;	// = (char*)malloc(sizeof(char) * 112);
	char *ano;			// = (char*)malloc(sizeof(char) * 5);
	char *preco;		// = (char*)malloc(sizeof(char) * 7);

	char *chave;

	int comando;
	printf("---------------------------------------------------------\n");
	comando = get_int("Digite:\n1 para Inserir um produto no catálogo\n2 para Atualizar o preco um produto do catálogo a partir da chave primária\n3 para Buscar um produto a partir da Chave primária\n4 para Listar os produtos do catálogo\n5 para Remover um produto do catálogo\n6 para Sair\n");

	//Insercao
	if(comando == 1) {    
		nome = get_string("Digite o Nome do produto: ");
		marca = get_string("Digite o Marca do produto: ");
		preco = get_string("Digite o Preco do produto: ");
		descricao = get_string("Digite o Descricao do produto: ");
		ano = get_string("Digite o Ano do produto: ");

		insertData(tree, nome, marca, preco, descricao, ano);	
	}

	//Atualizacao do preco
	else if(comando == 2) { 
		chave = get_string("Digite a chave primária do produto: ");
		
		RESULT *res = search(tree->root, chave);
		int offset = res->rrn;

		if(res->found) {
			atualizar(offset);
		} else {
			printf("Esta chave não existe!\n");
		}

 	}

	//Busca
	else if(comando == 3) { 
		chave = get_string("Digite a chave primária do produto: ");
		
		RESULT *res = search(tree->root, chave);
		int offset = res->rrn;

		if(res->found) {
			impressao(offset);
		} else {
			printf("Not found\n");
		}

	}

	//Lista de produtos
	else if(comando == 4) { 
		printBTree(tree->root);	
	}

	//Delecao
	else if(comando == 5) { 
		chave = get_string("Digite a chave primária do produto: ");
		
		RESULT *res = search(tree->root, chave);
		int offset = res->rrn;

		if(res->found) {
			delecao(tree, chave, offset);
		} else {
			printf("Not found\n");
		}
	}

	//Sair
	else if(comando == 6) { 
		*loop = false;
	}

	return;
}


int main() {
	BTree *tree = createBTree();

	FILE *file = fopen("produtos.dat", "w");
	fclose(file);

	FILE *pFile = fopen("ibtree.idx", "w");
	fclose(pFile);

	bool loop = true;
	while(loop)
		console(tree, &loop);

	arquivoIndice(tree->root);

	return 0;
}