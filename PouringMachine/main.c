/*
 * teste_4bits.c
 *
 * Created: 31/07/2020 23:56:41
 * Author : Vitor Wagner, Mateus Lopes e Arthur Marqueze
 */ 

//................................................................
//Inclusão de bibliotecas

#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/pgmspace.h> 
#include <avr/interrupt.h>
#include "LCD.h"
#define F_CPU 16000000UL //Clock

//...................................................................
//Definição de portas

#define A0 PORTD3
#define A1 PORTD4
#define B0 PORTD5
#define B1 PORTD6
#define C0 PORTD7
#define C1 PORTB3
#define SP PORTD2
#define CILA PORTB0
#define CILB PORTB1
#define CILC PORTB2

#define UP_BUT PORTD0
#define DOWN_BUT PORTD1
#define START_STOP_BUT PORTC4
#define PAUSE_BUT PORTC3
#define ENTER_BUT PORTC5
#define PE_BUT PORTC2
//..................................................................
//Definições das funções do switch case

enum func {configura, aguarda, prende, enche, retorna};
enum func funcoes;

//.................................................................
//Definição de alguns parametros

int tempo_envase;
int tamanho_lote;
int contador_lotes = 0;
char c_cont_lotes[16];
volatile unsigned char flag = 0;

//.................................................................
//Função setup

void setup (void)
{
	DDRB = 0b11110111; //Definir entradas e saídas das portas B
	DDRC = 0b00000011; //Definir entradas e saídas das portas C
	DDRD = 0b00000000; //Definir entradas e saídas das portas D
	
	set_bit(PORTB,CILC);   //abre o cilindro C
	clr_bit(PORTB,CILA);  //fecha o cilindro A
	clr_bit(PORTB,CILB);  //fecha o cilindro B
	
	inic_LCD_4bits();  //inicializa LCD 4 Bits

}


ISR(PCINT1_vect)		//funcao de interrupcao do botão PARADA DE EMERGENCIA
{
	if (flag == 0)		//caso seja apertado o botão
	{
		main ();		//volta para o main()
		flag = 1;
	}
	
	else                //caso o botão não esteja apertado
	{
		for(;;);		//continua normalmente
		flag = 0;
	}
}



void senha()
{
	char senha_interna[5] = "1234";		//Definição da senha interna do usuário
	char senha_lida_teclado[5];			//Senha que será digitada pelo usuário
	int k = 0;							//variável para o loop
	char str_k[16];						//variável (char) para visualizar no LCD
	char tecla;							//variável
	cmd_LCD(0x01,0);					//comando do LCD para limpar
	cmd_LCD(0x80,0);					//comando do LCD para mover o cursor para a 1 linha
	escreve_LCD("Bem vindo!");			//escreve no LCD
	_delay_ms(5000);
	cmd_LCD(0x01,0);
	cmd_LCD(0x80,0);					//comando do LCD para mover o cursor para a 1 linha
	escreve_LCD("Digite a senha");		//escreve no LCD
	_delay_ms(1000);
	for(int contador = 0;contador<4;contador++) //como a senha é de 4 digitos, cria-se um contador para monitorar quantos digitos foram digitados
	{
		
		while (rd_bit(PINC,ENTER_BUT) != 0) //caso seja apertado o botão enter
		{
			cmd_LCD(0xC0,0);			//comando do LCD para mover para a segunda linha
			itoa(k,str_k,10);			//transforma int em char para mostrar no LCD
			escreve_LCD(str_k);
			if (rd_bit(PIND,DOWN_BUT)==0)	//caso seja apertado o botão para baixo		
			{
				if (k > 0) {			//para evitar casos de numeros negativos
					k = k - 1;			//subtrai um
					cmd_LCD(0xC0,0);
					itoa(k,str_k,10);	//transforma int em char para mostrar no LCD
					escreve_LCD(str_k);
					tecla = k + '0';	//como k é um int, para armazenar em um char, deve-se acrescentar +'0'
				}
			}
			_delay_ms(2000);
			if (rd_bit(PIND,UP_BUT)==0)  //caso seja apertado o botão para cima
			{
				if (k < 9) {			//para evitar casos maiores que 9
					k = k + 1;			//adiciona 1 no valor de k
					cmd_LCD(0xC0,0);	
					itoa(k,str_k,10);	//transforma o valor de k em um char
					escreve_LCD(str_k);	//escreve o char no LCD
					tecla = k + '0';	//como k é um int, para armazenar em um char, deve-se acrescentar +'0'
				}
			}
		}
		k = 0;							//volta a ser zero quando pressionado enter para o entendimento do usuário que seu botão foi registrado
		_delay_ms(2000);
		senha_lida_teclado[contador] = tecla;	//armazena o valor de tecla no vetor na posição onde está o contador
	}
	senha_lida_teclado[5] = '\0';
	_delay_ms(5000);
	if (strcmp(senha_interna,senha_lida_teclado) == 0) //compara a senha interna com a senha digitada pelo usuário
	{
		cmd_LCD(0x01,0);
		cmd_LCD(0x80,0);
		escreve_LCD("Senha Correta");
		_delay_ms(20000);
		funcoes = configura;  //caso as senhas sejam iguais, irá para o case configura
	}
	else
	{
		cmd_LCD(0x01,0);
		cmd_LCD(0x80,0);
		escreve_LCD("Senha incorreta");
		_delay_ms(20000);
		senha();  //caso contrário, irá voltar a função senha
	}
}

void configurar()
{
	int i = 1;					//variável de contador para o tempo de envase
	int j = 1;					//variável de contador para numero de pecas	
	char str_i[16];				//criado para armazenar o numero do tempo de envae
	char str_j[16];				//criado para armazenar o numero de pecas
	cmd_LCD(0x01,0);
	cmd_LCD(0x80,0);
	escreve_LCD("Configuracao");
	_delay_ms(10000);
	cmd_LCD(0x01,0);
	cmd_LCD(0x80,0);
	escreve_LCD("Tempo de envase");
	cmd_LCD(0xC0,0);
	itoa(i,str_i,10);
	escreve_LCD(str_i);
	cmd_LCD(0xC4,0);					//posiciona o cursos na 4 posição da segunda linha
	escreve_LCD("segundos");
	while (rd_bit(PINC,ENTER_BUT) != 0) //enquanto o botão de enter não for pressionado significa que o usuário ainda está interagindo procurando o tempo que deseja
	{
		if (rd_bit(PIND,DOWN_BUT)==0)	//caso seja pressionado o botão inferior, o número do tempo em segundos diminui.
		{  
			if (i > 1) {				//caso o número seja maior que um, para evitar casos negativos e 0
				i = i - 1;
				cmd_LCD(0xC0,0);
				itoa(i,str_i,10);
				escreve_LCD(str_i);
			}
		}
		_delay_ms(1000);
		if (rd_bit(PIND,UP_BUT)==0) //caso seja pressionado o botão superior, o número do tempo em segundos aumenta
		{
			if (i < 99) {			//caso o número seja menor que 99, para evitar casos acima
				i = i + 1;
				cmd_LCD(0xC0,0);
				itoa(i,str_i,10);
				escreve_LCD(str_i);
			}
		}
		_delay_ms(1000);
	}
	_delay_ms(2000);
	tempo_envase = i;				//define o tempo de envase como a variável i controlada pelo usuário.
	cmd_LCD(0x01,0);
	cmd_LCD(0x80,0);
	escreve_LCD("Tamanho Lote");
	cmd_LCD(0xC0,0);
	itoa(j,str_j,10);
	escreve_LCD(str_j);
	cmd_LCD(0xC4,0);
	escreve_LCD("pecas");
	while (rd_bit(PINC,ENTER_BUT) != 0)	//enquanto o botão de enter não for pressionado significa que o usuário ainda está interagindo procurando o numero de pecas que deseja
	{
		if (rd_bit(PIND,DOWN_BUT)==0) //caso seja pressionado o botão inferior, o número de pecas diminui.
		{
			if (j > 1) {			//para evitar caso o numero de peças for numeros negativos e 0
				j = j - 1;
				cmd_LCD(0xC0,0);
				itoa(j,str_j,10);
				escreve_LCD(str_j);
			}
		}
		_delay_ms(2000);
		if (rd_bit(PIND,UP_BUT)==0) //caso seja pressionado o botão superior, o número de pecas aumenta.
		{
			if (j < 24) {			//para evitar caso o número de peças seja maior que 24
				j = j + 1;
				cmd_LCD(0xC0,0);
				itoa(j,str_j,10);
				escreve_LCD(str_j);
			}
		}
		_delay_ms(2000);
	}
	_delay_ms(2000);
	tamanho_lote = j;				//define o tamanho de pecas do lote como a variável j, controlada pelo usuário
	cmd_LCD(0x01,0);
	cmd_LCD(0x80,0);
	escreve_LCD("START p/ iniciar");
	while(rd_bit(PINC,START_STOP_BUT) != 0)		//enquanto o botão START não for pressionado, a máquina continuará esperando
	{
	}
	_delay_ms(2000);
	funcoes = aguarda;				//caso seja clicado start, vai para o case aguarda
}

void loop()
{
	switch(funcoes)				
	{
		case configura:
			configurar();
		break;
		
		case aguarda:
			if (contador_lotes < tamanho_lote)				//caso o contador de lotes já realizados seja maior do que o digitado pelo usuário
			{
				cmd_LCD(0x80,0);
				escreve_LCD("Aguardando peca  ");
				itoa(contador_lotes,c_cont_lotes,10);
				cmd_LCD(0xC0,0);
				escreve_LCD("Num. Pecas:");
				escreve_LCD(c_cont_lotes);
				
				set_bit(PORTB,CILC);					//abre o cilindro C
				clr_bit(PORTB,CILA);					//fecha o cilindro A
				clr_bit(PORTB,CILB);					//fecha o cilindro B
				if(((rd_bit(PIND,A0)) == 0) && ((rd_bit(PIND,B0)) == 0) && ((rd_bit(PINB,C1)) == 0) && ((rd_bit(PIND,SP)) == 0)) //caso os sensores A0,B0,C1,SP estejam ativos
				{
					while (rd_bit(PINC,PAUSE_BUT) == 0)		//Caso o botão de PAUSE sejá pressionado, permanecerá nesse loop
					{	
					}
					funcoes = prende;					//quando os sensores estão ativos, vai para o case "prende"
				}
			}
			else
			{
				cmd_LCD(0x01,0);
				cmd_LCD(0x80,0);
				escreve_LCD("Operacao");
				cmd_LCD(0xC0,0);
				escreve_LCD("Finalizada");
				_delay_ms(10000);
				contador_lotes = 0;
				funcoes = configura;					//quando a operação for finalizada, vai para o case "configura"
			}
		break;
		
		case prende:
			cmd_LCD(0x80,0);
			escreve_LCD("Prendendo peca   ");
			_delay_ms(1000);
			set_bit(PORTB,CILC);						//Abre o cilindro C
			set_bit(PORTB,CILA);						//Abre o cilindro A
			set_bit(PORTB,CILB);						//Abre o cilindro B
			if(((rd_bit(PIND,A1)) == 0) && ((rd_bit(PIND,B1)) == 0) && ((rd_bit(PINB,C1)) == 0) && ((rd_bit(PIND,SP)) == 0))		//caso os sensores A1,B1,C1,SP estejam ativos
			{
				while (rd_bit(PINC,PAUSE_BUT) == 0)		//Caso o botão de PAUSE sejá pressionado, permanecerá nesse loop
				{
				}
				funcoes = enche;			//quando os sensores estão ativos, vai para o case "enche"
			}
		break;
		
		case enche:
		
			clr_bit(PORTB,CILC);						//Fecha o cilindro C
			set_bit(PORTB,CILA);						//Abre o cilindro A
			set_bit(PORTB,CILB);						//Abre o cilindro B
			cmd_LCD(0x80,0);
			escreve_LCD("Enchendo peca    ");
			if(((rd_bit(PIND,A1)) == 0) && ((rd_bit(PIND,B1)) == 0) && ((rd_bit(PIND,C0)) == 0) && ((rd_bit(PIND,SP)) == 0))
			{
				while (rd_bit(PINC,PAUSE_BUT) == 0)					//Caso o botão de PAUSE sejá pressionado, permanecerá nesse loop
				{
				}
				for (tempo_envase;tempo_envase>0;tempo_envase--)	//tempo que o cilindro C fica aberto em segundos. Diminui a variavel tempo de envase e multiplica por 1000
				{
					_delay_ms(1000);									
				}
				funcoes = retorna;									//quando os sensores estão ativos, vai para o case "retorna"
			}
		break;
			
		case retorna:
			set_bit(PORTB,CILC);
			set_bit(PORTB,CILA);
			set_bit(PORTB,CILB);
			cmd_LCD(0x80,0);
			escreve_LCD("Retornando...    ");
			_delay_ms(5000);
			if(((rd_bit(PIND,A1)) == 0) && ((rd_bit(PIND,B1)) == 0) && ((rd_bit(PINB,C1)) == 0) && ((rd_bit(PIND,SP)) == 0))
			{
				while (rd_bit(PINC,PAUSE_BUT) == 0)		//caso o botão de PAUSE seja pressionado, mantém o usuário parado no loop
				{
				}
				funcoes = aguarda;						//quando os sensores estão ativos, volta para o case "aguarda"
				contador_lotes = contador_lotes + 1;	//acrescenta um no contador
			}
		break;
	}
}

//................................................................

int main(void)
{
	setup();		//chama a função setup		
	senha();		//chama a função senha
	cli();
	PORTC = 0b00011100;
	PCICR = (1<<PCIE1);
	PCMSK1 = (1<<PCINT10);	//define a entrada do botão parada de emergencia
	sei();
    while (1) 
    {
		loop();
    }
}

