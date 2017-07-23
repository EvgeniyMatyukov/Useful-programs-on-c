//работа с uart
#define RXB8 1
#define TXB8 0
#define UPE 2
#define DOR 3
#define FE 4
#define UDRE 5
#define RXC 7
#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<DOR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)

// USART0 Transmitter buffer
#define TX_BUFFER_SIZE0 64
volatile unsigned char tx_buffer0[TX_BUFFER_SIZE0];

#if TX_BUFFER_SIZE0<256
volatile unsigned char tx_wr_index0,tx_rd_index0,tx_counter0;
#else
volatile unsigned int tx_wr_index0,tx_rd_index0,tx_counter0;
#endif

// USART0 Receiver buffer
#define RX_BUFFER_SIZE0 64
volatile unsigned char rx_buffer0[RX_BUFFER_SIZE0];

#if RX_BUFFER_SIZE0<256
volatile unsigned char rx_wr_index0,rx_rd_index0,rx_counter0;
#else
volatile unsigned int rx_wr_index0,rx_rd_index0,rx_counter0;
#endif

// This flag is set on USART0 Receiver buffer overflow
volatile bool rx_buffer_overflow0;

volatile bool readlineen=0;

// USART0 Receiver interrupt service routine
ISR (USART0_RX_vect)
{
	char status,data;
	status=UCSR0A;
	data=UDR0;

	if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
	{
		rx_buffer0[rx_wr_index0]=data;
		if (data==0x0D) readlineen=1; //если принята строка, то обработать её

		if (++rx_wr_index0 == RX_BUFFER_SIZE0) rx_wr_index0=0;
		if (++rx_counter0 == RX_BUFFER_SIZE0)
		{
			rx_counter0=0;
			rx_buffer_overflow0=1;
		};

	};
}

char uart_getchar(void)
{
	char data;
	while (rx_counter0==0);
	data=rx_buffer0[rx_rd_index0];
	if (++rx_rd_index0 == RX_BUFFER_SIZE0) rx_rd_index0=0;
	asm("cli");
	--rx_counter0;
	asm("sei");
	return data;
}



// USART0 Transmitter interrupt service routine
ISR (USART0_TX_vect)
{
	if (tx_counter0)
	{
		--tx_counter0;
		UDR0=tx_buffer0[tx_rd_index0];
		if (++tx_rd_index0 == TX_BUFFER_SIZE0) tx_rd_index0=0;
	};
}

void uart_putchar(char c)
{
	while (tx_counter0 == TX_BUFFER_SIZE0);
	asm("cli");
	if (tx_counter0 || ((UCSR0A & DATA_REGISTER_EMPTY)==0))
	{
		tx_buffer0[tx_wr_index0]=c;
		if (++tx_wr_index0 == TX_BUFFER_SIZE0) tx_wr_index0=0;
		++tx_counter0;
	}
	else
	UDR0=c;
	asm("sei");
}

void uart_writeline(char *string) //отправить строку через буфер (на прерываниях)
{
	unsigned char sym=0;     //указатель на текущий символ
	while(string[sym])  {     //пока символ не нулевой
		uart_putchar(string[sym]);  //отправить текущий символ
		sym++;                 //обработать следующий символ
	}
}

void uart_writeline(char *string, unsigned int sym) //отправить строку через буфер (на прерываниях), sym - количество знаков в строке
{
	unsigned int length=sym;
	while(sym)  {     //пока символ не нулевой
		uart_putchar(string[length-sym]);  //отправить текущий символ
		sym--;
	}
}
