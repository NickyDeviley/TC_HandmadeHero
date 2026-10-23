#include <windows.h>
#include <stdint.h>
/*
    Para criar uma aplicação Windows, que utiliza a Win32
    para manipular janelas, aúdio, etc. Utilização o método
    WinMain como porta de entrada para código, ao contrário
    do padrão normal da linguagem C/C++, que seria o método
    main.
*/

// Definindo nomes para static com o objetivo
// de melhorar a leitura do código
#define internal static                     // Método internos, funcionam apenas neste arquivo 
#define local_persistent static             // Variável local persistente, ficam salvas após o contexto onde foram declaradas terminar
#define global_variable static              // Variável global, funciona apenas neste arquivo

global_variable bool running;               // Mantém o programa aberto

global_variable BITMAPINFO bitmapInfo;      // Guarda as informações do buffer, tamanho/formato
global_variable void *bitmapMemory;         // Ponteiro para buffer onde nossos pixels são desenhados

global_variable int bitmapWidth;
global_variable int bitmapHeight;
global_variable int bytePerPixel {4};

internal void
renderWeirdGradient(int blueOffset, int greenOffset)
{
    // Este método é responsável por desenhar um gradiente
    // que se move no buffer.

    int width = bitmapWidth;
    int height = bitmapHeight;

    int pitch = width * bytePerPixel;
    uint8_t *row = (uint8_t *) bitmapMemory;
    for (int i = 0; i < bitmapHeight; ++i) 
    {
        uint32_t *pixel = (uint32_t *) row;
        for (int j = 0; j < bitmapWidth; ++j) 
        {
            uint8_t blue = (uint8_t)(j + blueOffset);
            uint8_t green = (uint8_t)(i + greenOffset);
            uint8_t red = 0;

            /*
            
                cada pixel possuí 32 bits (4 bytes)

                00000000 00000000 00000000 00000000

                Nós manipulamos cada um separadamente
                e adicionamos no pixel.

            */

            *pixel++ = ((green << 8) | blue);
        }
        row += pitch;
    }
}

internal void
Win32ResizeDIBSection(int width, int height)
{
    // Este método serve para redimensionar o bitmapMemory
    // para que se mantenhha com o tamanho igual ao da janela

    if (bitmapMemory)   // Verifica se a memória está ocupada e libera
    {
        VirtualFree(bitmapMemory, 0, MEM_RELEASE);
    }

    bitmapWidth = width;
    bitmapHeight = height;

    // Preenchendo todas as informações do buffer
    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);         // Especifica o número de byts exigidos pela estrutura
    bitmapInfo.bmiHeader.biWidth = bitmapWidth;                         // especifica a largura do bitmap em pixels
    bitmapInfo.bmiHeader.biHeight = -bitmapHeight;                      // especifica a altura do bitmap em pixels
    bitmapInfo.bmiHeader.biPlanes = 1;                                  // Especifica o número de planos para o dispositivo de destino
    bitmapInfo.bmiHeader.biBitCount = 32;                               // Especifica o número de bits por pixel (32 bits -> 4 bytes)
    bitmapInfo.bmiHeader.biCompression = BI_RGB;                        // Sem compressão, os bits são as cores puras
    //bitmapInfo.bmiHeader.biSizeImage = 0;
    //bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
    //bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
    //bitmapInfo.bmiHeader.biClrUsed = 0;
    //bitmapInfo.bmiHeader.biClrImportant = 0;

    // Calculando o tamanho de bytes da janela
    // e alocando memória através da API do windows
    int bitmapMemorySize {(width * height) * bytePerPixel};
    bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);    

    // A API do windows aloca memória através de páginas
    // isso é, a memória é dividida em pedaços de tamanho
    // fixo, o tamanho varia dependendo da versão e do OS
    // caso nós precisemos alocar poucos bytes, o sistema
    // nos entrega uma certa quantidade de páginas, e a 
    // parte das páginas que nós não usamos são apenas
    // desperdiçadas.

}

internal void 
Win32UpdateWindow(HDC deviceContext, RECT *windowRect, int X, int Y, int  width, int height) 
{
    // Copia o Buffer de pixels para dentro da janela
    // copia o bitmapMemory (buffer) para dentro da
    // janela através do deviceContext (Acesso para a janela criada)
    // Graças a anotação "SRCCOPY"

    int windowWidth {windowRect->right - windowRect->left};
    int windowHeight {windowRect->bottom - windowRect->top};

    StretchDIBits(deviceContext,                // Acesso ao buffer que queremos desenhar
                  /*
                  X, Y, width, height,          // Área que vamos pintar
                  X, Y, width, height,          // Qual pedaço do buffer vamos copiar
                  */
                  0, 0, bitmapWidth, bitmapHeight,
                  0, 0, windowWidth, windowHeight,
                  bitmapMemory,                 // Buffer que será copiado
                  &bitmapInfo,                  // formato/tamanho do buffer
                  DIB_RGB_COLORS, SRCCOPY);     // Indica que os pixels são cores RGB

}

LRESULT CALLBACK 
Win32MainWindowCallBack(
    HWND window,        // Identificador de janela
    UINT message,       // O OS usa as mensagens para controlar e enviar informações à aplicativos
    WPARAM WParam,      // Informações adicionais da mensagem
    LPARAM LParam)      // Informações adicionais da mensagem
{
    // Quando o windows dispacha uma mensagem
    // para nossa janela este é o método que ele chama.
    // por isso a anotação "CALLBACK"

    LRESULT result {0}; // Valor que será retornado caso a mensagem seja tratada

    // Usamos um Switch para tratar as mensagens
    // que o sistema envio para o método
    switch (message) {
        case WM_SIZE: {                             // Redimensinar a janela
            // Quando a janela é redimensionada,
            // Nós precisamos saber o tamanho
            // dela, para que possamos redimensionar
            // o Buffer de desenho

            RECT clientRect;                        // Armazena o tamanho da janela
            GetClientRect(window, &clientRect);     // Método que retorna o tamanho
            
            // Calculando a quantidade de pixels de altura e largura
            int width {clientRect.right - clientRect.left};
            int height {clientRect.bottom - clientRect.top};
            
            Win32ResizeDIBSection(width, height);   // Atualizando o buffer de desenho
        } break;
        case WM_CLOSE: {                            // fechar através do X
            running = false;
        } break;
        case WM_ACTIVATEAPP: {                      // A janela está em primeiro plano
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        case WM_DESTROY: {                          // Destruir a janela
            running = false;
        } break;
        case WM_PAINT: {                            // Pintar algo na janela

            // contém as informações da área que podemos desenhar
            PAINTSTRUCT paint; 

            // DeviceContext é um acesso ao buffer da janela
            // não é onde nós vamos desenhar diretamente
            // iremos desenhar em um backbuffer e depois
            // copiar o seu conteúdo para o buffer da janela
            HDC deviceContext {BeginPaint(window, &paint)};
            
            int X {paint.rcPaint.left};
            int Y {paint.rcPaint.top};
            int height {paint.rcPaint.bottom - paint.rcPaint.top};
            int width {paint.rcPaint.right - paint.rcPaint.left};

            RECT clientRect;
            GetClientRect(window, &clientRect);

            // Método que atualiza a janela
            // copiando o deviceContext
            Win32UpdateWindow(deviceContext, &clientRect, X, Y, width, height);

            EndPaint(window, &paint);

        } break;
        default: {
//            OutputDebugStringA("default\n");
            result = DefWindowProcA(window, message, WParam, LParam);
        }   break;

    }

    return(result);
}

// Método de entrada do código
int CALLBACK WinMain(
    HINSTANCE instance,         // Ponteiro para o endereço de memória inicial onde o .exe foi carregado
    HINSTANCE prevInstance,     // Ponteiro para a antiga instancia na memória (Não é mais utilizado, é mantido por compatibilidade)
    LPSTR     commandLine,      // Equivalente ao argv, para adicionar parâmetros ao executar o programa
    int       showCode)         // Armazena o estado da janela através de anotações SW_*, que determinam como a janela do programa inicia (raramente utilizada)
{

    // Criando uma Struct de Janela
    // Inicializando a struct com todos os valores zerados
    WNDCLASSA windowClass {};

    // Preenchendo todos os valores do Struct de janela
    // Atribuímos um DC (Device Context) próprio
    //windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;       // É um conjunto de de sinalizadores binários, um conjunto de bits com propriedades para a janela
    windowClass.lpfnWndProc = Win32MainWindowCallBack;        // É um ponteiro para uma função para determinar como a janela responde a eventos
    //windowClass.cbClsExtra;                                   // Serve para armazenar bytes extras junto a instância da janela
    //windowClass.cbWndExtra;                                   // Função semelhante ao cbClsExtra, onde armazenamos memória extra junto a instância da janela
    windowClass.hInstance = instance;                         // A intância que define a janela
    windowClass.hIcon;                                        // Define um ícone para a janela
    //windowClass.hCursor;                                      // Define um cursor
    //windowClass.hbrBackground;                                // Limpa o fundo da janela
    //windowClass.lpszMenuName;                                 // Adiciona um menu semelhante ao do notepad do windows
    windowClass.lpszClassName = "HandmadeHeroWindowClass";    // Define um nome para a janela 

    // Método que registra a classe janela 
    // Ele exige um ponteiro para a var. comp. windowClass
    if (RegisterClassA(&windowClass))
    {
        
        // Criando uma janela, atribuímos ao tipo HWND 
        // que é uma variável que armazena a janela depois de criada
        // o método responsável por criar uma janela e passamos
        // como parâmetro os dados que queremos
        HWND window {CreateWindowExA(0,                               // Estilo da janela (A maneira como a janela se comporta a certas ações)
                                           windowClass.lpszClassName,       // Nome da classe
                                           "Handmade Hero",                 // Nome da janela
                                           WS_OVERLAPPEDWINDOW|WS_VISIBLE,  // Comportamentos específicos da janela
                                           CW_USEDEFAULT,                   // Posicao inicial X da janela
                                           CW_USEDEFAULT,                   // Posicao inicial Y da janela
                                           CW_USEDEFAULT,                   // Largura da janela
                                           CW_USEDEFAULT,                   // Altura da janela
                                           0,               // Identificador de janela pai (Serve para criar janelas menores que responde a essa janela pai)
                                           0,               // Para utilizar um menu em janelas filho
                                           instance,        // Identificador da instância da janela (Onde está a janela na memória)
                                           0)};             // Ponteiro para um valor a ser passado para a janela

        // Verifica se a janela foi criada
        if (window) 
        {
            int XOffset = 0;
            int YOffset = 0;
            running = true; // inicia a variável que mantém o loop rodando
            while (running) 
            {
                // Loop que pede ao windows todas as mensagens pendentes
                // referente a qualquer janela nesta thread, se foi redimensionada,
                // se está em primeiro plano, se foi fechada, se foi clicada, etc.
                // O código verifica se a mensagem é para a nossa janela
                // e então trata ela.


                MSG message;    // Variável mensagem
                while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
                {
                    // Se existe uma mensagem traduz e dispacha ela
                    // "dispachar" significa que pedimos ao windows
                    // para enviar a mensagem para a janela certa
                    // no caso a que nós criamos e, como atribuímos
                    // o metodo Win32MainWindowCallBack a nossa
                    // janela, o windows chama este método para
                    // tratar a mensagem
                    if (message.message == WM_QUIT) 
                    {
                        running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
                renderWeirdGradient(XOffset, YOffset);

                HDC deviceContext = GetDC(window);
                
                RECT clientRect;
                GetClientRect(window, &clientRect);

                int windowWidth {clientRect.right - clientRect.left};
                int windowHeight {clientRect.bottom - clientRect.top};
                
                Win32UpdateWindow(deviceContext, &clientRect, 0, 0, windowWidth, windowHeight);
                
                ReleaseDC(window, deviceContext);
                
                ++XOffset;
                ++YOffset;
            }
        }
        else 
        {
            // TODO(Nicholas): Implementar
        }
    }
    else 
    {
        // TODO(Nicholas): Implementar
    }

    return(0);
}

