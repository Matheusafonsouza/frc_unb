# Sobre o que é
Esse reposiório contempla a entrega do primeiro trabalho da matéria de Fundamento de Redes de Computadores. Abaixo você irá ver como compilar e executar o dado trabalho:

## Para compilar o projeto

~~~BASH
make compile
~~~

## Para executar

~~~BASH
`para subir o server que vai receber o arquivo`
make run-server IP_SERVER="127.0.0.1" PORT_SERVER="8001"
`para subir o client que vai enviar o arquivo`
make run-client IP_SERVER="127.0.0.1" PORT_SERVER2="8002" IP_CLIENT="127.0.0.1" PORT_CLIENT="8001"
~~~
