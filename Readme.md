# Como testar o Projeto

* Deve ter a biblioteca FUSE instalada (Use esse comando no terminal para instalar ela):

  ```bash
    sudo apt install g++ libfuse3-dev
  ```

* Crie as pastas necessárias no diretório do projeto:

  ```bash
  mkdir -p pasta_real ponto_montagem


* Altere a seguinte linha do código:

   * ###### define REAL_DIR "/home/vinicius_castro_filaretti/MEGA/Faculdade 5 Periodo/Sistemas Operacionais/Códigos/Trabalho 2/pasta_real"
 
    Para o diretório da pasta "**pasta_real**"


* ## Abra 3 terminais

    1. No primeiro o código será compilado e executado, com os seguintes comandos:
     
        ```bash
          g++ -Wall -o meu_fuse main.cpp -lfuse3
          ./meu_fuse ./ponto_montagem -f
        ```

    2. Nesse você vai colocar o arquivo de log, com o comando:
 
        ```bash
          tail -f /tmp/fuse_log.txt
        ```

    3. No ultimo terminal, você vai testar os comando para criar, escrever, ler e renomear os arquivos, com os seguintes comandos:
 
        ```bash
          echo "Teste de criação" > ./ponto_montagem/arquivo_teste.txt # CRIAR ARQUIVO
        ```
        ```bash
          cat ./ponto_montagem/arquivo_teste.txt # LER ARQUIVO
        ```
        ```bash
          echo "Nova linha adicionada" >> ./ponto_montagem/arquivo_teste.txt # ESCREVER NO ARQUIVO
        ```
        ```bash
          mv ./ponto_montagem/arquivo_teste.txt ./ponto_montagem/arquivo_renomeado.txt # RENOMEAR ARQUIVO
        ```
        ```bash
          ls ./ponto_montagem # LISTAR ARQUIVOS
        ```
    * E a cada comando, olha o arquivo log no segundo terminal.
