#include "..\\HoBaram\\HoTextFile.h"

char *HoTextLoginMessage[18] =  {" Estableciendo Conexi\363n.... ",  //  Estableciendo Conexión.... 
				" Imposible conectar al servidor ",
				"    Por favor, reingrese su identificaci\363n   ",  //     Por favor, reingrese su identificación   
				" Por favor, reingrese su palabra clave  ",
				" Identificaci\363n bloqueada, por favor contactar al soporte del Mundo Priston ",  //  Identificación bloqueada, por favor contactar al soporte del Mundo Priston 
				"  La identificaci\363n ya est\341 ingresada  ",  //   La identificación ya está ingresada  
				"  La versi\363n no concuerda   ",  //   La versión no concuerda   
				"      El servidor est\341 completo       ",  //       El servidor está completo       
				" Se perdi\363 la conexi\363n con el servidor ",  //  Se perdió la conexión con el servidor 
				"   Expir\363 el tiempo disponible  ",  //    Expiró el tiempo disponible  
				"   Expir\363 el tiempo disponible  ",  //    Expiró el tiempo disponible  
				" Borrando la cuenta en el servidor ",
				 " El servidor est\341 sobrecargado, por favor intente m\341s tarde. ",  //  El servidor está sobrecargado, por favor intente más tarde. 
				" Por favor intente m\341s tarde.... ",  //  Por favor intente más tarde.... 
				" Excedi\363 los intentos de ingresar la palabra clave, por favor contactar al soporte. ",  //  Excedió los intentos de ingresar la palabra clave, por favor contactar al soporte. 
				" No se permite usar caracteres especiales en la palabra clave. ",
				};

char *HoTextSelectMessage[12] = {" Desea borrar? ",
				  " Continuar este relato? ",
				  " Excedi\363 el l\355mite de caracteres. ",  //  Excedió el límite de caracteres. 
				  " Creaci\363n completada. Confirmar? ", //3  //  Creación completada. Confirmar? 
				  " Ingresar el nombre del personaje ", //4
				  " El nombre ya existe ", //5
				  "  Seleccionar un personaje   ", //6
				  "    Seleccionar una tribu     ", //7
				  "    Seleccionar una clase     ", //8
				  "     Seleccionar un rostro     ", //9
				  " Imposible borrar al maestro del clan ", //10
				  " Imposible localizar informaci\363n del clan " //11  //  Imposible localizar información del clan 
				};

char *HoTextTempscronInfo[5] = {" Tempskron es una tribu que pone las habilidades f\355sicas y la t\351cnica en su m\341s alta estima, y es tambi\351n la \372nica tribu con la capacidad t\351cnica para usar maquinaria. ",  //  Tempskron es una tribu que pone las habilidades físicas y la técnica en su más alta estima, y es también la única tribu con la capacidad técnica para usar maquinaria. 
								/* Maybe " La tribu Tempskron es renombrada por sus habilidades f\355sicas y t\351cnicas eficientes... "*/	//  La tribu Tempskron es renombrada por sus habilidades físicas y técnicas eficien
                                " Renombrado por sus habilidades con las m\341quinas, son la \372nica clase que puede usar aparatos de ataque. ",  //  Renombrado por sus habilidades con las máquinas, son la única clase que puede usar aparatos de ataque. 
								" Los Luchadores nacen con la pasi\363n del combate, y son excelentes con hachas en mano. ",  //  Los Luchadores nacen con la pasión del combate, y son excelentes con hachas en mano. 
								" Maestros de las armas largas, usan su amplio campo visaul para aumentar sus habilidades con las armas hasta hacerlas mortales. ",
								" Especialistas en largas distancias y combate anti-a\351reo , son expertos con arcos y ballestas de toda clase. "};  //  Especialistas en largas distancias y combate anti-aéreo , son expertos con arcos y ballestas de toda clase. 



char *HoTextMoryon = " Morion ";
char *HoTextMoryonInfo[5] = {
	" Los Morions son un pueblo inocente y amable que reside en el norte de Mundo Priston. Los Morions son disciplinados tanto f\355sica como espiritualmente. Tambi\351n entrenan en magia, lo que los llev\363 a la prosperidad por a\361os. Con la gran combinaci\363n de su religi\363n y la magia, los Morions son capaces de usar sus poderes sagrados libremente. ",  //  Los Morions son un pueblo inocente y amable que reside en el norte de Mundo Priston. Los Morions son disciplinados tanto física como espiritualmente. También entrenan en magia, lo que los llevó a la prosperidad por años. Con la gran combinación de su religión y la magia, los Morions son capaces de usar sus poderes sagrados libremente. 
    " Con sus pesadas armaduras y poderosas espadas, los Caballeros representan a la m\341s alta clase guerrera en Morion. Ellos usan sus poderes sagrados para atacar f\355sicamente a los enemigos. ",  //  Con sus pesadas armaduras y poderosas espadas, los Caballeros representan a la más alta clase guerrera en Morion. Ellos usan sus poderes sagrados para atacar físicamente a los enemigos. 
	" Las Atalantas son las guerreras que usan lanzas y jabalinas. Su verdadera fuerza aparece cuando usan esas armas. Con su habilidad de ataque, su mezcla de ataque m\341gico/f\355sico es el fundamento de su gran fuerza. ",  //  Las Atalantas son las guerreras que usan lanzas y jabalinas. Su verdadera fuerza aparece cuando usan esas armas. Con su habilidad de ataque, su mezcla de ataque mágico/físico es el fundamento de su gran fuerza. 
	" Las Sacerdotisas son m\351dicas de campo sagradas y usan sus poderes sagrados para atacar enemigos o curar las heridas de sus camaradas. Aunque su poder de sanaci\363n es muy fuerte, ellas son, de alguna manera, f\355sicamente vulnerables. ",  //  Las Sacerdotisas son médicas de campo sagradas y usan sus poderes sagrados para atacar enemigos o curar las heridas de sus camaradas. Aunque su poder de sanación es muy fuerte, ellas son, de alguna manera, físicamente vulnerables. 
	" Los Magos hacen converger toda su fuerza m\341gica para aumentar su habilidad de ataque. Con su gran entendimiento de las propiedades de cada elemento, los magos son la clase que estudia las habilidades m\341gicas. Tambi\351n se los reconoce por su fuerza para afectar tierras y estrellas. ",  //  Los Magos hacen converger toda su fuerza mágica para aumentar su habilidad de ataque. Con su gran entendimiento de las propiedades de cada elemento, los magos son la clase que estudia las habilidades mágicas. También se los reconoce por su fuerza para afectar tierras y estrellas. 
};


char *HoText_PartyMessage = " ha enviado un requerimiento de grupo ";

char *HoTextTempskron = " Tempskron ";

char *HoTextFriendNear = " [Reciente] ";
char *HoTextFriendList = " [Amigos] %d/%d ";
char *HoTextFriendRefuse = " [Ignorado] %d/%d ";