/*
* Programa que manipula dados de arquivos,
* permitindo leitura, escrita, impressao,
* remocao, adicao e atualizacao de campos
* e registros escritos em disco.
*
* Codigo original (ate T3): Vitor Santana
* Modificacoes (T4/T5):
* - 10734769, Joao Vitor Silva Ramos
* - 10734345, Vitor Santana Cordeiro
*
* Turma: BCC B
* Sao Carlos, SP - Brasil
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "escreverTela.h"
#include "manipulaArquivo.h"
#include "manipulaIndice.h"

#define THRESHOLD 16384 //chute inicial para o numero maximo de registros de dados em um arquivo binario

/*
    Funcao que le o arquivo CSV e organiza seus
    registros em um arquivo binario de saida,
    utilizando organizacao hibrida dos campos.
*/
void leCSV() {
    char fileName[51];   //vai guardar o nome do arquivo a ser aberto
    scanf("%50s", fileName);

    FILE *readFile = fopen(fileName, "r");  //abro o arquivo de nome "fileName" para leitura
    FILE *writeFile = fopen("arquivoTrab1.bin", "wb");  //crio um novo arquivo binario para escrita

    if (readFile == NULL || writeFile == NULL) {   //erro na abertura dos arquivos
        printf("Falha no carregamento do arquivo.");
        return;
    }

    regCabec *cabecalho = criaCabecalho();   //crio um registro de cabecalho, utilizando valores definidos na especificacao

    //armazeno os metadados
    char c, buffer[100];  //o buffer sera utilizado tanto aqui quanto na hora de ler os registros de dados
    int idx = 0, cont = 0;   //indexador e contador

    while ((c = fgetc(readFile)) != EOF) {   //enquanto nao chegar no final do arquivo...
        if (c == ',' || c == '\n') {   //se achei uma virgula ou uma quebra de linha, cheguei ao fim de um campo
            buffer[idx++] = '\0';   //finalizo a string ate entao lida
            strcpy(cabecalho->desCampo[cont++], buffer);   //guardo o metadado lido no campo correspondente
            idx = 0;  //reinicio o indexador do buffer

            if (c == '\n') break;   //se cheguei no '\n', isso significa que a primeira linha (que contem os metadados) ja foi lida
        }
        else buffer[idx++] = c;  //guardo o char lido no buffer
    }

    insereCabecalho(writeFile, cabecalho);

    //agora leio o resto do arquivo, criando os registros de dados
    regDados *registro = criaRegistro();    //crio um "molde" para guardar os valores lidos do CSV

    cont = 0, idx = 0;   //reinicio o contador e o indexador do buffer
    memset(buffer, '\0', 100);  //limpo o buffer
    int ehNulo = 0;   //indica se o campo que esta se lendo eh nulo ou nao
    int tamAntigo = 0;  //guarda o tamanho do ultimo registro escrito no arquivo binario
    int haDados = 0;  //indica se o arquivo CSV possui dados (alem do cabecalho)

    while ((c = fgetc(readFile)) != EOF) {  //enquanto nao chegar no final do arquivo...
        haDados = 1;

        if (c == '\n') {    //se cheguei no '\n', significa que este eh o ultimo campo do registro de dados atual
            if (ehNulo) {
                if (registro->cargoServidor != NULL) free(registro->cargoServidor);   //libero a memoria alocada anteriormente para o campo
                registro->cargoServidor = NULL;
            } else {
                buffer[idx] = '\0';   //finalizo a string ate entao lida
                if (registro->cargoServidor == NULL) registro->cargoServidor = malloc(100*sizeof(char));  //aloco memoria para o campo, caso ele nao tenha
                strcpy(registro->cargoServidor, buffer);
                registro->tamCampo5 = strlen(registro->cargoServidor) + 2;  // +2 pois conta 1 byte da tag e 1 byte do '\0' (ele nao eh contabilizado na strlen)
            }

            calculaTamanho(registro);
            checaFimPagina(writeFile, registro, tamAntigo);
            insereRegistro(writeFile, registro);

            cont = 0; //reinicio o contador
            idx = 0;  //reinicio o indexador do buffer
            memset(buffer, '\0', 100);  //limpo o buffer
            tamAntigo = registro->tamanhoRegistro; //atualizo o tamanho do ultimo registro inserido
        }
        else if (c == ',') {
            switch (cont) {
                case 0: //o campo que li foi o idServidor
                    registro->idServidor = atoi(buffer); //converto o valor lido para inteiro e o armazeno
                break;
                case 1: //o campo que li foi o salarioServidor
                    registro->salarioServidor = atof(buffer);  //converto o valor lido para double e o armazeno
                break;
                case 2: //o campo que li foi o telefoneServidor
                    if (ehNulo) {
                        strcpy(registro->telefoneServidor, "\0");
                    } else {
                        for (int i = 0; i < 14; i++) registro->telefoneServidor[i] = buffer[i];
                    }
                break;
                case 3: //o campo que li foi o nomeServidor
                    if (ehNulo) {
                        if (registro->nomeServidor != NULL) free(registro->nomeServidor);  //libero a memoria alocada anteriormente para o campo
                        registro->nomeServidor = NULL;
                    } else {
                        buffer[idx] = '\0';   //finalizo a string ate entao lida
                        if (registro->nomeServidor == NULL) registro->nomeServidor = malloc(100*sizeof(char));  //aloco memoria para o campo, caso ele nao tenha
                        strcpy(registro->nomeServidor, buffer);
                        registro->tamCampo4 = strlen(registro->nomeServidor) + 2;  // +2 pois conta 1 byte da tag e 1 byte do '\0' (ele nao eh contabilizado na strlen)
                    }
                break;
            }
            cont++;
            idx = 0;  //reinicio o indexador do buffer
            memset(buffer, '\0', 100);  //limpo o buffer
            ehNulo = 1;   //faz com que, se o proximo char lido for outra virgula, o loop saiba que aquele campo tem valor nulo
        }
        else {
            buffer[idx++] = c;
            ehNulo = 0;   //nao foi outra virgula, entao o valor do campo nao eh nulo
        }
    }

    if (haDados) {      //registro os dados da ultima linha do csv (ja que ela nao possui o '\n' no final)
        calculaTamanho(registro);
        checaFimPagina(writeFile, registro, tamAntigo);
        insereRegistro(writeFile, registro);
    }

    printf("arquivoTrab1.bin");
    fclose(readFile);

    //antes de fechar o arquivo, coloco seu status para '1'
    fseek(writeFile, 0, SEEK_SET);  //coloco o ponteiro de escrita no primeiro byte do arquivo
    fputc('1', writeFile);  //sobrescrevo o campo "status" do arquivo binario

    free(cabecalho);
    freeRegistro(registro);
    fclose(writeFile);
}

/*
    Funcao que imprime na tela, organizadamente,
    um registro de um arquivo binario gerado
    anteriormente por este programa. A funcao
    assume que o usuario ira chama-la quando
    o ponteiro de leitura estiver exatamente no
    comeco do registro.
    Parametro:
        FILE *file - arquivo binario
*/
void mostraRegistro(FILE *file) {
    regDados *registro = criaRegistro();  //registro que ajudara a guardar os dados lidos

    fseek(file, -1, SEEK_CUR);  //volto ao comeco do campo "removido"
    leRegistro(file, registro); //leio o registro do arquivo binario

    printf("%d", registro->idServidor);   //mostro o valor do campo "idServidor" na tela

    if (registro->salarioServidor != -1.0) printf(" %.2lf", registro->salarioServidor);   //mostro o valor do campo "salarioServidor" na tela
    else printf("         ");   //se o campo for nulo, mostro 8 espacos em branco

    if (registro->telefoneServidor[0] != '\0') printf(" %.14s", registro->telefoneServidor);   //mostro o valor do campo "telefoneServidor" na tela
    else printf("               "); //se o campo for nulo, mostro 14 espacos em branco

    if (registro->nomeServidor != NULL) {
        printf(" %d", registro->tamCampo4 - 2);  //mostro quantos caracteres o campo possui (-2 por conta do '\0' e da tag)
        printf(" %s", registro->nomeServidor);
    }

    if (registro->cargoServidor != NULL) {
        printf(" %d", registro->tamCampo5 - 2);  //mostro quantos caracteres o campo possui (-2 por conta do '\0' e da tag)
        printf(" %s", registro->cargoServidor);
    }

    printf("\n");   //termino de mostrar o registro

    fseek(file, registro->tamanhoRegistro+5, SEEK_CUR); //vou para o final do registro (+5 por conta do campo "removido" e do indicador de tamanho, nao contabilizados no mesmo)
    freeRegistro(registro);
}

/*
    Le um arquivo binario (anteriormente gerado pelo programa)
    e mostra na tela todos os seus registros, organizadamente.
    Ao final, mostra quantas paginas de disco foram acessadas
    ao todo.
*/
void mostraBin() {
    char fileName[51];   //vai guardar o nome do arquivo a ser aberto
    scanf("%50s", fileName);

    FILE *readFile = fopen(fileName, "rb");  //abre o arquivo "fileName" para leitura binária

    if (readFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    int acessosPagina = 0; //vai contar a quantidade de acessos a paginas de disco no decorrer da execucao

    if (fgetc(readFile) == '0') {   //se o byte "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }
    acessosPagina++;

    fseek(readFile, TAMPAG-1, SEEK_CUR);  //pulo a primeira pagina, que so tem o registro de cabecalho (lembrando que ja tinha lido o campo "status", por isso o -1)

    byte b = fgetc(readFile);

    if (feof(readFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
      printf("Registro inexistente.");
      return;
    }

    while (!feof(readFile)) {
        if (ftell(readFile)%TAMPAG == 1) acessosPagina++;   //se o ponteiro de leitura passou pelo primeiro byte de uma pagina de disco, entao conta-se mais um acesso

        if (b == '-') mostraRegistro(readFile);  //mostra o registro se ele nao esta removido
        else if (b == '*') {    //se ele esta removido...
            int pulo;
            fread(&pulo, 4, 1, readFile);   //lera o indicador de tamanho do registro (4 bytes)
            fseek(readFile, pulo, SEEK_CUR);    //pula o registro
        }

        b = fgetc(readFile);
    }

    printf("Número de páginas de disco acessadas: %d", acessosPagina);

    fclose(readFile);
}

/*
    Busca, em todo o arquivo binario, registros que
    satisfacam um criterio de busca determinado pelo
    usuario, mostrando-os na tela assim que sao encontrados.
    Um exemplo de busca seria "cargoServidor ENGENHEIRO",
    no qual a funcao ira mostrar na tela todos os registros
    em que o campo "cargoServidor" possui o valor "ENGENHEIRO".
    Tambem eh mostrado, ao final da execucao, quantas paginas
    de disco foram acessadas ao todo.
*/
void buscaReg() {
    char fileName[51];   //vai guardar o nome do arquivo a ser aberto
    char nomeCampo[51];    //campo a ser considerado na busca
    byte valorCampo[100];    //valor a ser considerado na busca
    regCabec *cabecalho = criaCabecalho();  //estrutura que sera utilizada para guardar os valores do registro de cabecalho
    regDados *registro = criaRegistro();  //estrutura que sera utilizada para guardar os registros lidos do arquivo binario
    int acessosPagina = 0; //vai contar a quantidade de acessos a paginas de disco no decorrer da execucao

    scanf("%50s %50s", fileName, nomeCampo);

    printf("%s\n", fileName);

    scanf(" %[^\r\n]", valorCampo);     //paro de ler antes da quebra de linha

    FILE *readFile = fopen(fileName, "rb");  //abre o arquivo "fileName" para leitura binária

    if (readFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    leCabecalho(readFile, cabecalho);
    acessosPagina++;

    if (cabecalho->status == '0') {   //se o campo "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }

    fseek(readFile, TAMPAG, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)

    byte b = fgetc(readFile);

    if (feof(readFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
      printf("Registro inexistente.");
      return;
    }

    int achou = 0;    //indicara se pelo menos um registro foi achado

    while (!feof(readFile)) {
        ungetc(b, readFile); //"devolvo" o byte lido para o arquivo binario
        if (ftell(readFile)%TAMPAG == 0) acessosPagina++;   //o cabecote esta no comeco de uma pagina de disco

        leRegistro(readFile, registro);
        int indicTam = registro->tamanhoRegistro;

        if (registro->removido == '-') {   //o registro pode ser manipulado
            if (!strcmp(nomeCampo, "idServidor")) {    //se o campo a ser buscado eh "idServidor"...
                if (registro->idServidor == atoi(valorCampo)) {    //se o valor do campo no registro lido eh igual ao do dado como criterio de busca...
                    mostraRegistroMeta(cabecalho, registro);
                    achou = 1;
                    break;  //ja que o numero do idServidor eh unico, se acharmos um igual nao precisaremos mais continuar procurando
                }
            }
            else if (!strcmp(nomeCampo, "salarioServidor")) {    //se o campo a ser buscado eh "salarioServidor"...
                if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                    if (registro->salarioServidor == -1) {
                        mostraRegistroMeta(cabecalho, registro);
                        achou = 1;
                    }
                }
                else {  //o valor a ser buscado nao eh nulo
                    if (registro->salarioServidor == atof(valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                        mostraRegistroMeta(cabecalho, registro);
                        achou = 1;
                    }
                }
            }
            else if (!strcmp(nomeCampo, "telefoneServidor")) {    //se o campo a ser buscado eh "telefoneServidor"...
                if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                    if (registro->telefoneServidor[0] == '\0') {
                        mostraRegistroMeta(cabecalho, registro);
                        achou = 1;
                    }
                }
                else {  //o valor a ser buscado nao eh nulo
                    if (!strcmp(registro->telefoneServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                        mostraRegistroMeta(cabecalho, registro);
                        achou = 1;
                    }
                }
            }
            else if (!strcmp(nomeCampo, "nomeServidor")) {    //se o campo a ser buscado eh "nomeServidor"...
                if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                    if (registro->nomeServidor == NULL) {
                        mostraRegistroMeta(cabecalho, registro);
                        achou = 1;
                    }
                }
                else {  //o valor a ser buscado nao eh nulo
                    if (registro->nomeServidor != NULL && !strcmp(registro->nomeServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                        mostraRegistroMeta(cabecalho, registro);
                        achou = 1;
                    }
                }
            }
            else if (!strcmp(nomeCampo, "cargoServidor")) {    //se o campo a ser buscado eh "cargoServidor"...
                if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                    if (registro->cargoServidor == NULL) {
                        mostraRegistroMeta(cabecalho, registro);
                        achou = 1;
                    }
                }
                else {  //o valor a ser buscado nao eh nulo
                    if (registro->cargoServidor != NULL && !strcmp(registro->cargoServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                        mostraRegistroMeta(cabecalho, registro);
                        achou = 1;
                    }
                }
            }
            else {  //o usuario digitou errado o nome do campo
                printf("Falha no processamento do arquivo.");
                return;
            }
        }

        if (registro->nomeServidor != NULL) free(registro->nomeServidor);
        if (registro->cargoServidor != NULL) free(registro->cargoServidor);
        fseek(readFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
        b = fgetc(readFile);
    }

    if (!achou) {
        printf("Registro inexistente.");
    }
    else {
        printf("Número de páginas de disco acessadas: %d", acessosPagina);
    }

    free(cabecalho);
    freeRegistro(registro);
    fclose(readFile);
}

/*
    Busca no arquivo binario registros que satisfacam
    um criterio de busca determinado pelo usuario,
    removendo-os logicamente assim que sao encontrados.
    O usuario deve informar quantas buscas diferentes
    deseja fazer perante uma mesma execucao. Alem disso,
    deve informar, em cada uma delas, o nome e o valor
    do campo a ser buscado.
*/
void removeReg() {
    char fileName[51];   //vai guardar o nome do arquivo a ser aberto
    int n;      //numero de remocoes a serem realizadas
    char nomeCampo[51];    //campo a ser considerado na busca
    byte valorCampo[200];    //valor a ser considerado na busca
    regDados *registro = criaRegistro();  //estrutura que será utilizada para guardar os registros lidos do arquivo binario

    scanf("%50s", fileName);
    scanf("%d", &n);

    FILE *binFile = fopen(fileName, "rb+");  //abre o arquivo "fileName" para leitura e escrita binária

    if (binFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    if (fgetc(binFile) == '0') {   //se o byte "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }
    ungetc('0', binFile);  //como o arquivo foi aberto para escrita, seu status deve ser '0'

    fseek(binFile, TAMPAG, SEEK_CUR);  //pulo o registro de cabecalho

    for (int i = 0; i < n; i++) {
        memset(nomeCampo, 0, 51);   //limpo o vetor (setando tudo para 0)
        memset(valorCampo, 0, 200);  //limpo o vetor (setando tudo para 0)

        scanf("%50s %[^\r\n]", nomeCampo, valorCampo);  //paro de ler antes da quebra de linha

        byte b = fgetc(binFile);

        if (feof(binFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
          printf("Registro inexistente.");
          return;
        }

        while (!feof(binFile)) {
            ungetc(b, binFile); //"devolvo" o byte lido para o arquivo binario
            leRegistro(binFile, registro);
            int indicTam = registro->tamanhoRegistro;

            if (registro->removido == '*') {   //o registro esta removido
                fseek(binFile, indicTam+5, SEEK_CUR);   //se o registro esta removido, apenas o pulo
            }
            else if (registro->removido == '-') {   //o registro pode ser manipulado
                if (!strcmp(nomeCampo, "idServidor")) {    //se o campo a ser buscado eh "idServidor"...
                    if (registro->idServidor == atoi(valorCampo)) {    //se o valor do campo no registro lido eh igual ao do dado como criterio de busca...
                        long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                        fputc('*', binFile);    //marco o registro como REMOVIDO
                        adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                        completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                        break;  //ja que o numero do idServidor eh unico, se acharmos um igual nao precisaremos mais continuar procurando
                    }
                    else {
                        fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                    }
                }

                else if (!strcmp(nomeCampo, "salarioServidor")) {    //se o campo a ser buscado eh "salarioServidor"...
                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->salarioServidor == -1) {
                            long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', binFile);    //marco o registro como REMOVIDO
                            adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        if (registro->salarioServidor == atof(valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                            long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', binFile);    //marco o registro como REMOVIDO
                            adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else if (!strcmp(nomeCampo, "telefoneServidor")) {    //se o campo a ser buscado eh "telefoneServidor"...
                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->telefoneServidor[0] == '\0') {
                            long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', binFile);    //marco o registro como REMOVIDO
                            adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        if (!strcmp(registro->telefoneServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                            long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', binFile);    //marco o registro como REMOVIDO
                            adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else if (!strcmp(nomeCampo, "nomeServidor")) {    //se o campo a ser buscado eh "nomeServidor"...
                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->nomeServidor == NULL) {
                            long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', binFile);    //marco o registro como REMOVIDO
                            adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        char *valorSemAspas = strtok(valorCampo, "\"");     //retiro as aspas da string (para efeitos de comparacao)

                        if (registro->nomeServidor != NULL && !strcmp(registro->nomeServidor, valorSemAspas)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                            long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', binFile);    //marco o registro como REMOVIDO
                            adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else if (!strcmp(nomeCampo, "cargoServidor")) {    //se o campo a ser buscado eh "cargoServidor"...
                    char *valorSemAspas = strtok(valorCampo, "\"");     //retiro as aspas da string (para efeitos de comparacao)

                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->cargoServidor == NULL) {
                            long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', binFile);    //marco o registro como REMOVIDO
                            adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        if (registro->cargoServidor != NULL && !strcmp(registro->cargoServidor, valorSemAspas)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                            long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', binFile);    //marco o registro como REMOVIDO
                            adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else {  //o usuario digitou errado o nome do campo
                    printf("Falha no processamento do arquivo.");
                    return;
                }
            }

            b = fgetc(binFile);
        }

        fseek(binFile, TAMPAG, SEEK_SET);    //volto o ponteiro de leitura para o inicio da segunda pagina de disco (a que inicia os registros de dados)
    }

    binarioNaTela1(binFile);

    //antes de fechar o arquivo, coloco seu status para '1'
    fseek(binFile, 0, SEEK_SET);  //coloco o ponteiro de escrita no primeiro byte do arquivo
    fputc('1', binFile);  //sobrescrevo o campo "status" do arquivo binario

    freeRegistro(registro);
    fclose(binFile);
}

/*
    Adiciona novos registros ao arquivo binario,
    reaproveitando os espacos deixados pelos
    registros logicamente removidos. Para tanto,
    percorre-se a lista de removidos ate que se
    encontre um espaco em que caiba o novo registro.
    Caso nao se encontre nenhum espaco grande o
    suficiente, o registro eh escrito no final do
    arquivo. O usuario deve informar quantas insercoes
    diferentes deseja fazer perante uma mesma execucao.
    Alem disso, deve informar, em cada uma delas, o
    valor dos campos do registro.
*/
void adicionaReg() {
    char fileName[51];   //vai guardar o nome do arquivo a ser aberto
    int n;      //numero de insercoes a serem realizadas
    regDados *registro = criaRegistro();  //ira guardar os dados fornecidos pelo usuario
    char buffer[201];
    long long posUltimoReg = -1;  //ira guardar o byte offset do ultimo registro do arquivo

    scanf("%50s", fileName);
    scanf("%d", &n);

    FILE *binFile = fopen(fileName, "rb+");  //abre o arquivo "fileName" para leitura e escrita binária

    if (binFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    if (fgetc(binFile) == '0') {   //se o byte "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }
    ungetc('0', binFile);  //como o arquivo foi aberto para escrita, seu status deve ser '0'

    for (int i = 0; i < n; i++) {

        scanf("%d ", &(registro->idServidor));   //pego o valor de idServidor (esse campo nao pode ser nulo)

        //pego o valor de salarioServidor
        memset(buffer, 0, 201);   //limpo o buffer
        char c = fgetc(stdin);
        if (c == 'N') {     //o campo eh nulo
            registro->salarioServidor = -1;
            fseek(stdin, 4, SEEK_CUR);  //pulo os caracteres "ULO "
        }
        else {
            ungetc(c, stdin);   //devolvo o char lido para a entrada padrao
            scanf("%s ", buffer);
            registro->salarioServidor = atof(buffer);
        }

        //pego o valor de telefoneServidor
        memset(buffer, 0, 201);   //limpo o buffer
        c = fgetc(stdin);
        if (c == 'N') {     //o campo eh nulo
            registro->telefoneServidor[0] = '\0';
            fseek(stdin, 4, SEEK_CUR);  //pulo os caracteres "ULO "
        }
        else {
            ungetc(c, stdin);   //devolvo o char lido para a entrada padrao
            scanf("%*[\"]%[^\"]", buffer);  //le o campo, desconsiderando as aspas
            fseek(stdin, 2, SEEK_CUR);  //pulo os caracteres "\" "
            strcpy(registro->telefoneServidor, buffer);
        }

        //pego o valor de nomeServidor
        memset(buffer, 0, 201);   //limpo o buffer
        c = fgetc(stdin);
        if (c == 'N') {     //o campo eh nulo
            registro->nomeServidor = NULL;
            fseek(stdin, 4, SEEK_CUR);  //pulo os caracteres "ULO "
        }
        else {
            ungetc(c, stdin);   //devolvo o char lido para a entrada padrao
            scanf("%*[\"]%[^\"]", buffer);  //le o campo, desconsiderando as aspas
            fseek(stdin, 2, SEEK_CUR);  //pulo os caracteres "\" "
            registro->nomeServidor = malloc(100*sizeof(char));
            strcpy(registro->nomeServidor, buffer);
            registro->tamCampo4 = strlen(registro->nomeServidor) + 2;     //conta tambem o '\0' e a tag do campo
        }

        //pego o valor de cargoServidor
        memset(buffer, 0, 201);   //limpo o buffer
        c = fgetc(stdin);
        if (c == 'N') {     //o campo eh nulo
            registro->cargoServidor = NULL;
            fseek(stdin, 4, SEEK_CUR);  //pula os caracteres "ULO "
        }
        else {
            ungetc(c, stdin);   //devolvo o char lido para a entrada padrao
            scanf("%*[\"]%[^\"]", buffer);  //le o campo, desconsiderando as aspas
            fseek(stdin, 2, SEEK_CUR);  //pulo os caracteres "\" "
            registro->cargoServidor = malloc(100*sizeof(char));
            strcpy(registro->cargoServidor, buffer);
            registro->tamCampo5 = strlen(registro->cargoServidor) + 2;     //conta tambem o '\0' e a tag do campo
        }

        calculaTamanho(registro);

        long long temp = achaPosicaoInsere(binFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
        if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado
    }

    binarioNaTela1(binFile);

    //antes de fechar o arquivo, coloco seu status para '1'
    fseek(binFile, 0, SEEK_SET);  //coloco o ponteiro de escrita no primeiro byte do arquivo
    fputc('1', binFile);  //sobrescrevo o campo "status" do arquivo binario

    freeRegistro(registro);
    fclose(binFile);
}

/*
    Muda o campo de um registro de dados, de acordo
    com valores passados para a funcao.
    Parametros:
        regDados *registro - registro a ser modificado
        char *nomeCampo - nome do campo a ser modificado
        byte *valorCampo - valor novo do campo
    Retorno:
        int - retorna a diferenca entre o tamanho do
    registro original e o novo registro; sera negativo
    caso o registro novo seja maior que o original.
*/
int mudaRegistro(regDados *registro, char *nomeCampo, byte *valorCampo) {
    int tamOriginal = registro->tamanhoRegistro;
    char *valorSemAspas;    //sera usado para retirar as aspas dos valores string

    if (!strcmp(nomeCampo, "idServidor")) {
        registro->idServidor = atoi(valorCampo);
    }
    else if (!strcmp(nomeCampo, "salarioServidor")) {
        if (!strcmp(valorCampo, "NULO")) {  //se o valor for nulo...
            registro->salarioServidor = -1;
        }
        else {
            registro->salarioServidor = atof(valorCampo);
        }
    }
    else if (!strcmp(nomeCampo, "telefoneServidor")) {
        if (!strcmp(valorCampo, "NULO")) {  //se o valor for nulo...
            registro->telefoneServidor[0] = '\0';
            for (int i = 1; i < 14; i++) registro->telefoneServidor[i] = '@';   //completo com lixo
        }
        else {
            valorSemAspas = strtok(valorCampo, "\"");
            for (int i = 0; i < 14; i++) registro->telefoneServidor[i] = valorSemAspas[i];
        }
    }
    else if (!strcmp(nomeCampo, "nomeServidor")) {
        if (!strcmp(valorCampo, "NULO")) {  //se o valor for nulo...
            registro->nomeServidor = NULL;
        }
        else {
            valorSemAspas = strtok(valorCampo, "\"");
            if (registro->nomeServidor == NULL) registro->nomeServidor = malloc(100*sizeof(char));
            strcpy(registro->nomeServidor, valorSemAspas);
            registro->tamCampo4 = strlen(registro->nomeServidor) + 2;
        }
    }
    else if (!strcmp(nomeCampo, "cargoServidor")) {
        if (!strcmp(valorCampo, "NULO")) {  //se o valor for nulo...
            registro->cargoServidor = NULL;
        }
        else {
            valorSemAspas = strtok(valorCampo, "\"");
            if (registro->cargoServidor == NULL) registro->cargoServidor = malloc(100*sizeof(char));
            strcpy(registro->cargoServidor, valorSemAspas);
            registro->tamCampo5 = strlen(registro->cargoServidor) + 2;
        }
    }

    int tamNovo;

    //calculo o tamanho total do registro modificado
    if (registro->nomeServidor == NULL && registro->cargoServidor == NULL) {
        tamNovo = 34;
        //34 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor e 14 do telefoneServidor
    }
    else if (registro->cargoServidor == NULL) {
        tamNovo = 38 + registro->tamCampo4;
        //38 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor e mais 4 do indicador de tamanho do campo 4
    }
    else if (registro->nomeServidor == NULL) {
        tamNovo = 38 + registro->tamCampo5;
        //38 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor e mais 4 do indicador de tamanho do campo 5
    }
    else {
        tamNovo = 42 + registro->tamCampo4 + registro->tamCampo5;
        //42 -> 8 do encadeamentoLista, 4 do idServidor, 8 do salarioServidor, 14 do telefoneServidor, 4 do indicador de tamanho do campo 4 e mais 4 do indicador de tamanho do campo 5
    }

    if (tamNovo > registro->tamanhoRegistro) registro->tamanhoRegistro = tamNovo;

    return (tamOriginal-tamNovo);
}

/*
    Busca no arquivo binario registros que satisfacam
    um criterio de busca determinado pelo usuario,
    atualizando-os assim que sao encontrados.
    O usuario deve informar quantas buscas diferentes
    deseja fazer perante uma mesma execucao. Alem disso,
    deve informar, em cada uma delas, o nome e o valor
    do campo a ser buscado e, também, o nome e o valor
    do campo a ser atualizado, que pode ou nao ser igual
    ao campo de busca.
*/
void atualizaReg() {
    char fileName[51];   //vai guardar o nome do arquivo a ser aberto
    int n;      //numero de remocoes a serem realizadas
    char nomeCampo[51];    //campo a ser considerado na busca
    byte valorCampo[200];   //valor a ser considerado na busca
    char nomeAtualiza[51];      //nome do campo a ser atualizado
    byte valorAtualiza[200];    //valor do campo a ser atualizado
    regDados *registro = criaRegistro();  //estrutura que será utilizada para guardar os registros lidos do arquivo binario
    long long posUltimoReg = -1;  //ira guardar o byte offset do ultimo registro do arquivo

    scanf("%50s", fileName);
    scanf("%d", &n);

    FILE *binFile = fopen(fileName, "rb+");  //abre o arquivo "fileName" para leitura e escrita binária

    if (binFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    if (fgetc(binFile) == '0') {   //se o byte "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }
    ungetc('0', binFile);  //como o arquivo foi aberto para escrita, seu status deve ser '0'

    fseek(binFile, TAMPAG, SEEK_CUR);  //pulo o registro de cabecalho

    for (int i = 0; i < n; i++) {
        memset(nomeCampo, 0, 51);   //limpo o vetor (setando tudo para 0)
        memset(valorCampo, 0, 200);  //limpo o vetor (setando tudo para 0)
        memset(nomeAtualiza, 0, 51);   //limpo o vetor (setando tudo para 0)
        memset(valorAtualiza, 0, 200);  //limpo o vetor (setando tudo para 0)

        scanf("%50s ", nomeCampo);

        char c = fgetc(stdin);
        if (c == '\"') {    //se o valor comecar com uma aspas, eh uma string com espacos (ou seja, nao pode ser lida do jeito convencional)
            scanf("%[^\"]", valorCampo);    //paro de ler a string na ultima aspas
            fgetc(stdin);   //pulo um char
            fgetc(stdin);   //pulo mais um char
        } else {    //o valor nao tem espacos (pode ser lido normalmente)
            ungetc(c, stdin);   //devolvo o char lido para a entrada
            scanf("%s ", valorCampo);
        }

        scanf("%50s %[^\r\n]", nomeAtualiza, valorAtualiza);  //paro de ler antes da quebra de linha

        byte b = fgetc(binFile);

        if (feof(binFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
          printf("Registro inexistente.");
          return;
        }

        while (!feof(binFile)) {
            ungetc(b, binFile); //"devolvo" o byte lido para o arquivo binario
            leRegistro(binFile, registro);
            int indicTam = registro->tamanhoRegistro;

            if (registro->removido == '*') {   //o registro esta removido
                fseek(binFile, indicTam+5, SEEK_CUR);   //se o registro esta removido, apenas o pulo
            }
            else if (registro->removido == '-') {   //o registro pode ser manipulado
                if (!strcmp(nomeCampo, "idServidor")) {    //se o campo a ser buscado eh "idServidor"...
                    if (registro->idServidor == atoi(valorCampo)) {    //se o valor do campo no registro lido eh igual ao do dado como criterio de busca...

                        int diff = mudaRegistro(registro, nomeAtualiza, valorAtualiza);    //atualizo o registro (fora do arquivo)

                        if (diff < 0) {     //o registro novo eh maior do que o original
                            long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', binFile);    //marco o registro como REMOVIDO
                            adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo

                            long long temp = achaPosicaoInsere(binFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
                            if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado
                        }
                        else {
                            insereRegistro(binFile, registro); //gravo o registro atualizado no arquivo
                            for (int i = 0; i < diff; i++) {
                                fputc('@', binFile);    //completo o registro com lixo
                            }
                        }

                        break;  //ja que o numero do idServidor eh unico, se acharmos um igual nao precisaremos mais continuar procurando
                    }
                    else {
                        fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                    }
                }

                else if (!strcmp(nomeCampo, "salarioServidor")) {    //se o campo a ser buscado eh "salarioServidor"...
                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->salarioServidor == -1) {

                            int diff = mudaRegistro(registro, nomeAtualiza, valorAtualiza);    //atualizo o registro (fora do arquivo)

                            if (diff < 0) {     //o registro novo eh maior do que o original
                                long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                                fputc('*', binFile);    //marco o registro como REMOVIDO
                                adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                                completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo

                                long long temp = achaPosicaoInsere(binFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
                                if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado
                                fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                            }
                            else {
                                insereRegistro(binFile, registro); //gravo o registro atualizado no arquivo
                                for (int i = 0; i < diff; i++) {
                                    fputc('@', binFile);    //completo o registro com lixo
                                }
                            }

                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        if (registro->salarioServidor == atof(valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...

                            int diff = mudaRegistro(registro, nomeAtualiza, valorAtualiza);    //atualizo o registro (fora do arquivo)

                            if (diff < 0) {     //o registro novo eh maior do que o original
                                long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                                fputc('*', binFile);    //marco o registro como REMOVIDO
                                adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                                completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo

                                long long temp = achaPosicaoInsere(binFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
                                if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado
                                fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                            }
                            else {
                                insereRegistro(binFile, registro); //gravo o registro atualizado no arquivo
                                for (int i = 0; i < diff; i++) {
                                    fputc('@', binFile);    //completo o registro com lixo
                                }
                            }

                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else if (!strcmp(nomeCampo, "telefoneServidor")) {    //se o campo a ser buscado eh "telefoneServidor"...
                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->telefoneServidor[0] == '\0') {

                            int diff = mudaRegistro(registro, nomeAtualiza, valorAtualiza);    //atualizo o registro (fora do arquivo)

                            if (diff < 0) {     //o registro novo eh maior do que o original
                                long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                                fputc('*', binFile);    //marco o registro como REMOVIDO
                                adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                                completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo

                                long long temp = achaPosicaoInsere(binFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
                                if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado
                                fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                            }
                            else {
                                insereRegistro(binFile, registro); //gravo o registro atualizado no arquivo
                                for (int i = 0; i < diff; i++) {
                                    fputc('@', binFile);    //completo o registro com lixo
                                }
                            }

                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        if (!strcmp(registro->telefoneServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...

                            int diff = mudaRegistro(registro, nomeAtualiza, valorAtualiza);    //atualizo o registro (fora do arquivo)

                            if (diff < 0) {     //o registro novo eh maior do que o original
                                long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                                fputc('*', binFile);    //marco o registro como REMOVIDO
                                adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                                completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo

                                long long temp = achaPosicaoInsere(binFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
                                if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado
                                fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                            }
                            else {
                                insereRegistro(binFile, registro); //gravo o registro atualizado no arquivo
                                for (int i = 0; i < diff; i++) {
                                    fputc('@', binFile);    //completo o registro com lixo
                                }
                            }

                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else if (!strcmp(nomeCampo, "nomeServidor")) {    //se o campo a ser buscado eh "nomeServidor"...
                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->nomeServidor == NULL) {

                            int diff = mudaRegistro(registro, nomeAtualiza, valorAtualiza);    //atualizo o registro (fora do arquivo)

                            if (diff < 0) {     //o registro novo eh maior do que o original
                                long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                                fputc('*', binFile);    //marco o registro como REMOVIDO
                                adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                                completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo

                                long long temp = achaPosicaoInsere(binFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
                                if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado
                                fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                            }
                            else {
                                insereRegistro(binFile, registro); //gravo o registro atualizado no arquivo
                                for (int i = 0; i < diff; i++) {
                                    fputc('@', binFile);    //completo o registro com lixo
                                }
                            }

                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        if (registro->nomeServidor != NULL && !strcmp(registro->nomeServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...

                            int diff = mudaRegistro(registro, nomeAtualiza, valorAtualiza);    //atualizo o registro (fora do arquivo)

                            if (diff < 0) {     //o registro novo eh maior do que o original
                                long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                                fputc('*', binFile);    //marco o registro como REMOVIDO
                                adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                                completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo

                                long long temp = achaPosicaoInsere(binFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
                                if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado
                                fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                            }
                            else {
                                insereRegistro(binFile, registro); //gravo o registro atualizado no arquivo
                                for (int i = 0; i < diff; i++) {
                                    fputc('@', binFile);    //completo o registro com lixo
                                }
                            }

                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else if (!strcmp(nomeCampo, "cargoServidor")) {    //se o campo a ser buscado eh "cargoServidor"...
                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->cargoServidor == NULL) {

                            int diff = mudaRegistro(registro, nomeAtualiza, valorAtualiza);    //atualizo o registro (fora do arquivo)

                            if (diff < 0) {     //o registro novo eh maior do que o original
                                long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                                fputc('*', binFile);    //marco o registro como REMOVIDO
                                adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                                completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo

                                long long temp = achaPosicaoInsere(binFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
                                if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado
                                fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                            }
                            else {
                                insereRegistro(binFile, registro); //gravo o registro atualizado no arquivo
                                for (int i = 0; i < diff; i++) {
                                    fputc('@', binFile);    //completo o registro com lixo
                                }
                            }

                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        if (registro->cargoServidor != NULL && !strcmp(registro->cargoServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...

                            int diff = mudaRegistro(registro, nomeAtualiza, valorAtualiza);    //atualizo o registro (fora do arquivo)

                            if (diff < 0) {     //o registro novo eh maior do que o original
                                long long byteOffset = ftell(binFile);  //guardo o byte offset do registro a ser logicamente removido
                                fputc('*', binFile);    //marco o registro como REMOVIDO
                                adicionaLista(binFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                                completaLixo(binFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo

                                long long temp = achaPosicaoInsere(binFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
                                if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado
                                fseek(binFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                            }
                            else {
                                insereRegistro(binFile, registro); //gravo o registro atualizado no arquivo
                                for (int i = 0; i < diff; i++) {
                                    fputc('@', binFile);    //completo o registro com lixo
                                }
                            }

                        }
                        else {
                            fseek(binFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else {  //o usuario digitou errado o nome do campo
                    printf("Falha no processamento do arquivo.");
                    return;
                }
            }

            b = fgetc(binFile);
        }

        fseek(binFile, TAMPAG, SEEK_SET);    //volto o ponteiro de leitura para o inicio da segunda pagina de disco (a que inicia os registros de dados)
    }

    binarioNaTela1(binFile);

    //antes de fechar o arquivo, coloco seu status para '1'
    fseek(binFile, 0, SEEK_SET);  //coloco o ponteiro de escrita no primeiro byte do arquivo
    fputc('1', binFile);  //sobrescrevo o campo "status" do arquivo binario

    freeRegistro(registro);
    fclose(binFile);
}

/*
    Funcao de comparacao entre dois registros de
    dados, considerando seus campos idServidor.
    Utilizada como parametro da funcao qsort().
*/
int compare(const void *r1, const void *r2) {
    int f = ((regDados*)r1)->idServidor;
    int s = ((regDados*)r2)->idServidor;
    return (f > s) - (f < s);
}

/*
    Recebe como entrada um arquivo binario,
    anteriormente gerado por este programa,
    e cria um novo arquivo que contem todos
    os registros daquele arquivo, porém
    ordenados em funcao do campo idServidor.
    Aqueles que estavam logicamente removidos
    no arquivo de entrada NAO sao copiados
    para o arquivo de saida.
*/
void ordenaReg() {
    char inputFileName[51];   //vai guardar o nome do arquivo de entrada
    char outputFileName[51];   //vai guardar o nome do arquivo de saida
    regCabec *cabecalho = criaCabecalho();  //estrutura que sera utilizada para guardar os valores do registro de cabecalho
    regDados *registro = criaRegistro();  //estrutura que sera utilizada para guardar os registros lidos do arquivo binario

    scanf("%50s %50s", inputFileName, outputFileName);

    FILE *inFile = fopen(inputFileName, "rb");  //abro o arquivo binario para leitura
    FILE *outFile = fopen(outputFileName, "wb+");  //crio um novo arquivo binario para escrita

    if (inFile == NULL || outFile == NULL) {   //erro na abertura dos arquivos
        printf("Falha no carregamento do arquivo.");
        return;
    }

    leCabecalho(inFile, cabecalho);

    if (cabecalho->status == '0') {   //se o campo "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }

    fseek(inFile, TAMPAG, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)

    byte b = fgetc(inFile);

    if (feof(inFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
      printf("Registro inexistente.");
      return;
    }

    regDados *arquivo = calloc(THRESHOLD, sizeof(regDados)); //aloco espaco para o arquivo binario na memoria RAM
    int index = 0;  //iremos iterar no vetor de registros a partir desta variavel

    while (!feof(inFile)) {
        ungetc(b, inFile); //"devolvo" o byte lido para o arquivo binario
        leRegistro(inFile, registro);

        if (registro->removido == '-') {   //se o registro nao esta removido, ele pode ser manipulado
            if (index == THRESHOLD) { //se o numero de registros no arquivo excede o limite maximo proposto...
                arquivo = realloc(arquivo, 4*THRESHOLD);  //provavelmente nunca vai entrar aqui, mas foi colocado por precaucao
            }
            memcpy(arquivo+index, registro, sizeof(regDados));  //copio o registro lido para o vetor
            index++;
        }
        fseek(inFile, registro->tamanhoRegistro+5, SEEK_CUR);   //avanco o leitor do arquivo

        b = fgetc(inFile);
    }

    qsort(arquivo, index, sizeof(regDados), compare);   //ordeno os registros, em RAM, pelo idServidor

    cabecalho->topoLista = -1L;     //ja que o novo arquivo nao tem registros logicamente removidos
    insereCabecalho(outFile, cabecalho);
    fseek(outFile, TAMPAG, SEEK_CUR);  //vou para o inicio da segunda pagina de disco

    int tamAntigo = 0;
    for (int i = 0; i < index; i++) {
        arquivo[i].encadeamentoLista = -1L; //ja que o novo arquivo nao tem registros logicamente removidos
        calculaTamanho(arquivo+i);  //recalculo o tamanho do registro (util no caso em que ele era o ultimo de uma pagina de disco)
        checaFimPagina(outFile, arquivo+i, tamAntigo);
        insereRegistro(outFile, arquivo+i);
        tamAntigo = arquivo[i].tamanhoRegistro;
    }

    binarioNaTela1(outFile);

    //antes de fechar o arquivo, coloco seu status para '1'
    fseek(outFile, 0, SEEK_SET);  //coloco o ponteiro de escrita no primeiro byte do arquivo
    fputc('1', outFile);  //sobrescrevo o campo "status" do arquivo binario

    free(cabecalho);
    free(registro);
    free(arquivo);
    fclose(inFile);
    fclose(outFile);
}

/*
    Compara dois arquivos binarios, gerados
    pela funcionalidade 7 deste programa,
    e copia para um arquivo de saida uma
    uniao dos registros desses arquivos.
*/
void merging() {
    char inputF1Name[51];   //vai guardar o nome do arquivo de entrada 1
    char inputF2Name[51];   //vai guardar o nome do arquivo de entrada 2
    char outputFileName[51];   //vai guardar o nome do arquivo de saida
    regCabec *cabecalho = criaCabecalho();  //estrutura que sera utilizada para guardar os valores do registro de cabecalho
    regDados *registro1 = criaRegistro();  //estrutura que sera utilizada para guardar os registros lidos do arquivo binario de entrada 1
    regDados *registro2 = criaRegistro();  //estrutura que sera utilizada para guardar os registros lidos do arquivo binario de entrada 2

    scanf("%50s %50s %50s", inputF1Name, inputF2Name, outputFileName);

    FILE *firstFile = fopen(inputF1Name, "rb");  //abro o arquivo binario 1 para leitura
    FILE *secondFile = fopen(inputF2Name, "rb");  //abro o arquivo binario 2 para leitura
    FILE *outFile = fopen(outputFileName, "wb+");  //crio um novo arquivo binario para escrita

    if (firstFile == NULL || secondFile == NULL || outFile == NULL) {   //erro na abertura dos arquivos
        printf("Falha no carregamento do arquivo.");
        return;
    }

    leCabecalho(secondFile, cabecalho);

    if (cabecalho->status == '0') {   //se o campo "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }

    leCabecalho(firstFile, cabecalho);

    if (cabecalho->status == '0') {   //se o campo "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }

    insereCabecalho(outFile, cabecalho);
    fseek(outFile, TAMPAG, SEEK_CUR);  //vou para o inicio da segunda pagina de disco

    fseek(firstFile, TAMPAG, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)
    fseek(secondFile, TAMPAG, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)

    byte b1 = fgetc(firstFile);
    byte b2 = fgetc(secondFile);

    int tamAntigo = 0;
    int indicTam1 = 0;
    int indicTam2 = 0;

    while (!feof(firstFile) && !feof(secondFile)) {
        ungetc(b1, firstFile); //"devolvo" o byte lido para o arquivo binario 1
        ungetc(b2, secondFile); //"devolvo" o byte lido para o arquivo binario 2
        leRegistro(firstFile, registro1);
        leRegistro(secondFile, registro2);

        if (registro1->idServidor < registro2->idServidor) {
            indicTam1 = registro1->tamanhoRegistro;
            calculaTamanho(registro1);
            checaFimPagina(outFile, registro1, tamAntigo);
            insereRegistro(outFile, registro1);
            tamAntigo = registro1->tamanhoRegistro;
            fseek(firstFile, indicTam1+5, SEEK_CUR);   //avanco o leitor do arquivo 1
        }
        else if (registro2->idServidor < registro1->idServidor) {
            indicTam2 = registro2->tamanhoRegistro;
            calculaTamanho(registro2);
            checaFimPagina(outFile, registro2, tamAntigo);
            insereRegistro(outFile, registro2);
            tamAntigo = registro2->tamanhoRegistro;
            fseek(secondFile, indicTam2+5, SEEK_CUR);   //avanco o leitor do arquivo 2
        }
        else {
            indicTam1 = registro1->tamanhoRegistro;
            calculaTamanho(registro1);
            checaFimPagina(outFile, registro1, tamAntigo);
            insereRegistro(outFile, registro1);
            tamAntigo = registro1->tamanhoRegistro;
            fseek(firstFile, indicTam1+5, SEEK_CUR);   //avanco o leitor do arquivo 1
            fseek(secondFile, registro2->tamanhoRegistro+5, SEEK_CUR);   //avanco o leitor do arquivo 2
        }

        b1 = fgetc(firstFile);
        b2 = fgetc(secondFile);
    }

    while (!feof(firstFile)) {
        ungetc(b1, firstFile); //"devolvo" o byte lido para o arquivo binario
        leRegistro(firstFile, registro1);

        indicTam1 = registro1->tamanhoRegistro;
        calculaTamanho(registro1);
        checaFimPagina(outFile, registro1, tamAntigo);
        insereRegistro(outFile, registro1);
        tamAntigo = registro1->tamanhoRegistro;

        fseek(firstFile, registro1->tamanhoRegistro+5, SEEK_CUR);   //avanco o leitor do arquivo 1
        b1 = fgetc(firstFile);
    }

    while (!feof(secondFile)) {
        ungetc(b2, secondFile); //"devolvo" o byte lido para o arquivo binario
        leRegistro(secondFile, registro2);

        indicTam2 = registro2->tamanhoRegistro;
        calculaTamanho(registro2);
        checaFimPagina(outFile, registro2, tamAntigo);
        insereRegistro(outFile, registro2);
        tamAntigo = registro2->tamanhoRegistro;

        fseek(secondFile, registro2->tamanhoRegistro+5, SEEK_CUR);   //avanco o leitor do arquivo 2
        b2 = fgetc(secondFile);
    }

    binarioNaTela1(outFile);

    //antes de fechar o arquivo, coloco seu status para '1'
    fseek(outFile, 0, SEEK_SET);  //coloco o ponteiro de escrita no primeiro byte do arquivo
    fputc('1', outFile);  //sobrescrevo o campo "status" do arquivo binario

    free(cabecalho);
    free(registro1);
    free(registro2);
    fclose(firstFile);
    fclose(secondFile);
    fclose(outFile);
}

/*
    Compara dois arquivos binarios, gerados
    pela funcionalidade 7 deste programa,
    e copia para um arquivo de saida uma
    intersecao dos registros desses arquivos.
*/
void matching() {
    char inputF1Name[51];   //vai guardar o nome do arquivo de entrada 1
    char inputF2Name[51];   //vai guardar o nome do arquivo de entrada 2
    char outputFileName[51];   //vai guardar o nome do arquivo de saida
    regCabec *cabecalho = criaCabecalho();  //estrutura que sera utilizada para guardar os valores do registro de cabecalho
    regDados *registro1 = criaRegistro();  //estrutura que sera utilizada para guardar os registros lidos do arquivo binario de entrada 1
    regDados *registro2 = criaRegistro();  //estrutura que sera utilizada para guardar os registros lidos do arquivo binario de entrada 2

    scanf("%50s %50s %50s", inputF1Name, inputF2Name, outputFileName);

    FILE *firstFile = fopen(inputF1Name, "rb");  //abro o arquivo binario 1 para leitura
    FILE *secondFile = fopen(inputF2Name, "rb");  //abro o arquivo binario 2 para leitura
    FILE *outFile = fopen(outputFileName, "wb+");  //crio um novo arquivo binario para escrita

    if (firstFile == NULL || secondFile == NULL || outFile == NULL) {   //erro na abertura dos arquivos
        printf("Falha no carregamento do arquivo.");
        return;
    }

    leCabecalho(secondFile, cabecalho);

    if (cabecalho->status == '0') {   //se o campo "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }

    leCabecalho(firstFile, cabecalho);

    if (cabecalho->status == '0') {   //se o campo "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }

    insereCabecalho(outFile, cabecalho);
    fseek(outFile, TAMPAG, SEEK_CUR);  //vou para o inicio da segunda pagina de disco

    fseek(firstFile, TAMPAG, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)
    fseek(secondFile, TAMPAG, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)

    byte b1 = fgetc(firstFile);
    byte b2 = fgetc(secondFile);

    int tamAntigo = 0;
    int indicTam1 = 0;

    while (!feof(firstFile) && !feof(secondFile)) {
        ungetc(b1, firstFile); //"devolvo" o byte lido para o arquivo binario 1
        ungetc(b2, secondFile); //"devolvo" o byte lido para o arquivo binario 2
        leRegistro(firstFile, registro1);
        leRegistro(secondFile, registro2);

        if (registro1->idServidor < registro2->idServidor) {
            fseek(firstFile, registro1->tamanhoRegistro+5, SEEK_CUR);   //avanco o leitor do arquivo 1
        }
        else if (registro2->idServidor < registro1->idServidor) {
            fseek(secondFile, registro2->tamanhoRegistro+5, SEEK_CUR);   //avanco o leitor do arquivo 2
        }
        else {
            indicTam1 = registro1->tamanhoRegistro;
            calculaTamanho(registro1);
            checaFimPagina(outFile, registro1, tamAntigo);
            insereRegistro(outFile, registro1);
            tamAntigo = registro1->tamanhoRegistro;
            fseek(firstFile, indicTam1+5, SEEK_CUR);   //avanco o leitor do arquivo 1
            fseek(secondFile, registro2->tamanhoRegistro+5, SEEK_CUR);   //avanco o leitor do arquivo 2
        }

        b1 = fgetc(firstFile);
        b2 = fgetc(secondFile);
    }

    binarioNaTela1(outFile);

    //antes de fechar o arquivo, coloco seu status para '1'
    fseek(outFile, 0, SEEK_SET);  //coloco o ponteiro de escrita no primeiro byte do arquivo
    fputc('1', outFile);  //sobrescrevo o campo "status" do arquivo binario

    free(cabecalho);
    free(registro1);
    free(registro2);
    fclose(firstFile);
    fclose(secondFile);
    fclose(outFile);
}

/*
    Cria um arquivo de indice secundario
    fortemente ligado, indexado pelo campo
    "nomeServidor", para um arquivo binario
    anteriormente gerado por este programa.
    Os registros logicamente removidos e os
    registros que possuirem valor nulo para
    o campo nao serao referenciados nele.
*/
void criaArqIndices() {
	char inputFileName[51];   //vai guardar o nome do arquivo de entrada
    char outputFileName[51];   //vai guardar o nome do arquivo de saida
    regCabec *cabecalho = criaCabecalho();  //estrutura que sera utilizada para guardar os valores do registro de cabecalho do arquivo binario de entrada
    regDados *registro = criaRegistro();  //estrutura que sera utilizada para guardar os registros lidos do arquivo binario de entrada
    regCabecI *cabecInd = criaCabecalhoIndice();  //inicializo o cabecalho do arquivo de indices
    regDadosI *regisInd = criaRegistroIndice();  //inicializo um registro de dados do arquivo de indices
    SuperLista listaRAM = criaSuperLista(); //crio uma estrutura de dados que ira guardar todos os registros de dados do arquivo de indice em memoria RAM

    scanf("%50s %50s", inputFileName, outputFileName);

    FILE *dataFile = fopen(inputFileName, "rb");  //abro o arquivo binario de entrada para leitura
    FILE *indexFile = fopen(outputFileName, "wb+");  //crio um novo arquivo binario para escrita (o de indices)

    if (dataFile == NULL || indexFile == NULL) {   //erro na abertura dos arquivos
        printf("Falha no carregamento do arquivo.");
        return;
    }

    leCabecalho(dataFile, cabecalho);

    if (cabecalho->status == '0') {   //se o campo "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }

    fseek(dataFile, TAMPAG, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)

    byte b = fgetc(dataFile);

    if (feof(dataFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
      printf("Registro inexistente.");
      return;
    }

	while (!feof(dataFile)) {
        ungetc(b, dataFile); //"devolvo" o byte lido para o arquivo binario 1
        leRegistro(dataFile, registro);

        if (registro->removido == '-') {
            //o registro pode ser considerado
            if (registro->nomeServidor != NULL) {
                //o registro eh considerado
                regisInd->byteOffset = ftell(dataFile);    //guardo o byte offset dele
                strcpy(regisInd->chaveBusca, registro->nomeServidor);   //guardo a chave de busca (o nome do servidor)
                adicionaSuperLista(listaRAM, regisInd);
                cabecInd->nroRegistros++;
            }
        }

        fseek(dataFile, registro->tamanhoRegistro+5, SEEK_CUR); //vou para o proximo registro de dados
        b = fgetc(dataFile);
    }

    reescreveArquivoIndice(indexFile, cabecInd, listaRAM);
    binarioNaTela1(indexFile);

    free(cabecalho);
    free(registro);
    free(cabecInd);
    free(regisInd);
    freeSuperLista(listaRAM);
    fclose(indexFile);
    fclose(dataFile);
}

/*
    Busca, com auxilio do arquivo de indice, registros que
    satisfacam uma busca determinada pelo usuario (sempre
    levando em conta o campo "nomeServidor") mostrando-os
    na tela assim que sao encontrados.
    Tambem eh mostrado, ao final da execucao, quantas
    paginas de disco foram acessadas ao todo.
*/
void buscaIndice() {


    char dataFileName[51];   //vai guardar o nome do arquivo de dados
    char indexFileName[51];   //vai guardar o nome do arquivo de indices
    char nomeServidor[120];
    regCabecI *cabecalhoI = criaCabecalhoIndice();
    regCabec *cabecalho = criaCabecalho();  //estrutura que sera utilizada para guardar os valores do registro de cabecalho do arquivo binario de entrada
    regDados *registro = criaRegistro();  //estrutura que sera utilizada para guardar os registros lidos do arquivo binario de entrada

    scanf("%50s %50s nomeServidor %[^\r\n]", dataFileName, indexFileName, nomeServidor);

    FILE *dataFile = fopen(dataFileName, "rb");  //abro o arquivo binario de entrada para leitura
    FILE *indexFile = fopen(indexFileName, "rb");  //crio um novo arquivo binario para escrita (o de indices)

    if (dataFile == NULL || indexFile == NULL) {   //erro na abertura dos arquivos
        printf("Falha no carregamento do arquivo.");
        return;
    }

    leCabecalho(dataFile, cabecalho);
    leCabecalhoIndice(indexFile, cabecalhoI);

    if (cabecalho->status == '0' || cabecalhoI->status == '0') {   //se o campo "status" for '0', entao o arquivo esta inconsistente
        printf("Falha no processamento do arquivo.");
        return;
    }

    fseek(dataFile, TAMPAG, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)

    byte b = fgetc(dataFile);

    if (feof(dataFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
        printf("Registro inexistente.");
        return;
    }

    b = fgetc(indexFile);

    if (feof(indexFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
        printf("Registro inexistente.");
        return;
    }

    //carrega arquivo de indice em um vetor
    regDadosI* dadosI = carregaIndiceVetor(indexFile);

    int comeco, tam;
    long long* posDados = buscaRegistroIndice(dadosI, nomeServidor, 0, cabecalhoI->nroRegistros, &comeco, &tam);
    regDados* r = criaRegistro();

    if (posDados == NULL) {
        printf("Registro inexistente.");
        return;
    }

    for (int i = comeco; i >= 0; i--) {
        fseek(dataFile, posDados[i], SEEK_SET);
        leRegistro(dataFile, r);
        mostraRegistroMeta(cabecalho, r);
    }

    for (int i = comeco+1; i < tam; i++) {
        fseek(dataFile, posDados[i], SEEK_SET);
        leRegistro(dataFile, r);
        mostraRegistroMeta(cabecalho, r);
    }

    fseek(indexFile, 0, SEEK_END);

    printf("Número de páginas de disco para carregar o arquivo de índice: %d\n", ((int)ftell(indexFile)/32000)+1);
    printf("Número de páginas de disco para acessar o arquivo de dados: %d\n", tam);

    free(cabecalhoI);
    free(cabecalho);
    freeRegistro(registro);
    if (posDados != NULL) free(posDados);
    freeRegistro(r);
    fclose(dataFile);
    fclose(indexFile);
}

/*
    Atualiza os registros do arquivo de indice
    que estao em RAM, removendo entre eles
    aquele que referencia um registro que acabou
    de ser logicamente removido.
    Parametros:
        SuperLista sl - indices a serem considerados
        char *nomeServidor - valor deste campo do
    registro logicamente removido
        long long byteOffset - byte offset do registro
    logicamente removido
*/
void atualizaRemIndice(SuperLista sl, char *nomeServidor, long long byteOffset, regCabecI *cabecalho) {
    regDadosI *removido = criaRegistroIndice();
    strcpy(removido->chaveBusca, nomeServidor);
    removido->byteOffset = byteOffset;
    removeSuperLista(sl, removido);
    cabecalho->nroRegistros--;
    free(removido);
}

/*
    Busca no arquivo binario registros que satisfacam
    um criterio de busca determinado pelo usuario,
    removendo-os logicamente assim que sao encontrados.
    Apos isso, o indice referente a esse registro eh
    removido do arquivo de indices.
    O usuario deve informar quantas buscas diferentes
    deseja fazer perante uma mesma execucao. Alem disso,
    deve informar, em cada uma delas, o nome e o valor
    do campo a ser buscado.
*/
void removeRegInd() {
    char dataFileName[51];   //vai guardar o nome do arquivo de dados
    char indexFileName[51]; //vai guardar o nome do arquivo de indices
    int n;      //numero de remocoes a serem realizadas
    char nomeCampo[51];    //campo a ser considerado na busca
    byte valorCampo[200];    //valor a ser considerado na busca
    regDados *registro = criaRegistro();  //estrutura que será utilizada para guardar os registros lidos do arquivo binario
    regCabecI *cabecInd = criaCabecalhoIndice();  //estrutura que guardara o cabecalho do arquivo de indices

    scanf("%50s %50s", dataFileName, indexFileName);
    scanf("%d", &n);

    FILE *dataFile = fopen(dataFileName, "rb+");  //abre o arquivo "dataFileName" para leitura e escrita binária
    FILE *indexFile = fopen(indexFileName, "rb");  //abre o arquivo "dataFileName" para leitura binária

    if (dataFile == NULL || indexFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    if (fgetc(dataFile) == '0') {   //se o byte "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }
    ungetc('0', dataFile);  //como o arquivo foi aberto para escrita, seu status deve ser '0'

    if (fgetc(indexFile) == '0') {   //se o byte "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }
    ungetc('0', indexFile);  //como o arquivo foi aberto para escrita, seu status deve ser '0'

    leCabecalhoIndice(indexFile, cabecInd);
    SuperLista indiceRAM = carregaIndiceLista(indexFile);  //carrego o arquivo de indices para a RAM
    fclose(indexFile);  //a partir de agora, so trabalho com o indice em memoria

    fseek(dataFile, TAMPAG, SEEK_CUR);  //pulo o registro de cabecalho do arquivo de dados

    for (int i = 0; i < n; i++) {
        memset(nomeCampo, 0, 51);   //limpo o vetor (setando tudo para 0)
        memset(valorCampo, 0, 200);  //limpo o vetor (setando tudo para 0)

        scanf("%50s %[^\r\n]", nomeCampo, valorCampo);  //paro de ler antes da quebra de linha

        byte b = fgetc(dataFile);

        if (feof(dataFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
          printf("Registro inexistente.");
          return;
        }

        while (!feof(dataFile)) {
            ungetc(b, dataFile); //"devolvo" o byte lido para o arquivo binario
            leRegistro(dataFile, registro);
            int indicTam = registro->tamanhoRegistro;

            if (registro->removido == '*') {   //o registro esta removido
                fseek(dataFile, indicTam+5, SEEK_CUR);   //se o registro esta removido, apenas o pulo
            }
            else if (registro->removido == '-') {   //o registro pode ser manipulado
                if (!strcmp(nomeCampo, "idServidor")) {    //se o campo a ser buscado eh "idServidor"...
                    if (registro->idServidor == atoi(valorCampo)) {    //se o valor do campo no registro lido eh igual ao do dado como criterio de busca...
                        long long byteOffset = ftell(dataFile);  //guardo o byte offset do registro a ser logicamente removido
                        fputc('*', dataFile);    //marco o registro como REMOVIDO
                        adicionaLista(dataFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                        completaLixo(dataFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                        if (registro->nomeServidor != NULL) atualizaRemIndice(indiceRAM, registro->nomeServidor, byteOffset, cabecInd);   //atualiza a remocao no indice
                        break;  //ja que o numero do idServidor eh unico, se acharmos um igual nao precisaremos mais continuar procurando
                    }
                    else {
                        fseek(dataFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                    }
                }

                else if (!strcmp(nomeCampo, "salarioServidor")) {    //se o campo a ser buscado eh "salarioServidor"...
                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->salarioServidor == -1) {
                            long long byteOffset = ftell(dataFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', dataFile);    //marco o registro como REMOVIDO
                            adicionaLista(dataFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(dataFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            if (registro->nomeServidor != NULL) atualizaRemIndice(indiceRAM, registro->nomeServidor, byteOffset, cabecInd);   //atualiza a remocao no indice
                            fseek(dataFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(dataFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        if (registro->salarioServidor == atof(valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                            long long byteOffset = ftell(dataFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', dataFile);    //marco o registro como REMOVIDO
                            adicionaLista(dataFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(dataFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            if (registro->nomeServidor != NULL) atualizaRemIndice(indiceRAM, registro->nomeServidor, byteOffset, cabecInd);   //atualiza a remocao no indice
                            fseek(dataFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(dataFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else if (!strcmp(nomeCampo, "telefoneServidor")) {    //se o campo a ser buscado eh "telefoneServidor"...
                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->telefoneServidor[0] == '\0') {
                            long long byteOffset = ftell(dataFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', dataFile);    //marco o registro como REMOVIDO
                            adicionaLista(dataFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(dataFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            if (registro->nomeServidor != NULL) atualizaRemIndice(indiceRAM, registro->nomeServidor, byteOffset, cabecInd);   //atualiza a remocao no indice
                            fseek(dataFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(dataFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        if (!strcmp(registro->telefoneServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                            long long byteOffset = ftell(dataFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', dataFile);    //marco o registro como REMOVIDO
                            adicionaLista(dataFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(dataFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            if (registro->nomeServidor != NULL) atualizaRemIndice(indiceRAM, registro->nomeServidor, byteOffset, cabecInd);   //atualiza a remocao no indice
                            fseek(dataFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(dataFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else if (!strcmp(nomeCampo, "nomeServidor")) {    //se o campo a ser buscado eh "nomeServidor"...
                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->nomeServidor == NULL) {
                            long long byteOffset = ftell(dataFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', dataFile);    //marco o registro como REMOVIDO
                            adicionaLista(dataFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(dataFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            if (registro->nomeServidor != NULL) atualizaRemIndice(indiceRAM, registro->nomeServidor, byteOffset, cabecInd);   //atualiza a remocao no indice
                            fseek(dataFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(dataFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        char *valorSemAspas = strtok(valorCampo, "\"");     //retiro as aspas da string (para efeitos de comparacao)

                        if (registro->nomeServidor != NULL && !strcmp(registro->nomeServidor, valorSemAspas)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                            long long byteOffset = ftell(dataFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', dataFile);    //marco o registro como REMOVIDO
                            adicionaLista(dataFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(dataFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            if (registro->nomeServidor != NULL) atualizaRemIndice(indiceRAM, registro->nomeServidor, byteOffset, cabecInd);   //atualiza a remocao no indice
                            fseek(dataFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(dataFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else if (!strcmp(nomeCampo, "cargoServidor")) {    //se o campo a ser buscado eh "cargoServidor"...
                    char *valorSemAspas = strtok(valorCampo, "\"");     //retiro as aspas da string (para efeitos de comparacao)

                    if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                        if (registro->cargoServidor == NULL) {
                            long long byteOffset = ftell(dataFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', dataFile);    //marco o registro como REMOVIDO
                            adicionaLista(dataFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(dataFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            if (registro->nomeServidor != NULL) atualizaRemIndice(indiceRAM, registro->nomeServidor, byteOffset, cabecInd);   //atualiza a remocao no indice
                            fseek(dataFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(dataFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                    else {  //o valor a ser buscado nao eh nulo
                        if (registro->cargoServidor != NULL && !strcmp(registro->cargoServidor, valorSemAspas)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                            long long byteOffset = ftell(dataFile);  //guardo o byte offset do registro a ser logicamente removido
                            fputc('*', dataFile);    //marco o registro como REMOVIDO
                            adicionaLista(dataFile, byteOffset, indicTam);    //adiciono o registro a lista de removidos
                            completaLixo(dataFile);  //sobreescrevo todos os campos do registro (menos encadeamentoLista) com lixo
                            if (registro->nomeServidor != NULL) atualizaRemIndice(indiceRAM, registro->nomeServidor, byteOffset, cabecInd);   //atualiza a remocao no indice
                            fseek(dataFile, indicTam+4, SEEK_CUR);    //vou para o proximo registro (+4 por conta dos bytes do indicador de tamanho, que ele mesmo nao contabiliza)
                        }
                        else {
                            fseek(dataFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
                        }
                    }
                }

                else {  //o usuario digitou errado o nome do campo
                    printf("Falha no processamento do arquivo.");
                    return;
                }
            }

            if (registro->nomeServidor != NULL) free(registro->nomeServidor);
            if (registro->cargoServidor != NULL) free(registro->cargoServidor);
            b = fgetc(dataFile);
        }

        fseek(dataFile, TAMPAG, SEEK_SET);    //volto o ponteiro de leitura para o inicio da segunda pagina de disco (a que inicia os registros de dados)
    }

    indexFile = fopen(indexFileName, "wb+");  //abre o arquivo "dataFileName" para escrita binária

    if (indexFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    reescreveArquivoIndice(indexFile, cabecInd, indiceRAM);
    binarioNaTela1(indexFile);

    //antes de fechar os arquivo, coloco seus status para '1'
    fseek(dataFile, 0, SEEK_SET);  //coloco o ponteiro de escrita no primeiro byte do arquivo
    fputc('1', dataFile);  //sobrescrevo o campo "status" do arquivo binario

    free(registro);
    free(cabecInd);
    freeSuperLista(indiceRAM);
    fclose(dataFile);
    fclose(indexFile);
}

/*
    Atualiza os registros do arquivo de indice
    que estao em RAM, adicionando entre eles
    aquele que referencia um registro que acabou
    de ser adicionado ao arquivo principal.
    Parametros:
        SuperLista sl - indices a serem considerados
        char *nomeServidor - valor deste campo do
    registro adicionado
        long long byteOffset - byte offset do registro
    adicionado
*/
void atualizaAdcIndice(SuperLista sl, char *nomeServidor, long long byteOffset, regCabecI *cabecalho) {
    regDadosI *adicionado = criaRegistroIndice();
    strcpy(adicionado->chaveBusca, nomeServidor);
    adicionado->byteOffset = byteOffset;
    adicionaSuperLista(sl, adicionado);
    cabecalho->nroRegistros++;
    free(adicionado);
}

/*
    Adiciona novos registros ao arquivo binario,
    reaproveitando os espacos deixados pelos
    registros logicamente removidos. Para tanto,
    percorre-se a lista de removidos ate que se
    encontre um espaco em que caiba o novo registro.
    Caso nao se encontre nenhum espaco grande o
    suficiente, o registro eh escrito no final do
    arquivo. Logo depois de adicionado, se o registro
    nao tem campo "nomeServidor" NULO, uma referencia
    a ele eh criada no arquivo de indices.
    O usuario deve informar quantas insercoes
    diferentes deseja fazer perante uma mesma execucao.
    Alem disso, deve informar, em cada uma delas, o
    valor dos campos do registro.
*/
void adicionaRegInd() {
    char dataFileName[51];   //vai guardar o nome do arquivo de dados
    char indexFileName[51]; //vai guardar o nome do arquivo de indices
    int n;      //numero de insercoes a serem realizadas
    regDados *registro = criaRegistro();  //ira guardar os dados fornecidos pelo usuario
    regCabecI *cabecInd = criaCabecalhoIndice();  //estrutura que guardara o cabecalho do arquivo de indices
    char buffer[201];
    long long posUltimoReg = -1;  //ira guardar o byte offset do ultimo registro do arquivo

    scanf("%50s %50s", dataFileName, indexFileName);
    scanf("%d", &n);

    FILE *dataFile = fopen(dataFileName, "rb+");  //abre o arquivo "dataFileName" para leitura e escrita binária
    FILE *indexFile = fopen(indexFileName, "rb");  //abre o arquivo "dataFileName" para leitura binária

    if (dataFile == NULL || indexFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    if (fgetc(dataFile) == '0') {   //se o byte "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }
    ungetc('0', dataFile);  //como o arquivo foi aberto para escrita, seu status deve ser '0'

    if (fgetc(indexFile) == '0') {   //se o byte "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
    }
    ungetc('0', indexFile);  //como o arquivo foi aberto para escrita, seu status deve ser '0'

    leCabecalhoIndice(indexFile, cabecInd);
    SuperLista indiceRAM = carregaIndiceLista(indexFile);  //carrego o arquivo de indices para a RAM
    fclose(indexFile);  //a partir de agora, so trabalho com o indice em memoria

    for (int i = 0; i < n; i++) {

        scanf("%d ", &(registro->idServidor));   //pego o valor de idServidor (esse campo nao pode ser nulo)

        //pego o valor de salarioServidor
        memset(buffer, 0, 201);   //limpo o buffer
        char c = fgetc(stdin);
        if (c == 'N') {     //o campo eh nulo
            registro->salarioServidor = -1;
            fseek(stdin, 4, SEEK_CUR);  //pulo os caracteres "ULO "
        }
        else {
            ungetc(c, stdin);   //devolvo o char lido para a entrada padrao
            scanf("%s ", buffer);
            registro->salarioServidor = atof(buffer);
        }

        //pego o valor de telefoneServidor
        memset(buffer, 0, 201);   //limpo o buffer
        c = fgetc(stdin);
        if (c == 'N') {     //o campo eh nulo
            registro->telefoneServidor[0] = '\0';
            fseek(stdin, 4, SEEK_CUR);  //pulo os caracteres "ULO "
        }
        else {
            ungetc(c, stdin);   //devolvo o char lido para a entrada padrao
            scanf("%*[\"]%[^\"]", buffer);  //le o campo, desconsiderando as aspas
            fseek(stdin, 2, SEEK_CUR);  //pulo os caracteres "\" "
            strcpy(registro->telefoneServidor, buffer);
        }

        //pego o valor de nomeServidor
        memset(buffer, 0, 201);   //limpo o buffer
        c = fgetc(stdin);
        if (c == 'N') {     //o campo eh nulo
            registro->nomeServidor = NULL;
            fseek(stdin, 4, SEEK_CUR);  //pulo os caracteres "ULO "
        }
        else {
            ungetc(c, stdin);   //devolvo o char lido para a entrada padrao
            scanf("%*[\"]%[^\"]", buffer);  //le o campo, desconsiderando as aspas
            fseek(stdin, 2, SEEK_CUR);  //pulo os caracteres "\" "
            registro->nomeServidor = malloc(100*sizeof(char));
            strcpy(registro->nomeServidor, buffer);
            registro->tamCampo4 = strlen(registro->nomeServidor) + 2;     //conta tambem o '\0' e a tag do campo
        }

        //pego o valor de cargoServidor
        memset(buffer, 0, 201);   //limpo o buffer
        c = fgetc(stdin);
        if (c == 'N') {     //o campo eh nulo
            registro->cargoServidor = NULL;
            fseek(stdin, 4, SEEK_CUR);  //pula os caracteres "ULO "
        }
        else {
            ungetc(c, stdin);   //devolvo o char lido para a entrada padrao
            scanf("%*[\"]%[^\"]", buffer);  //le o campo, desconsiderando as aspas
            fseek(stdin, 2, SEEK_CUR);  //pulo os caracteres "\" "
            registro->cargoServidor = malloc(100*sizeof(char));
            strcpy(registro->cargoServidor, buffer);
            registro->tamCampo5 = strlen(registro->cargoServidor) + 2;     //conta tambem o '\0' e a tag do campo
        }

        calculaTamanho(registro);

        char *nomeServidor;
        if (registro->nomeServidor != NULL) {
            nomeServidor = malloc(120*sizeof(char));
            strcpy(nomeServidor, registro->nomeServidor);
        }
        else {
            nomeServidor = NULL;
        }

        int tamReg = registro->tamanhoRegistro;

        long long temp = achaPosicaoInsereSeek(dataFile, registro, posUltimoReg);  //adiciona o registro novo ao arquivo
        if (temp != -1) posUltimoReg = temp;    //se o valor retornado pela funcao atualiza o BO do ultimo registro da lista, entao este valor deve ser guardado

        if (nomeServidor != NULL) {   //se o campo "nomeServidor" nao eh nulo, podemos adicionar esse registro ao arquivo de indices
            fseek(dataFile, -(tamReg+5), SEEK_CUR);  //volto ao inicio do registro inserido
            atualizaAdcIndice(indiceRAM, nomeServidor, ftell(dataFile), cabecInd);   //atualiza a adicao no indice
        }

        if (nomeServidor != NULL) free(nomeServidor);
    }

    indexFile = fopen(indexFileName, "wb+");  //abre o arquivo "dataFileName" para escrita binária

    if (indexFile == NULL) {   //erro na abertura do arquivo
      printf("Falha no processamento do arquivo.");
      return;
    }

    reescreveArquivoIndice(indexFile, cabecInd, indiceRAM);
    binarioNaTela1(indexFile);

    //antes de fechar os arquivo, coloco seus status para '1'
    fseek(dataFile, 0, SEEK_SET);  //coloco o ponteiro de escrita no primeiro byte do arquivo
    fputc('1', dataFile);  //sobrescrevo o campo "status" do arquivo binario

    free(registro);
    free(cabecInd);
    freeSuperLista(indiceRAM);
    fclose(dataFile);
    fclose(indexFile);
}

/*
*/
void comparaBuscas() {

  char dataFileName[51];   //vai guardar o nome do arquivo de dados
  char indexFileName[51];   //vai guardar o nome do arquivo de indices
  char nomeServidor[120];
  regCabecI *cabecalhoI = criaCabecalhoIndice();
  regCabec *cabecalho = criaCabecalho();  //estrutura que sera utilizada para guardar os valores do registro de cabecalho do arquivo binario de entrada
  regDados *registro = criaRegistro();  //estrutura que sera utilizada para guardar os registros lidos do arquivo binario de entrada

  scanf("%50s %50s nomeServidor %[^\r\n] ", dataFileName, indexFileName, nomeServidor);

  FILE *dataFile = fopen(dataFileName, "rb");  //abro o arquivo binario de entrada para leitura
  FILE *indexFile = fopen(indexFileName, "rb");  //crio um novo arquivo binario para escrita (o de indices)

  if (dataFile == NULL || indexFile == NULL) {   //erro na abertura dos arquivos
      printf("Falha no carregamento do arquivo.");
      return;
  }

  leCabecalho(dataFile, cabecalho);
  leCabecalhoIndice(indexFile, cabecalhoI);

  if (cabecalho->status == '0' || cabecalhoI->status == '0') {   //se o campo "status" for '0', entao o arquivo esta inconsistente
      printf("Falha no processamento do arquivo.");
      return;
  }

  fseek(dataFile, TAMPAG, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)

  byte b = fgetc(dataFile);

  if (feof(dataFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
      printf("Registro inexistente.");
      return;
  }

  b = fgetc(indexFile);

  if (feof(indexFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
      printf("Registro inexistente.");
      return;
  }

  //carrega arquivo de indice em um vetor
  regDadosI* dadosI = carregaIndiceVetor(indexFile);

  int comeco, tam;
  long long* posDados = buscaRegistroIndice(dadosI, nomeServidor, 0, cabecalhoI->nroRegistros, &comeco, &tam);
  regDados* r = criaRegistro();

  fseek(dataFile,0,SEEK_SET);

  char fileName[51];   //vai guardar o nome do arquivo a ser aberto
  char nomeCampo[51];    //campo a ser considerado na busca
  byte valorCampo[100];    //valor a ser considerado na busca
  int acessosPagina = 0; //vai contar a quantidade de acessos a paginas de disco no decorrer da execucao

  strcpy(fileName,dataFileName);
  strcpy(nomeCampo,"nomeServidor");
  strcpy(valorCampo,nomeServidor);

  leCabecalho(dataFile, cabecalho);
  acessosPagina++;

  if (cabecalho->status == '0') {   //se o campo "status" for '0', entao o arquivo esta inconsistente
    printf("Falha no processamento do arquivo.");
    return;
  }

  fseek(dataFile, TAMPAG, SEEK_CUR);  //vou para a segunda pagina de disco (que contem os registros de dados)

  b = fgetc(dataFile);

  if (feof(dataFile)) {   //se o primeiro byte da primeira pagina de disco contendo os registros de dados for o final do arquivo, entao nao existem registros para serem mostrados
    printf("Registro inexistente.");
    return;
  }

  int achou = 0;    //indicara se pelo menos um registro foi achado


  printf("*** Realizando a busca sem o auxílio de índice\n");

  while (!feof(dataFile)) {

      ungetc(b, dataFile); //"devolvo" o byte lido para o arquivo binario
      if (ftell(dataFile)%TAMPAG == 0) acessosPagina++;   //o cabecote esta no comeco de uma pagina de disco

      leRegistro(dataFile, registro);
      int indicTam = registro->tamanhoRegistro;

      if (registro->removido == '-') {   //o registro pode ser manipulado
          if (!strcmp(nomeCampo, "idServidor")) {    //se o campo a ser buscado eh "idServidor"...
              if (registro->idServidor == atoi(valorCampo)) {    //se o valor do campo no registro lido eh igual ao do dado como criterio de busca...
                  mostraRegistroMeta(cabecalho, registro);
                  achou = 1;
                  break;  //ja que o numero do idServidor eh unico, se acharmos um igual nao precisaremos mais continuar procurando
              }
          }
          else if (!strcmp(nomeCampo, "salarioServidor")) {    //se o campo a ser buscado eh "salarioServidor"...
              if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                  if (registro->salarioServidor == -1) {
                      mostraRegistroMeta(cabecalho, registro);
                      achou = 1;
                  }
              }
              else {  //o valor a ser buscado nao eh nulo
                  if (registro->salarioServidor == atof(valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                      mostraRegistroMeta(cabecalho, registro);
                      achou = 1;
                  }
              }
          }
          else if (!strcmp(nomeCampo, "telefoneServidor")) {    //se o campo a ser buscado eh "telefoneServidor"...
              if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                  if (registro->telefoneServidor[0] == '\0') {
                      mostraRegistroMeta(cabecalho, registro);
                      achou = 1;
                  }
              }
              else {  //o valor a ser buscado nao eh nulo
                  if (!strcmp(registro->telefoneServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                      mostraRegistroMeta(cabecalho, registro);
                      achou = 1;
                  }
              }
          }
          else if (!strcmp(nomeCampo, "nomeServidor")) {    //se o campo a ser buscado eh "nomeServidor"...
              if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                  if (registro->nomeServidor == NULL) {
                      mostraRegistroMeta(cabecalho, registro);
                      achou = 1;
                  }
              }
              else {  //o valor a ser buscado nao eh nulo
                  if (registro->nomeServidor != NULL && !strcmp(registro->nomeServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                      mostraRegistroMeta(cabecalho, registro);
                      achou = 1;
                  }
              }
          }
          else if (!strcmp(nomeCampo, "cargoServidor")) {    //se o campo a ser buscado eh "cargoServidor"...
              if (!strcmp(valorCampo, "NULO")) {  //se o valor a ser buscado eh nulo...
                  if (registro->cargoServidor == NULL) {
                      mostraRegistroMeta(cabecalho, registro);
                      achou = 1;
                  }
              }
              else {  //o valor a ser buscado nao eh nulo
                  if (registro->cargoServidor != NULL && !strcmp(registro->cargoServidor, valorCampo)) {    //se o valor lido eh igual ao do dado como criterio de busca...
                      mostraRegistroMeta(cabecalho, registro);
                      achou = 1;
                  }
              }
          }
          else {  //o usuario digitou errado o nome do campo
              printf("Falha no processamento do arquivo.");
              return;
          }
      }

      if (registro->nomeServidor != NULL) free(registro->nomeServidor);
      if (registro->cargoServidor != NULL) free(registro->cargoServidor);
      fseek(dataFile, indicTam+5, SEEK_CUR);   //vou para o proximo registro (+5 por conta dos bytes do indicador de tamanho e do campo "removido")
      b = fgetc(dataFile);
  }

    if (!achou) {
        printf("Registro inexistente.\n");
        printf("Número de páginas de disco acessadas: %d", acessosPagina);
    }

    else {
        printf("Número de páginas de disco acessadas: %d", acessosPagina);

    }

    printf("\n*** Realizando a busca com o auxílio de um índice \n");

    int acessosI = 1;
    long long antByteOffset = -1;

    if (posDados == NULL) {
        printf("Registro inexistente.\n");
    }

    else {

        for (int i = comeco; i >= 0; i--) {
            fseek(dataFile, posDados[i], SEEK_SET);
            leRegistro(dataFile, r);
            mostraRegistroMeta(cabecalho, r);
            if ((posDados[i]/32000) != (antByteOffset/32000)) acessosI++;
            antByteOffset = posDados[i];
        }

        for (int i = comeco+1; i < tam; i++) {
            fseek(dataFile, posDados[i], SEEK_SET);
            leRegistro(dataFile, r);
            mostraRegistroMeta(cabecalho, r);
            if ((posDados[i]/32000) != (antByteOffset/32000)) acessosI++;
            antByteOffset = posDados[i];
        }

    }

    fseek(indexFile, 0, SEEK_END);

    printf("Número de páginas de disco para carregar o arquivo de índice: %d\n", ((int)ftell(indexFile)/32000)+1);
    printf("Número de páginas de disco para acessar o arquivo de dados: %d\n", acessosI);

    printf("\nA diferença no número de páginas de disco acessadas: %d", acessosPagina - acessosI);

    free(cabecalhoI);
    free(cabecalho);
    free(posDados);
    free(registro);
    free(r);
    fclose(dataFile);
    fclose(indexFile);


}

/*
  Cuida da execucao do programa.
*/
int main() {
    int opt; //vai guardar a opcao digitada pelo usuario
    scanf("%d ", &opt);

    switch (opt) {
        case 1:
            leCSV();
            break;
        case 2:
            mostraBin();
            break;
        case 3:
            buscaReg();
            break;
        case 4:
            removeReg();
            break;
        case 5:
            adicionaReg();
            break;
        case 6:
            atualizaReg();
            break;
        case 7:
            ordenaReg();
            break;
        case 8:
            merging();
            break;
        case 9:
            matching();
            break;
        case 10:
        	criaArqIndices();
        	break;
        case 11:
        	buscaIndice();
        	break;
        case 12:
            removeRegInd();
            break;
        case 13:
            adicionaRegInd();
            break;
        case 14:
            comparaBuscas();
            break;
        default:
            printf("Opção inválida!\n");
    }

    return 0;
}
