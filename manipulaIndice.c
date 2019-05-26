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
    Funcao que calcula o tamanho do registro
    de dados passado por parametro e troca o
    seu indicador de tamanho, atualizando-o.
    CUIDADO: a funcao NAO o coloca no arquivo.
    Para tanto, utilize a funcao insereRegistro().

    Parametros:
        regDados *registro - registro de dados
    a ser considerado
*/
void calculaTamanho(regDados *registro) {

    if (registro->nomeServidor == NULL && registro->cargoServidor == NULL) {
        registro->tamanhoRegistro = 34;
        //34 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor e 14 do telefoneServidor
    }
    else if (registro->cargoServidor == NULL) {
        registro->tamanhoRegistro = 38 + registro->tamCampo4;
        //38 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor e mais 4 do indicador de tamanho do campo 4
    }
    else if (registro->nomeServidor == NULL) {
        registro->tamanhoRegistro = 38 + registro->tamCampo5;
        //38 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor e mais 4 do indicador de tamanho do campo 5
    }
    else {
        registro->tamanhoRegistro = 42 + registro->tamCampo4 + registro->tamCampo5;
        //42 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor, 4 do indicador de tamanho do campo 4 e mais 4 do indicador de tamanho do campo 5
    }

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
    Adiciona para a lista ordenada de registros removidos
    um novo elemento, adicionando-o em uma ordem crescente.

    Parametros:
        FILE *file - arquivo binario a ser considerado
        long long newBO - byte offset do registro a ser adicionado
        int tamRegistro - tamanho do registro a ser adicionado
*/
void adicionaLista(FILE *file, long long newBO, int tamRegistro) {
    long long nextBO;   //ira dizer qual o byte offset do proximo registro da lista
    long long changeBO; //ira guardar o byte offset do registro anterior ao atual (util na hora da insercao)
    int tamAtual = 0;   //tamanho do registro atual (considerei que o cabecalho tem tamanho "0")

    fseek(file, 1, SEEK_SET);   //posiciono o cabecote de leitura no inicio do campo "topoLista" do registro de cabecalho
    fread(&nextBO, 8, 1, file);  //leio e armazeno o seu valor

    if (nextBO == -1L) {    //se a lista esta vazia
        fseek(file, -8, SEEK_CUR);
        fwrite(&newBO, 8, 1, file); //sobreescrevo o campo "topoLista", que agora aponta para o novo registro removido
        fseek(file, newBO+1, SEEK_SET); //reposiciono o cabecote para ficar na mesma posicao de quando entrou na funcao
        return;
    }
    else while (nextBO != -1L && tamAtual < tamRegistro) {   //aqui, estou a procurar o local correto para encaixar o novo registro removido
        changeBO = ftell(file) - 13;  //-13 para descontar os bytes ja lidos
        fseek(file, nextBO, SEEK_SET);  //vou para o proximo registro no encadeamento
        fgetc(file);    //"joga fora" o primeiro byte do registro
        fread(&tamAtual, 4, 1, file);   //le o indicador de tamanho do registro
        fread(&nextBO, 8, 1, file);   //le o byte offset do proximo registro no encadeamento
    }

    if (tamAtual >= tamRegistro) {  //inserir no meio
        fseek(file, changeBO+5, SEEK_SET);   //somo 5 aos bytes do byte offset para pular o campo "removido" e o indicador de tamanho do registro de dados
        fread(&nextBO, 8, 1, file);     //le o byte offset do proximo registro no encadeamento
        fseek(file, -8, SEEK_CUR);
        fwrite(&newBO, 8, 1, file);     //sobreescrevo o campo "encadeamentoLista", que agora aponta para o novo registro removido

        fseek(file, newBO+5, SEEK_SET);    //vou para o byte offset do mais novo registro removido e pulo os bytes do campo "removido" e do indicador de tamanho
        fwrite(&nextBO, 8, 1, file);    //termino de atualizar o encadeamento
    }
    else if (nextBO == -1L) {   //inserir no final
        fseek(file, changeBO+5, SEEK_SET);   //somo 5 aos bytes do byte offset para pular o campo "removido" e o indicador de tamanho do registro de dados
        fread(&changeBO, 8, 1, file);     //le o byte offset do proximo registro no encadeamento

        fseek(file, changeBO+5, SEEK_SET);   //somo 5 aos bytes do byte offset para pular o campo "removido" e o indicador de tamanho do registro de dados
        fread(&nextBO, 8, 1, file);     //le o byte offset do proximo registro no encadeamento
        fseek(file, -8, SEEK_CUR);
        fwrite(&newBO, 8, 1, file);     //sobreescrevo o campo "encadeamentoLista", que agora aponta para o novo registro removido

        fseek(file, newBO+5, SEEK_SET);    //vou para o byte offset do mais novo registro removido e pulo os bytes do campo "removido" e do indicador de tamanho
        fwrite(&nextBO, 8, 1, file);    //termino de atualizar o encadeamento
    }

    fseek(file, newBO+1, SEEK_SET);     //reposiciono o cabecote para ficar na mesma posicao de quando entrou na funcao
}

/*
    Sobreescreve o conteudo de um registro
    logicamente removido com lixo ("@").

    Parametros:
        FILE *file - arquivo binario que
    contem o registro
*/
void completaLixo(FILE *file) {
    long long origin = ftell(file);     //guardo a posicao inicial do registro
    int tamanho;

    fread(&tamanho, 4, 1, file);   //leio o indicador de tamanho e o guardo
    fseek(file, 8, SEEK_CUR);   //pulo os 8 bytes do campo "encadeamentoLista"
    tamanho -= 8;

    for (int i = 0; i < tamanho; i++) fputc('@', file); //completo com lixo
    fseek(file, origin, SEEK_SET);  //volto para a posicao inicial do registro
}

/*
    Procura na lista de removidos um espaco que
    comporte o novo registro a ser inserido. Caso
    nao encontre nenhum, insere-o no final.

    Parametros:
        FILE *file - arquivo binario a ser considerado
        regDados *registro - registro a ser inserido
        long long ultimoBO - byte offset do ultimo
    registro do arquivo
    Retorno:
        long long - byte offset do ultimo registro do
    arquivo (apos as modificacoes)
*/
long long achaPosicaoInsere(FILE *file, regDados *registro, long long ultimoBO) {
    long origin = ftell(file);    //guarda a posicao em que o ponteiro de leitura entrou na funcao
    fseek(file, 1, SEEK_SET);  //coloco o ponteiro de leitura no comeco do campo "topoLista"

    long long pos, posAnt, posProx, posUlt;
    int tam = 0;

    fread(&posProx, 8, 1, file);    //leio o campo "topoLista"

    if (posProx == -1L) {
        //insiro o registro no final do arquivo
        fseek(file, 0, SEEK_END);
        posUlt = ftell(file);

        if (ultimoBO != -1) {
            //verifico o espaco disponivel na pagina de disco atual
            if ( (ftell(file)%TAMPAG + registro->tamanhoRegistro + 5) > TAMPAG ) { //se nao ha espaco suficiente... (+5 por conta do campo "removido" e do indicador de tamanho, que nao sao contabilizados no tamanho do registro)
                //insiro o registro em uma outra pagina de disco, mas antes vou precisar completar a pagina de disco atual com lixo e trocar o indicador de tamanho do ultimo registro dela, para que ele contabilize esse lixo tambem
                int diff = TAMPAG - ftell(file)%TAMPAG;  //quantidade necessaria de lixo para completar a pagina de disco

                int tamAntigo;
                fread(&tamAntigo, 4, 1, file);
                fseek(file, -4, SEEK_CUR);

                int tamNovo = diff + tamAntigo;  //calculo o valor do novo indicador de tamanho
                fwrite(&tamNovo, 4, 1, file); //sobreescrevo o valor do indicador de tamanho antigo pelo novo

                fseek(file, tamAntigo, SEEK_CUR);  //coloco o ponteiro no final do ultimo registro
                for (int i = 0; i < diff; i++) fputc('@', file);  //completo com lixo
            }

        }

        insereRegistro(file, registro);
    }
    else {
        while (posProx != -1L && tam < registro->tamanhoRegistro) {
            posAnt = ftell(file) - 13;  //-13 para descontar os bytes ja lidos
            fseek(file, posProx, SEEK_SET);  //vou para o proximo registro no encadeamento

            fgetc(file);    //"joga fora" o primeiro byte do registro
            fread(&tam, 4, 1, file);   //le o indicador de tamanho do registro
            fread(&posProx, 8, 1, file);   //le o byte offset do proximo registro no encadeamento
        }

        if (tam >= registro->tamanhoRegistro) {  //inserir no "meio"
            pos = ftell(file) - 13;    //retiro os 13 bytes que já havia lido
            fseek(file, posAnt+5, SEEK_SET);   //somo 5 aos bytes do byte offset para pular o campo "removido" e o indicador de tamanho do registro de dados
            fwrite(&posProx, 8, 1, file);     //sobreescrevo o campo "encadeamentoLista", que agora aponta para o próximo registro depois do que foi sobreescrito pelo registro adicionado

            fseek(file, pos, SEEK_SET);
            registro->tamanhoRegistro = -1;  //como o registro vai ser inserido em um espaco removido logicamente, seu indicador de tamanho nao deve ser escrito, permanecendo o do registro anterior a ele
            insereRegistro(file, registro);
            posUlt = -1;
        }
        else {   //inserir no final
            fseek(file, 0, SEEK_END);

            if (ultimoBO != -1) {
                //verifico o espaco disponivel na pagina de disco atual
                if ( (ftell(file)%TAMPAG + registro->tamanhoRegistro + 5) > TAMPAG ) { //se nao ha espaco suficiente... (+5 por conta do campo "removido" e do indicador de tamanho, que nao sao contabilizados no tamanho do registro)
                    //insiro o registro em uma outra pagina de disco, mas antes vou precisar completar a pagina de disco atual com lixo e trocar o indicador de tamanho do ultimo registro dela, para que ele contabilize esse lixo tambem
                    int diff = TAMPAG - ftell(file)%TAMPAG;  //quantidade necessaria de lixo para completar a pagina de disco

                    fseek(file, ultimoBO+1, SEEK_SET);    //vou para o comeco do indicador de tamanho do ultimo registro do arquivo
                    int tamAntigo;
                    fread(&tamAntigo, 4, 1, file);
                    fseek(file, -4, SEEK_CUR);

                    int tamNovo = diff + tamAntigo;  //calculo o valor do novo indicador de tamanho
                    fwrite(&tamNovo, 4, 1, file); //sobreescrevo o valor do indicador de tamanho antigo pelo novo

                    fseek(file, tamAntigo, SEEK_CUR);  //coloco o ponteiro no final do ultimo registro
                    for (int i = 0; i < diff; i++) fputc('@', file);  //completo com lixo
                }

            }

            posUlt = ftell(file);
            insereRegistro(file, registro);
        }
    }

    fseek(file, origin, SEEK_SET);     //volto o ponteiro de leitura para o lugar original dele
    return posUlt;
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

/*
    Funcao que imprime na tela a lista de removidos
    do arquivo binario passado como parametro.

    Parametros:
        FILE *file - arquivo binario a ser considerado
*/
void printLista(FILE *file) {
    long origin = ftell(file);
    long long nextBO;   //ira dizer qual o byte offset do proximo registro da lista
    int tam;

    fseek(file, 1, SEEK_SET);   //posiciono o cabecote de leitura no inicio do campo "topoLista" do registro de cabecalho
    fread(&nextBO, 8, 1, file);  //leio e armazeno o seu valor
    printf("CABEC -> ");

    while (nextBO != -1L) {
        fseek(file, nextBO, SEEK_SET);  //vou para o proximo registro no encadeamento
        printf("%ld ", ftell(file));
        fgetc(file);    //"joga fora" o primeiro byte do registro
        fread(&tam, 4, 1, file);   //le o indicador de tamanho do registro
        printf("(%d) -> ", tam);
        fread(&nextBO, 8, 1, file);   //le o byte offset do proximo registro no encadeamento
    }
    printf("-1\n\n");

    fseek(file, origin, SEEK_SET);
}

void freeRegistro(regDados *registro) {
    if (registro->nomeServidor != NULL) free(registro->nomeServidor);
    if (registro->cargoServidor != NULL) free(registro->cargoServidor);
    free(registro);
}
