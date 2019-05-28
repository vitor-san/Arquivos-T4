#include "manipulaIndice.h"
#include <stdlib.h>
#include <string.h>
#include "sort.h"

//TODO: busca binaria modificada (provavelmente com funcao de comparacao generica e vai no sort.c), inserir, remover e atualizar registros em RAM
//TODO: checar fim de pagina de disco na insercao

typedef struct {
    ListaOrd[26] alfabeto;
} Estrutura;

int busca_binaria(int* v, int chave, int ini, int fim) {

    // 0 - caso base (busca sem sucesso)
    if (ini > fim) return FAIL;

    // 1 - calcula ponto central e verifica se chave foi encontrada
    int centro = (int)((ini+fim)/2.0);
    if (v[centro] == chave)
        return centro;

    // 2 - chamada recursiva para metade do espaco de busca
    if (chave < v[centro])
        // se chave eh menor, fim passa ser o centro-1
        return busca_binaria(v, chave, ini, centro-1);

    if (chave > v[centro])
        // se a chave eh maior, inicio passa ser centro+1
        return busca_binaria(v, chave, centro+1, fim);
    
}


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

    //registro->chaveBusca = NULL;
    registro->byteOffset = 0;

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
    Checa se o registro a ser inserido cabe
    na pagina de disco atual do arquivo. Caso
    nao caiba, a funcao ira completar a pagina
    atual com lixo, modificar o indicador de
    tamanho do ultimo registro da pagina (para
    que ele contabilize o lixo como parte dele)
    e abrir uma nova pagina de disco.

    Parametros:
        FILE *file - arquivo binario considerado
        regDados *registro - registro a ser inserido
        int tamAntigo - tamanho do ultimo registro
    inserido no arquivo
*/
//TODO: modificar para indice
void checaFimPaginaIndice(FILE *file, regDadosI *registro, int tamAntigo) {
    if ( (ftell(file)%TAMPAG + 128) > TAMPAG ) { //se nao ha espaco suficiente... (+5 por conta do campo "removido" e do indicador de tamanho, que nao sao contabilizados no tamanho do registro)
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

    for (int i = 0; i < n; i++) {
        if (vetorRAM[i].chaveBusca[0] == '\0') continue;    //o registro esta "logicamente removido" em RAM
        insereRegistroIndice(file, vetorRAM+i); //insiro o registro no arquivo
    }

    fseek(file, 0, SEEK_SET);
    fputc('1', file);   //ja que terminei de escreve no arquivo, seu status vira '1'
}

//TODO: fazer um vetor[26] de Listas Ordenadas, que ordenam pelo Byte Offset.
