#include "manipulaIndice.h"
#include <stdlib.h>
#include <string.h>
#include "sort.h"
#include "listaOrdenada.h"

//TODO: busca binaria modificada (provavelmente com funcao de comparacao generica e vai no sort.c), inserir, remover e atualizar registros em RAM

struct velo {   //vetor estatico de listas ordenadas
    ListaOrd[26] alfabeto;
};

long long* buscaRegistroIndice(regDadosI *v, char* chave, int ini, int fim, int* comeco) {

    // 0 - caso base (busca sem sucesso)
    if (ini > fim) return NULL;

    // 1 - calcula ponto central e verifica se chave foi encontrada
    int centro = (int)((ini+fim)/2.0);

    if (!strcmp(v[centro].chaveBusca,chave)) {

        long long* retorno = malloc(sizeof(long long));

        int pos = 0;
        retorno[pos] = v[centro].byteOffset;

        int prox = centro + 1;
        int ant = centro - 1;

        while(!strcmp(v[ant].chaveBusca,chave)) {
            pos++;
            retorno = realloc(retorno,sizeof(long long)*(pos));
            retorno[pos] = v[ant].byteOffset;
            ant--;
        }

        *comeco = pos-1;

        while(!strcmp(v[prox].chaveBusca,chave)) {
            pos++;
            retorno = realloc(retorno,sizeof(long long)*(pos));
            retorno[pos] = v[prox].byteOffset;
            prox++;
        }


        return retorno;

    }

    // 2 - chamada recursiva para metade do espaco de busca
    if (strcmp(chave,v[centro].chaveBusca) < 0)
        // se chave eh menor, fim passa ser o centro-1
        return busca_binaria(v, chave, ini, centro-1);

    if (strcmp(chave,v[centro].chaveBusca) > 0)
        // se a chave eh maior, inicio passa ser centro+1
        return busca_binaria(v, chave, centro+1, fim);

}


/*
    Cria um novo registro de cabecalho ZERADO.
    CUIDADO: a funcao NAO o coloca no arquivo.
    Para tanto, utilize a funcao insereCabecalho().

    Retorno:
        regCabec - registro de cabecalho inicializado
    com valores padrao
*/
regCabecI *criaCabecalhoIndice() {
    regCabecI *cabecalho = malloc(sizeof(regCabecI));

    cabecalho->status = '0'; //ao se estar escrevendo em um arquivo, seu status deve ser 0
    cabecalho->nroRegistros = 0;

    return cabecalho;
}

/*
    Cria um novo registro de dados ZERADO.
    CUIDADO: a funcao NAO o coloca no arquivo.
    Para tanto, utilize a funcao insereRegistro().

    Retorno:
        regDados - registro de dados inicializado
    com valores padrao
*/
regDadosI *criaRegistroIndice() {
    regDadosI *registro = malloc(sizeof(regDadosI));

    registro->chaveBusca = NULL;
    registro->byteOffset = -1;

    return registro;
}

/*
    Le os campos do registro de cabecalho
    e guarda seus valores em uma estrutura
    passada por parametro pelo usuario.

    Parametros:
        FILE *file - arquivo binario contendo o
    registro de cabecalho a ser lido
        regCabec *cabecalho - estrutura para onde
    devem ser copiados os dados lidos
*/
void leCabecalhoIndice(FILE *file, regCabecI *cabecalho) {
    long origin = ftell(file);    //guardo a posicao de inicio do registro
    fseek(file, 0, SEEK_SET);   //vou para o inicio do arquivo

    cabecalho->status = fgetc(file);
    fread(&(cabecalho->nroRegistros), sizeof(int), 1, file);

    fseek(file, origin, SEEK_SET);     //volto ao inicio do registro
}

/*
    Le os campos de um registro presente em um
    arquivo binario anteriormente gerado por
    este programa e guarda seus valores em uma
    estrutura passada por parametro pelo usuario.
    A funcao assume que o ponteiro de leitura do
    arquivo estara no inicio do registro ao ser
    chamada.

    Parametros:
        FILE *file - arquivo binario contendo o
    registro a ser lido
        regDadosI *registro - estrutura para onde
    devem ser copiados os dados do registro
*/
void leRegistroIndice(FILE *file, regDadosI *registro) {
    fread(registro->chaveBusca, 1, 120, file);
    fread(&(registro->byteOffset), 8, 1, file);
}

/*
    Funcao que insere o registro de cabecalho
    no arquivo binario passado como parametro.

    Parametros:
        FILE *file - arquivo binario a ser modificado
        regCabec *cabecalho - cabecalho a ser gravado
*/
void insereCabecalhoIndice(FILE *file, regCabecI *cabecalho) {
    long origin = ftell(file);
    fseek(file, 0, SEEK_SET);   //vou para o inicio do arquivo

    fwrite(&(cabecalho->status), 1, 1, file);  //escrevo o campo "status" no arquivo binario
    fwrite(&(cabecalho->nroRegistros), sizeof(int), 1, file);  //escrevo o campo "topoLista" no arquivo binario

    fseek(file, origin, SEEK_SET);
}

/*
    Funcao que insere um registro de dados
    na posicao atual do ponteiro de escrita
    do arquivo de indices.

    Parametros:
        FILE *file - arquivo de indices a ser
    modificado
        regDados *registro - registro de dados
    a ser gravado
*/
void insereRegistroIndice(FILE *file, regDadosI *registro) {
    fwrite(&(registro->chaveBusca), 1, 120, file);
    fwrite(&(registro->byteOffset), 8, 1, file);
}

/*
    Checa se mais um registro pode ser inserido
    na pagina de disco atual do arquivo. Caso
    nao caiba, a funcao ira completar a pagina
    atual com lixo e posicionar o ponteiro de
    escrita no inicio da nova pagina.

    Parametros:
        FILE *file - arquivo binario de indices
*/
void checaFimPaginaIndice(FILE *file) {
    if ( (ftell(file)%TAMPAG + 128) > TAMPAG ) { //se nao ha espaco suficiente...
        int diff = TAMPAG - ftell(file)%TAMPAG;  //quantidade necessaria de lixo para completar a pagina de disco
        for (int i = 0; i < diff; i++) fputc('@', file);  //completo com lixo
    }
}

/*
    Cria um vetor (alocado dinamicamente) que
    tera todos os registros de dados do arquivo
    de indice, para sua posterior manipulacao
    na memoria principal. Sera utilizado para
    fazer as buscas, ja que o custo sera da
    ordem log(n) (por conta da busca binaria).
*/
regDadosI *carregaIndiceVetor(FILE *file) {
    fseek(file, 0, SEEK_END);
    long tam = ftell(file);
    fseek(file, TAMPAG, SEEK_SET);  //vou para o inicio dos registros de dados
    tam -= TAMPAG;
    regDadosI *vetor = calloc(tam, sizeof(regDadosI));
    fread(vetor, 1, tam, file);
    return vetor;
}

/*
    Funcao de comparacao entre dois registros
    de dados do arquivo de indices.
*/
int compare(void *reg1, void *reg2) {
    regDadosI *r = (regDadosI *)reg1;
    regDadosI *s = (regDadosI *)reg2;

    int comp = strcmp(r->chaveBusca, s->chaveBusca);

    if (comp == 0) {
        long long byteOff1 = r->byteOffset;
        long long byteOff2 = s->byteOffset;
        return (r > s) - (r < s);
    }
    else return comp;
}

/*
    Funcao para imprimir um registro de
    dados do arquivo de indices.
*/
void printRegistroIndice(void *reg) {
    regDadosI print = *((regDadosI *)reg);
    printf("Chave de busca: %s\n", print.chaveBusca);
    printf("Byte offset: %lld\n\n", print.byteOffset);
}

/*
    Funcao para liberar memoria, anteriormente
    alocada pelo indice.
*/
void freeRegistroIndice(void *reg) {
    free(reg);
}

/*

*/
SuperLista criaSuperLista() {
    SuperLista nova = malloc(sizeof(SL));

    for (int i = 0; i < 26; i++) {
        nova->alfabeto[i] = criaListaOrd(compare, freeRegistroIndice, printRegistroIndice);
    }



    return nova;
}

/*

*/
SuperLista carregaIndiceLista(FILE *file) {
    regDadosI reg = criaRegistroIndice();

    fseek(file, TAMPAG, SEEK_SET);  //vou para o inicio dos registros de dados

    byte b = fgetc(file);

    while (!feof(file)) {
        ungetc(b, file);    //"devolvo" o byte lido
        leRegistroIndice(file, reg);



        b = fgetc(file);
    }
}



/*
    Reescreve o arquivo de indices, atualizando-o
    de acordo com as modificacoes feitas em RAM.

    Parametros:
        FILE *file - arquivo em que sera realizada
    a escrita
        regDadosI *vetorRAM - vetor que contem os
    registros modificados
*/
void reescreveArquivoIndice(FILE *file, regDadosI *vetorRAM) {
    int n = sizeof(vetorRAM)/sizeof(regDadosI); //numero de registros presentes no vetor

    fseek(file, 0, SEEK_SET);
    fputc('0', file);   //como estou escrevendo em um arquivo, seu status deve ser '0' ate que a escrita acabe

    for (int i = 0; i < n; i++) insereRegistroIndice(file, vetorRAM+i); //insiro o registro no arquivo

    fseek(file, 0, SEEK_SET);
    fputc('1', file);   //ja que terminei de escreve no arquivo, seu status vira '1'
}

//TODO: fazer um vetor[26] de Listas Ordenadas, que ordenam pelo Byte Offset.
