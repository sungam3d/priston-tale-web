#include "..\\HoBaram\\HoTextFile.h"

char *HoTextLoginMessage[18] =  {"Estabelecendo a conex\343o....",  // Estabelecendo a conexão....
				"N\343o foi poss\355vel se conectar ao servidor",  // Não foi possível se conectar ao servidor
				"     Digite sua ID novamente    ",
				" Digite sua senha novamente  ",
				"ID bloqueada, visite nosso site",
				"  Essa ID j\341 est\341 logada  ",  //   Essa ID já está logada  
				"  Vers\343o n\343o compat\355vel   ",  //   Versão não compatível   
				"      Servidor cheio       ",
				"Conex\343o perdida com o servidor",  // Conexão perdida com o servidor
				"   Tempo limite excedido  ",
				"   Tempo limite excedido   ",
				" Deletando a conta no servidor ",
				 "Servidor sobrecarregado!!!, por favor, tente mais tarde.",
				" Por favor, tente mais tarde... ",
				"Senha incorreta por mais de 3 vezes.",
				"Voc\352 n\343o pode usar esses caracteres na senha.",  // Você não pode usar esses caracteres na senha.
				"allow",
				};

char *HoTextSelectMessage[12] = {"Voc\352 deseja deletar?",  // Você deseja deletar?
				  "Continuar esta aventura?",
				  "Limite de personagens.",
				  "Cria\347\343o completa. Confirmar?", //3  // Criação completa. Confirmar?
				  "Digite um nome", //4
				  "Esse nome j\341 existe", //5  // Esse nome já existe
				  "  Selecione um personagem   ", //6
				  "    Selecione uma tribo     ", //7
				  "    Selecione uma classe     ", //8
				  "     Selecione a face     ", //9
				  "Imposs\355vel deletar L\355der do Clan", //10  // Impossível deletar Líder do Clan
				  "Dados do Clan n\343o recebidos" //11  // Dados do Clan não recebidos
				};

char *HoTextTempscronInfo[5] = {" Os Tempskrons s\343o a civiliza\347\343o mais avan\347ada do continente de Priston, sendo capazes de controlar todos os tipos de equipamentos, at\351 mesmo os mais modernos.",  //  Os Tempskrons são a civilização mais avançada do continente de Priston, sendo capazes de controlar todos os tipos de equipamentos, até mesmo os mais modernos.
								/* Maybe " A tribo dos Tempskrons ,famosa por suas t\351nicas de combate e sua grande habilidade com v\341ios tipos de armas"*/	//  A tribo dos Tempskrons ,famosa por suas ténicas de combate e sua grande habilid
                                " Conhecida pela sua tecnologia avan\347ada, a tribo dos Tempskrons \351 a un\355ca capaz de usar m\341quinas modernas a favor do combate.",  //  Conhecida pela sua tecnologia avançada, a tribo dos Tempskrons é a uníca capaz de usar máquinas modernas a favor do combate.
								" Os Lutadores s\343o guerreiros natos, sua especialidade \351 a luta com Machados...",  //  Os Lutadores são guerreiros natos, sua especialidade é a luta com Machados...
								" Personagens eficientes, podem usar suas t\351cnicas a favor de uma luta de qualidade,.",  //  Personagens eficientes, podem usar suas técnicas a favor de uma luta de qualidade,.
								" Mestres na arte da luta a dist\342ncia, s\343o as pioneiras com o Arco e Flecha."};  //  Mestres na arte da luta a distância, são as pioneiras com o Arco e Flecha.



char *HoTextMoryon = "Morion";
char *HoTextMoryonInfo[5] = {
	"Os Morions residem na parte Norte do continente de Priston. S\343o ing\352nuos e d\363ceis. Sua cultura prospera. Valorizam ao extremo ambos os treinamentos - o espiritual e o f\355sico. S\343o os pioneiros na arte de usar as for\347as da magia e da ora\347\343o, a favor do combate.",  // Os Morions residem na parte Norte do continente de Priston. São ingênuos e dóceis. Sua cultura prospera. Valorizam ao extremo ambos os treinamentos - o espiritual e o físico. São os pioneiros na arte de usar as forças da magia e da oração, a favor do combate.
        "Com suas armaduras reluzentes e suas espadas poderosas, representam a classe Top dos Guerreiros dos Morions. Eles usam a for\347a sagrada para derrotar os inimigos do mal.",  // Com suas armaduras reluzentes e suas espadas poderosas, representam a classe Top dos Guerreiros dos Morions. Eles usam a força sagrada para derrotar os inimigos do mal.
	"Atalantas s\343o profissionais na arte das Lan\347as. Sua t\351cnica \351 perfeita quando o assunto \351 arremesso. Com seus poderes magn\355ficos, o estrago dos ataques \351 ainda maior.",  // Atalantas são profissionais na arte das Lanças. Sua técnica é perfeita quando o assunto é arremesso. Com seus poderes magníficos, o estrago dos ataques é ainda maior.
	"As Sacerdotisas s\343o as rainhas da magia, usando sua forca para combater o mal, seja atacando, seja curando. Mesmo com seu alto poder de cura, s\343o consideravelmente vulner\341veis ",  // As Sacerdotisas são as rainhas da magia, usando sua forca para combater o mal, seja atacando, seja curando. Mesmo com seu alto poder de cura, são consideravelmente vulneráveis 
	"Magos convertem toda a for\347a da natureza em ataques brutais. Com conhecimento anormal acerca dos elementos, s\343o os melhores quando o assunto \351 Magia. Tamb\351m s\343o famosos por controlar a terra e as estrelas.",  // Magos convertem toda a força da natureza em ataques brutais. Com conhecimento anormal acerca dos elementos, são os melhores quando o assunto é Magia. Também são famosos por controlar a terra e as estrelas.
};


char *HoText_PartyMessage = "enviou um pedido de grupo";

char *HoTextTempskron = "Tempskron";

char *HoTextFriendNear = "[Recentes]";
char *HoTextFriendList = "[Amigos] %d/%d";
char *HoTextFriendRefuse = "[Ignore] %d/%d";