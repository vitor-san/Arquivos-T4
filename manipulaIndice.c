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

    registro->chaveBusca[0] = '\0';
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

    fputc(cabecalho->status, file);  //escrevo o campo "status" no arquivo binario
    fwrite(&(cabecalho->nroRegistros), sizeof(int), 1, file);  //escrevo o campo "topoLista" no arquivo binario
    for (int i = 5; i < TAMPAG; i++) fputc('@', file);  //completo a pagina de disco com lixo

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
    int qtdChars = strlen(registro->chaveBusca)+1;
    fwrite(&(registro->chaveBusca), 1, qtdChars, file);
    for (int i = 0; i < 120-qtdChars; i++) fputc('@', file);   //completo com lixo
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

    Parametros:
        FILE *file - arquivo de indices
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
    Essa funcao busca por um registro no arquivo de
    indices pelo nomeServidor, e retorna o byteOffset
    do arquivo de dados dos mesmos. Caso exista mais
    do que um registro com o nome buscado, sera
    retornado um vetor de byte offsets, ordenados em
    funcao do valor do byte offset.

    Parametros:
        regDadosI *v - vetor com o arquivo de indice em RAM
        char* chave - chave de busca
        int ini - comeco do vetor
        int fim - fim do vetor
        int* comeco - comeco do vetor de retorno
        int* tam - tamanho do vetor de retorno
    Retorno:
        long long* - vetor com os byteOffsets dos registro
    do arquivo de dados
*/
long long* buscaRegistroIndice(regDadosI *v, char* chave, int ini, int fim, int* comeco,int* tam) {

    // 0 - caso base (busca sem sucesso)
    if (ini > fim) return NULL;

    // 1 - calcula ponto central e verifica se chave foi encontrada
    int centro = (int)((ini+fim)/2.0);

    if (!strcmp(v[centro].chaveBusca,chave)) {

        long long* retorno = malloc(sizeof(long long));

        int pos = 0;
        retorno[pos] = v[centro].byteOffset;
        pos++;

        //busca os registros depois do encontrado
        int prox = centro + 1;
        //busca os registros antes do encontrado
        int ant = centro - 1;

        while(!strcmp(v[ant].chaveBusca,chave)) {
            pos++;
            retorno = realloc(retorno,sizeof(long long)*(pos));
            retorno[pos-1] = v[ant].byteOffset;
            ant--;
        }

        //marca a posicao do ultimo registro encontrado]
        //antes do primeiro, ou seja menores que ele
        *comeco = pos-1;

        while(!strcmp(v[prox].chaveBusca,chave)) {
            pos++;
            retorno = realloc(retorno,sizeof(long long)*(pos));
            retorno[pos-1] = v[prox].byteOffset;
            prox++;
        }

        //guarda o tamanho do vetor
        *tam = pos;

        return retorno;
    }

    // 2 - chamada recursiva para metade do espaco de busca
    if (strcmp(chave,v[centro].chaveBusca) < 0)
        // se chave eh menor, fim passa ser o centro-1
        return buscaRegistroIndice(v, chave, ini, centro-1, comeco,tam);

    if (strcmp(chave,v[centro].chaveBusca) > 0)
        // se a chave eh maior, inicio passa ser centro+1
        return buscaRegistroIndice(v, chave, centro+1, fim, comeco,tam);
}

/*
    Instancia uma estrutura de dados que
    armazenara todos os registros de dados
    do arquivo de indice, para sua posterior
    manipulacao na memoria principal.
    Sera utilizada para fazer as adicoes e
    remocoes, ja que o custo sera da ordem
    de n, porem com uma constante muito baixa.

    Parametros:
        FILE *file - arquivo de indices
*/
SuperLista carregaIndiceLista(FILE *file) {
    regDadosI *reg = criaRegistroIndice();
    SuperLista sl = criaSuperLista();

    fseek(file, TAMPAG, SEEK_SET);  //vou para o inicio dos registros de dados

    byte b = fgetc(file);

    while (!feof(file)) {
        ungetc(b, file);    //"devolvo" o byte lido
        if (b == '@') { //se esta no final de uma pagina de disco
            int pulo = TAMPAG - (ftell(file)%TAMPAG);
            fseek(file, pulo, SEEK_CUR);
        }
        else {
            leRegistroIndice(file, reg);
            adicionaSuperLista(sl, reg);
        }
        b = fgetc(file);
    }

    free(reg);
    return sl;
}

/*
    Reescreve o arquivo de indices, atualizando-o
    de acordo com as modificacoes feitas em memoria
    principal (por meio da SuperLista).

    Parametros:
        FILE *file - arquivo em que sera realizada
    a escrita
        regCabecI *cabec - novo cabecalho do arquivo
        SuperLista base - estrutura a ser utilizada
    como base para a escrita dos registros de dados
    do arquivo de indice
*/
void reescreveArquivoIndice(FILE *file, regCabecI *cabec, SuperLista base) {
    insereCabecalhoIndice(file, cabec); //geralmente, vem com status '0'
    fseek(file, TAMPAG, SEEK_SET);   //vou para a segunda pagina de disco

    regDadosI *regAtual;
    for (int i = 0; i < 26; i++) {
        while (!vaziaListaOrd(base->alfabeto[i])) {
            //pego o primeiro elemento presente na lista
            regAtual = primeiroListaOrd(base->alfabeto[i]);
            insereRegistroIndice(file, regAtual);
            free(regAtual);
        }
    }

    fseek(file, 0, SEEK_SET);   //vou para o comeco do arquivo
    fputc('1', file);   //coloco seu status para '1'
}
