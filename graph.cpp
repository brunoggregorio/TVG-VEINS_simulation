#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Vector.h"

typedef struct {
	int code;		//node[0]
	char label[5];	//nic #0
	int posX;		//startPos: (100.000,100.000,1.000)
	int posY;		//startPos: (100.000,100.000,1.000)
} node;

typedef struct {
	char vert1[5];
	char vert2[5];
} edge;

///
/// Functions
///
int nodeByLabel(Vector<node *> *vertices, char label[]);
int updateNode(Vector<node *> *vertices, node a);
int updateLabelNode(Vector<node *> *vertices, node a);
int removeNodeByLabel(Vector<node *> *vertices, Vector<edge *> *arestas, char label[]);
void removeEdgeByLabel(Vector<edge *> *arestas, edge *ed);
void removeAllEdgeByLabel(Vector<edge *> *arestas, char label[]);
int createGraphFile(Vector<node *> *vertices, Vector<edge *> *arestas, float startTime);

///
///
///
int main(int argc, char *argv[]) {
	Vector<edge *> *arestas = new Vector<edge *>();
	Vector<node *> *vertices = new Vector<node *>();
	float startTime = 0.0;
	node no;
	
	FILE *logFile;
	logFile = fopen("debug-0.elog", "r+");

	if(!logFile) {
		printf("Erro na leitura do arquivo de log 'Debug-0.elog'.");
		return 1;
	}

	//
	// Loop de leitura do log
	//
	while( !feof(logFile) ) { // enquanto arquivo de log nao terminar
		char word[4000];
		fscanf(logFile, " %[^\n]", word);
//printf("%s ", word);

		//
		// Procura por substring relacionada a um novo grafo
		//	"]::TraCIMobility: checkIfOutside, outside=0 borderStep: (0,0,0)"
		char *mobility = strstr(word,"]::TraCIMobility: checkIfOutside, outside=0 borderStep: (0,0,0)");
		if( mobility ) {
//printf("%ld; %s\n", until-word+1, word);

			// calcula parte da string que define o numero do node
			int gap = (mobility-word-7);

			// salva o node a ser processado em uma string
			char strNum[10];
			strncpy(strNum, word+7, gap); 	// pega somente a regiao da string relacionado ao numero do node
			strNum[gap] = '\0';

			// faz um cast do numero do node para int
			no.code = atoi(strNum);
//printf("PRIMEIRO NODE: %d\n", no.code);

			//
			// Lendo proxima linha que contem o startPos do node e o startTime
			//
			fscanf(logFile, " %[^\n]", word);

			// verifica se o startTime ainda eh o mesmo
			gap = (int) ( strstr(word, " speed:") - strstr(word, "startTime: ")-11 );
			strncpy(strNum, strstr(word, "startTime: ")+11, gap);
			strNum[gap] = '\0';

			// verifica se o startTime foi alterado,
			// 		se verdadeiro salva arquivo de grafo .gt
			// 		se falso continua com a atualizacao dos nodes
			if( startTime != atof(strNum) )
				createGraphFile(vertices, arestas, startTime);

			// faz um cast do startTime para float
			startTime = atof(strNum);
//printf("TIME: %.2f\n", startTime);

			//
			// Calcula a parte da string que define a posicao x,y do node
			//
			int wordPos = (int) (strstr(word, "startPos: (") - word + 11);
			int i = 0;
			
			// posicao x
			while(word[wordPos+i] != '.') {
				strNum[i] = word[wordPos+i];
				i++;
			}
			strNum[i] = '\0';
			no.posX = atoi(strNum);
//printf("%s\nX: %d\n", word, no.posX);

			// pula decimais (desnecessarios)
			while(word[wordPos+i] != ',') {
				i++;
			}
			wordPos = wordPos+i+1;	// atualiza valor da posicao na word
			i = 0;					// atualiza o valor de i para gravar em strNum

			// calcula y
			while(word[wordPos+i] != '.') {
				strNum[i] = word[wordPos+i];
				i++;
			}
			strNum[i] = '\0';
			no.posY = atoi(strNum);
//printf("Y: %d\n", no.posY);
//printf("NO pos code: %d\n", no.code);
			
			// atualiza o node no vetor de vertices
			updateNode(vertices, no);
		} else {

			//
			// Procura por substring relacionada ao registro de um novo node (so vai add label)
			//	"- connectionManager:  registering nic #"
			char *registering = strstr(word,"- connectionManager:  registering nic #");
			if( registering ) {

				int i=0;
				int gap = strlen(word) - 39;
				for(i=0; i<gap; i++) {
					no.label[i] = word[39+i];
				}
				no.label[i] = '\0';
//printf("%s\n%s\n", word, no.label);
//printf("NO label code: %d\n", no.code);
				// atualiza o node no vetor de vertices
				updateLabelNode(vertices, no);
			} else {

				//
				// Procura por substring relacionada a exclusao de um node (excluir pelo label 
				//								pq pode ter mais de um no mesmo loop de tempo)
				//	"- connectionManager:  unregistering nic #"
				char *unregistering = strstr(word,"- connectionManager:  unregistering nic #");
				if( unregistering ) {

					int gap = strlen(word) - 41;
					char label[5];
					for(int i=0; i<gap; i++) {
						label[i] = word[41+i];
					}
					label[gap] = '\0';
//printf("%s\nnic#%s\n", word, label);

					// exclui o node do vetor de vertices e todos seus edges pelo label
					removeNodeByLabel(vertices, arestas, label);
				} else {

					//
					// Procura por substring relacionada a conexao de novos edges
					//	"- NicEntry: connecting nic #"	" and #"
					char *connecting = strstr(word, "- NicEntry: connecting nic #");
					if( connecting ) {

						int gap;
						edge *ed = new edge();

						// pega label do primeiro node da conexao
						gap = (int) (strstr(word, " and #") - connecting - 28);
						for(int i=0; i<gap; i++) {
							ed->vert1[i] = word[28+i];
						}
						ed->vert1[gap] = '\0';

						// pega label do segundo node da conexao
						gap = (int) strlen(word) - (strstr(word, " and #") - word) - 6;
						for(int i=0; i<gap; i++) {
							ed->vert2[i] = word[(strstr(word, " and #") - word + 6) + i];
						}
						ed->vert2[gap] = '\0';
//printf("%s\nConnecting nic#%s and nic#%s\n", word, ed->vert1, ed->vert2);

						// registra novo edge no vetor de arestas
						arestas->add(ed);
					} else {

						//
						// Procura por substring relacionada a exclusao de edges
						//	"- NicEntry: disconnecting nic #"	" and #"
						char *disconnecting = strstr(word, "- NicEntry: disconnecting nic #");
						if( disconnecting ) {

							int gap;
							edge *ed = new edge();

							// pega label do primeiro node
							gap = (int) (strstr(word, " and #") - disconnecting - 31);
							for(int i=0; i<gap; i++) {
								ed->vert1[i] = word[31+i];
							}
							ed->vert1[gap] = '\0';

							// pega label do segundo node
							gap = (int) strlen(word) - (strstr(word, " and #") - word) - 6;
							for(int i=0; i<gap; i++) {
								ed->vert2[i] = word[(strstr(word, " and #") - word + 6) + i];
							}
							ed->vert2[gap] = '\0';
//printf("%s\nDisconnecting nic#%s and nic#%s\n", word, ed->vert1, ed->vert2);

							// remove o edge do vetor de arestas
							removeEdgeByLabel(arestas, ed);
						}
					}
				}
			}
		}
	}

	fclose(logFile);

	return 0;
}

///
/// Retorna a posicao de um node no vetor de vertices a partir de seu label
///
int nodeByLabel(Vector<node *> *vertices, char label[]) {
	for(int i=0; i < vertices->getSize(); i++) {
		node *no = vertices->get(i);
		if( !strcmp(no->label, label) )
			return i;
	}

	return -1;
}

///
/// Atualiza a posicao do node se ele ja existir, se nao cria-o
///
int updateNode(Vector<node *> *vertices, node a) {
	//
	// Verifica se ja existe no vetor
	//
	for(int i=0; i < vertices->getSize(); i++) {
		// se existir atualiza sua posicao
		node *b = vertices->get(i);
		if( a.code == b->code) {
			b->posX = a.posX;
			b->posY = a.posY;

			return 0;
		}
	}
	
	// se nao existe cria-se uma instancia
	node *no = new node();
		no->code = a.code;
		no->posX = a.posX;
		no->posY = a.posY;
	vertices->add(no);
//printf("ADD node: %d, LABEL: %s, POS: (%d,%d)\n", no->code, no->label, no->posX, no->posY);
	return 0;
}

///
///
///
int updateLabelNode(Vector<node *> *vertices, node a) {
	//
	// Verifica se ja existe no vetor
	//
	for(int i=0; i < vertices->getSize(); i++) {
		// se existir atualiza sua posicao
		node *b = vertices->get(i);
		if( a.code == b->code) {
			strcpy(b->label, a.label);
//printf("ATUALIZOU Label: %s\n", b->label);
			return 0;
		}
	}

	printf("Registro node=%d nao encontrado! Label nao atualizado!", a.code);

	return -1;
}

///
/// Remove um vertice e suas respectivas arestas do grafo a partir de um code
///
int removeNodeByLabel(Vector<node *> *vertices, Vector<edge *> *arestas, char label[]) {
	int index = nodeByLabel(vertices, label);

	if( index != -1 ) {
		vertices->remove(index);
		//removeAllEdgeByLabel(arestas, label);

		return 0;
	} else {
		printf("Registro nic#%s nao encontrado!\n", label);

		return -1;
	}
}

///
/// Remove aresta a partir dos labels
///
void removeEdgeByLabel(Vector<edge *> *arestas, edge *ed) {
	int count=0;

	for(int i=0; i < arestas->getSize(); i++) {
		edge *aresta = arestas->get(i);
		if( !strcmp(aresta->vert1, ed->vert1) && !strcmp(aresta->vert2, ed->vert2) ) {//VERIFICAR O IF
			arestas->remove(i);
			count++;
		}
	}

	if(count == 0)
		printf("Aresta nic#%s e nic#%s nao encontrado!\n", ed->vert1, ed->vert2);
}

///
/// Remove todas as arestas de um vertice a partir de seu label 			DESUSO
///
void removeAllEdgeByLabel(Vector<edge *> *arestas, char label[]) {
	for(int i=0; i < arestas->getSize(); i++) {
		edge *aresta = arestas->get(i);
		if( !strcmp(aresta->vert1, label) || !strcmp(aresta->vert2, label) ) //VERIFICAR O IF
			arestas->remove(i);
	}
}

///
/// Cria arquivo .gt com o grafo a cada intervalo de tempo
///
int createGraphFile(Vector<node *> *vertices, Vector<edge *> *arestas, float startTime) {
	
	// startTime inicial nao possui nodes para gerar grafo
	if(startTime == 0.0)
		return 0;

//printf("\n**************************************************************************\n");
//printf("\t\t\tARQUIVO %f", startTime);
//printf("\n**************************************************************************\n");
	char fileName[30];
	sprintf(fileName, "graph_%.2f.gt", startTime);

	FILE *f;
	if( !(f=fopen(fileName, "wr")) ) {
		printf("Falha ao criar arquivo de grafo: %s!\n", fileName);
	}

	fprintf(f, "\ninfo {\n\tcreator = \"GraphThing 1.3.2\"\n}\n\n");

	for(int i=0; i < vertices->getSize(); i++) {
		node *no = vertices->get(i);
		fprintf(f, "vertex \"%s\" at (%d,%d)\n", no->label, no->posX, no->posY);
	}

	fprintf(f, "\n");
	for(int i=0; i < arestas->getSize(); i++) {
		edge *ed = arestas->get(i);
		fprintf(f, "edge \"%s\" -- \"%s\"\n", ed->vert1, ed->vert2);
	}

	fclose(f);

	return 0;
}