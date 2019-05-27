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
regCabec *criaCabecalho() {
    regCabec *cabecalho = malloc(sizeof(regCabec));

    cabecalho->status = '0'; //ao se estar escrevendo em um arquivo, seu status deve ser 0
    cabecalho->topoLista = -1L; //a lista de removidos esta vazia

    cabecalho->tagCampo[0] = 'i';
    cabecalho->desCampo[0][0] = '\0';

    cabecalho->tagCampo[1] = 's';
    cabecalho->desCampo[1][0] = '\0';

    cabecalho->tagCampo[2] = 't';
    cabecalho->desCampo[2][0] = '\0';

    cabecalho->tagCampo[3] = 'n';
    cabecalho->desCampo[3][0] = '\0';

    cabecalho->tagCampo[4] = 'c';
    cabecalho->desCampo[4][0] = '\0';

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
regDados *criaRegistro() {
    regDados *registro = malloc(sizeof(regDados));

    registro->removido = '-';  //os registros vem "nao-removidos" por padrao
    registro->encadeamentoLista = -1L; //por padrao
    registro->salarioServidor = -1; //assume-se que o salario comeca nulo
    registro->telefoneServidor[0] = '\0'; //assume-se que o telefone comeca nulo
    registro->tagCampo4 = 'n';
    registro->nomeServidor = NULL;
    registro->tagCampo5 = 'c';
    registro->cargoServidor = NULL;

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
void leCabecalho(FILE *file, regCabec *cabecalho) {
    long origin = ftell(file);    //guardo a posicao de inicio do registro
    fseek(file, 0, SEEK_SET);   //vou para o inicio do arquivo

    cabecalho->status = fgetc(file);
    fread(&(cabecalho->topoLista), 8, 1, file);
    for (int i = 0; i < 5; i++) {
        cabecalho->tagCampo[i] = fgetc(file);
        fread(cabecalho->desCampo[i], 1, 40, file);  //descricao dos campos
    }

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
void leRegistro(FILE *file, regDados *registro) {
    long origin = ftell(file);    //guardo a posicao de inicio do registro

    registro->removido = fgetc(file);   //leio o campo "removido"
    fread(&(registro->tamanhoRegistro), 4, 1, file);    //leio o indicador de tamanho do registro
    fread(&(registro->encadeamentoLista), 8, 1, file);  //leio o campo "encadeamentoLista"
    fread(&(registro->idServidor), 4, 1, file);     //leio o campo "idServidor"
    fread(&(registro->salarioServidor), 8, 1, file);    //leio o campo "salarioServidor"
    fread(registro->telefoneServidor, 1, 14, file);     //leio o campo "telefoneServidor"

    if ((ftell(file)-origin) == (registro->tamanhoRegistro+5)) {    //se o registro ja acabou, os campos "nomeServidor" e "cargoServidor" sao nulos e, portanto, nao presentes no arquivo
        registro->nomeServidor = NULL;
        registro->cargoServidor = NULL;
    }
    else {
        char c = fgetc(file);
        if (c == '@') {     //eh o ultimo registro de uma pagina de disco
            registro->nomeServidor = NULL;
            registro->cargoServidor = NULL;
        }
        else {
            ungetc(c, file);    //devolvo o caracter para a entrada

            int tamCampo;
            fread(&(tamCampo), 4, 1, file);     //leio o indicador de tamanho do campo
            char tag = fgetc(file);     //leio a tag do campo

            if (tag == 'n') {
                registro->tamCampo4 = tamCampo;
                registro->tagCampo4 = tag;
                registro->nomeServidor = malloc(100*sizeof(char));
                fread(registro->nomeServidor, 1, tamCampo-1, file);   //leio o campo "nomeServidor"

                if ((ftell(file)-origin) == (registro->tamanhoRegistro+5)) {    //se o registro ja acabou, o campo "cargoServidor" eh nulo e, portanto, nao presente no arquivo
                    registro->cargoServidor = NULL;
                }
                else {
                    c = fgetc(file);
                    if (c == '@') {     //eh o ultimo registro de uma pagina de disco
                        registro->cargoServidor = NULL;
                    }
                    else {
                        ungetc(c, file);    //"devolvo" o byte lido para o arquivo
                        fread(&(registro->tamCampo5), 4, 1, file);     //leio o indicador de tamanho do campo
                        registro->tagCampo5 = fgetc(file);     //leio a tag do campo
                        registro->cargoServidor = malloc(100*sizeof(char));
                        fread(registro->cargoServidor, 1, registro->tamCampo5-1, file);   //leio o campo "cargoServidor"
                    }
                }
            }
            else {
                registro->nomeServidor = NULL;

                registro->tamCampo5 = tamCampo;
                registro->tagCampo5 = tag;
                registro->cargoServidor = malloc(100*sizeof(char));
                fread(registro->cargoServidor, 1, tamCampo-1, file);
            }
        }
    }

    fseek(file, origin, SEEK_SET);     //volto ao inicio do registro
}

/*
    Funcao que insere o registro de cabecalho
    no arquivo binario passado como parametro.

    Parametros:
        FILE *file - arquivo binario a ser modificado
        regCabec *cabecalho - cabecalho a ser gravado
*/
void insereCabecalho(FILE *file, regCabec *cabecalho) {
    long origin = ftell(file);
    fseek(file, 0, SEEK_SET);   //vou para o inicio do arquivo

    fwrite(&(cabecalho->status), 1, 1, file);  //escrevo o campo "status" no arquivo binario
    fwrite(&(cabecalho->topoLista), 8, 1, file);  //escrevo o campo "topoLista" no arquivo binario
    for (int i = 0; i < 5; i++) {
        fputc(cabecalho->tagCampo[i], file);  //escrevo o campo "tagCampoX" no arquivo binario
        fwrite(cabecalho->desCampo[i], strlen(cabecalho->desCampo[i])+1, 1, file);  //escrevo o campo "desCampoX" no arquivo binario
        for (int j = 0; j < (40-strlen(cabecalho->desCampo[i])-1); j++) fputc('@', file); //completo com lixo o que sobrar do campo
    }
    for (long i = ftell(file); i < TAMPAG; i++) fputc('@', file);   //completo o resto da pagina de disco com lixo

    fseek(file, origin, SEEK_SET);
}

/*
    Funcao que insere um registro de dados
    na posicao atual do ponteiro de escrita
    do arquivo.

    Parametros:
        FILE *file - arquivo binario a ser
    modificado
        regDados *registro - registro de dados
    a ser gravado
*/
void insereRegistro(FILE *file, regDados *registro) {

    fwrite(&(registro->removido), 1, 1, file);  //escrevo o campo "removido" no arquivo binario

    if (registro->tamanhoRegistro == -1) {  //se nao for para sobreescrever o indicador de tamanho...
        fseek(file, 4, SEEK_CUR);   //apenas pulo ele
    } else {
        fwrite(&(registro->tamanhoRegistro), 4, 1, file);  //escrevo o indicador de tamanho do registro no arquivo binario
    }

    fwrite(&(registro->encadeamentoLista), 8, 1, file);  //escrevo o campo "encadeamentoLista" no arquivo binario
    fwrite(&(registro->idServidor), 4, 1, file);  //escrevo o campo "idServidor" no arquivo binario
    fwrite(&(registro->salarioServidor), 8, 1, file);  //escrevo o campo "salarioServidor" no arquivo binario

    if (registro->telefoneServidor[0] == '\0') {   //se o campo "telefoneServidor" for nulo, entao...
        fwrite(registro->telefoneServidor, 1, 1, file);  //escrevo o '\0'
        for (int i = 0; i < 13; i++) fputc('@', file);  //completo o campo com lixo
    } else {
        fwrite(registro->telefoneServidor, 14, 1, file);  //escrevo o campo "telefoneServidor" no arquivo binario
    }

    if (registro->nomeServidor != NULL) {
        fwrite(&(registro->tamCampo4), 4, 1, file);  //escrevo seu indicador de tamanho no arquivo binario
        fwrite(&(registro->tagCampo4), 1, 1, file);  //escrevo sua tag no arquivo binario
        fwrite(registro->nomeServidor, strlen(registro->nomeServidor)+1, 1, file);  //escrevo-o no arquivo binario
        free(registro->nomeServidor);    //libero memoria anteriormente alocada
    }

    if (registro->cargoServidor != NULL) {
        fwrite(&(registro->tamCampo5), 4, 1, file);  //escrevo seu indicador de tamanho no arquivo binario
        fwrite(&(registro->tagCampo5), 1, 1, file);  //escrevo sua tag no arquivo binario
        fwrite(registro->cargoServidor, strlen(registro->cargoServidor)+1, 1, file);  //escrevo-o no arquivo binario
        free(registro->cargoServidor);   //libero memoria anteriormente alocada
    }

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
void checaFimPagina(FILE *file, regDados *registro, int tamAntigo) {
    if ( (ftell(file)%TAMPAG + registro->tamanhoRegistro + 5) > TAMPAG ) { //se nao ha espaco suficiente... (+5 por conta do campo "removido" e do indicador de tamanho, que nao sao contabilizados no tamanho do registro)
        //insiro o registro em uma outra pagina de disco, mas antes vou precisar completar a pagina de disco atual com lixo e trocar o indicador de tamanho do ultimo registro dela, para que ele contabilize esse lixo tambem
        int diff = TAMPAG - ftell(file)%TAMPAG;  //quantidade necessaria de lixo para completar a pagina de disco

        fseek(file, - (tamAntigo+4), SEEK_CUR); //movo o ponteiro de escrita para o inicio do indicador de tamanho do ultimo registro inserido
        int tamNovo = diff + tamAntigo;  //calculo o valor do novo indicador de tamanho
        fwrite(&tamNovo, 4, 1, file); //sobreescrevo o valor do indicador de tamanho antigo pelo novo

        fseek(file, tamAntigo, SEEK_CUR);  //coloco o ponteiro no final do ultimo registro
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
    long tam = ftell(file);]
    fseek(file, TAMPAG, SEEK_SET);  //vou para o inicio dos registros de dados
    tam -= TAMPAG;
    regDadosI *vetor = calloc(tam, sizeof(regDadosI));
    fread(vetor, 1, tam, file);
    return vetor;
}

/*
    Funcao que imprime na tela o registro de dados
    passado como parametro.

    Parametros:
        regDados *registro - registro de dados a
    ser mostrado
*/
void printRegistro(regDados *registro) {
    printf("Removido: %c\n", registro->removido);
    printf("Tamanho: %d\n", registro->tamanhoRegistro);
    printf("encadeamentoLista: %lld\n", registro->encadeamentoLista);
    printf("Id Servidor: %d\n", registro->idServidor);
    printf("Salario Servidor: %.2lf\n", registro->salarioServidor);
    printf("Telefone Servidor: %s\n", registro->telefoneServidor);

    if (registro->nomeServidor != NULL) {
        printf("-- Tamanho nome: %d\n", registro->tamCampo4);
        printf("-- Tag campo: %c\n", registro->tagCampo4);
        printf("Nome Servidor: \"%s\"\n", registro->nomeServidor);
    }
    if (registro->cargoServidor != NULL) {
        printf("-- Tamanho cargo: %d\n", registro->tamCampo5);
        printf("-- Tag campo: %c\n", registro->tagCampo5);
        printf("Cargo Servidor: \"%s\"\n", registro->cargoServidor);
    }
    printf("\n");
}
