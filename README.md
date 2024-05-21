# Segurança de Sistemas Informáticos
## Projeto Concordia

## Configurações Visudo

Criar o user `deamon` (responsavel por correr o deamon.c)
Alterar com o comando `sudo visudo`:

- `deamon ALL=(ALL) NOPASSWD: /usr/sbin/groupadd, /usr/sbin/usermod, /usr/sbin/groupdel, /usr/bin/gpasswd, /usr/sbin/deluser`

## Iniciar o programa

- `make` (core)
- `sudo -u deamon /bin/concordia/mydeamon` (core)

## Comandos concordia possiveis
- `/bin/concordia/concordia-ativar`
- `/bin/concordia/concordia-enviar <user_id> <msg>`
- `/bin/concordia/concordia-enviar G_<group_id> <msg>`
- `/bin/concordia/concordia-listar`
- `/bin/concordia/concordia-listar [-a]`
- `/bin/concordia/concordia-ler <msg_id>`
- `/bin/concordia/concordia-responder <msg_id> <msg>` 
- `/bin/concordia/concordia-remover <msg_id>`
- `/bin/concordia/concordia-grupo-criar <nome_do _grupo>`
- `/bin/concordia/concordia-grupo-remover <nome_do _grupo>`
- `/bin/concordia/concordia-grupo-listar <nome_do _grupo>`
- `/bin/concordia/concordia-grupo-destinatario-adicionar <user_id> <nome_do _grupo>`
- `/bin/concordia/concordia-grupo-destinatario-remover <user_id> <nome_do _grupo>`
- `/bin/concordia/concordia-desativar`


## Terminar 

- `make clean`  

## Verificar se deamon 
- `ps aux | grep mydeamon`

- `sudo pkill mydeamon`