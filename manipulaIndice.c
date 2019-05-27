#include "manipulaIndice.h"
#include <stdlib.h>
#include <string.h>

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
        regDados *registro - estrutura para onde
    devem ser copiados os dados do registro
*/
void leRegistroIndice(FILE *file, regDadosI *registro) {
    long origin = ftell(file);    //guardo a posicao de inicio do registro

    fread(registro->chaveBusca, sizeof(char), 120, file);
    fread(&(registro->byteOffset), sizeof(long long), 1, file);

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
    na memoria principal.
*/
regDadosI *carregaIndiceRAM(FILE *file) {
    fseek(file, 0, SEEK_END);
    long tam = ftell(file);
    fseek(file, TAMPAG, SEEK_SET);  //vou para o inicio dos registros de dados
    tam -= TAMPAG;
    regDadosI *vetor = calloc(tam, sizeof(regDadosI));
    fread(vetor, 1, tam, file);
    return vetor;
}
