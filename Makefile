CC = gcc
CFLAGS = -Iincludes  # Certifique-se que este caminho está correto para os seus arquivos de cabeçalho

# Diretórios
BINDIR = bin
OBJDIR = objetos
INSTALLDIR = /bin/concordia

# Cria os diretórios de objetos, binários e instalação se não existirem
$(shell mkdir -p $(OBJDIR))
$(shell mkdir -p $(BINDIR))
$(shell sudo mkdir -p $(INSTALLDIR))

# Alvo principal
all: install

# Compilar os binários
$(BINDIR)/mydeamon: $(OBJDIR)/deamon.o $(OBJDIR)/task.o $(OBJDIR)/groupTask.o $(OBJDIR)/aux.o
	$(CC) -o $@ $^

$(BINDIR)/concordia-ativar: $(OBJDIR)/concordia-ativar.o
	$(CC) -o $@ $^

$(BINDIR)/concordia-desativar: $(OBJDIR)/concordia-desativar.o
	$(CC) -o $@ $^

$(BINDIR)/concordia-enviar: $(OBJDIR)/concordia-enviar.o
	$(CC) -o $@ $^

$(BINDIR)/concordia-ler: $(OBJDIR)/concordia-ler.o
	$(CC) -o $@ $^

$(BINDIR)/concordia-listar: $(OBJDIR)/concordia-listar.o
	$(CC) -o $@ $^

$(BINDIR)/concordia-remover: $(OBJDIR)/concordia-remover.o
	$(CC) -o $@ $^

$(BINDIR)/concordia-responder: $(OBJDIR)/concordia-responder.o
	$(CC) -o $@ $^

$(BINDIR)/concordia-grupo-criar: $(OBJDIR)/concordia-grupo-criar.o
	$(CC) -o $@ $^

$(BINDIR)/concordia-grupo-remover: $(OBJDIR)/concordia-grupo-remover.o
	$(CC) -o $@ $^

$(BINDIR)/concordia-grupo-listar: $(OBJDIR)/concordia-grupo-listar.o 
	$(CC) -o $@ $^

$(BINDIR)/concordia-grupo-destinatario-adicionar: $(OBJDIR)/concordia-grupo-destinatario-adicionar.o 
	$(CC) -o $@ $^

$(BINDIR)/concordia-grupo-destinatario-remover: $(OBJDIR)/concordia-grupo-destinatario-remover.o 
	$(CC) -o $@ $^

# Regra genérica para compilar arquivos .c para .o
$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Alvo para instalar os binários em /bin/concordia
install: $(BINDIR)/mydeamon $(BINDIR)/concordia-ativar $(BINDIR)/concordia-desativar $(BINDIR)/concordia-enviar $(BINDIR)/concordia-ler $(BINDIR)/concordia-listar $(BINDIR)/concordia-remover $(BINDIR)/concordia-responder $(BINDIR)/concordia-grupo-criar $(BINDIR)/concordia-grupo-remover $(BINDIR)/concordia-grupo-listar $(BINDIR)/concordia-grupo-destinatario-adicionar $(BINDIR)/concordia-grupo-destinatario-remover
	sudo cp $(BINDIR)/mydeamon $(BINDIR)/concordia-* $(INSTALLDIR)/
	sudo chmod 755 $(INSTALLDIR)/concordia-*
	sudo chown deamon:deamon $(INSTALLDIR)/mydeamon
	sudo chmod 700 $(INSTALLDIR)/mydeamon

# Limpeza dos arquivos compilados e instalados
clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/*
	sudo rm -f $(INSTALLDIR)/concordia-*

.PHONY: all install clean
