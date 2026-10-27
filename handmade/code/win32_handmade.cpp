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

struct win32_offscreen_buffer
{
    BITMAPINFO info;      // Guarda as informações do buffer, tamanho/formato
    void *memory;         // Ponteiro para buffer onde nossos pixels são desenhados
    int width;
    int height;
    int pitch;
};

struct win32_window_dimension
{
    int width;
    int height;
};

global_variable bool running;               // Mantém o programa aberto
global_variable win32_offscreen_buffer globalBackBuffer;

internal win32_window_dimension
getWindowDimension (HWND window)
{

    win32_window_dimension result;

    RECT clientRect;                        // Armazena o tamanho da janela
    GetClientRect(window, &clientRect);     // Método que retorna o tamanho
    
    // Calculando a quantidade de pixels de altura e largura
    result.width = clientRect.right - clientRect.left;
    result.height = clientRect.bottom - clientRect.top;

    return (result);
}

internal void
renderWeirdGradient(win32_offscreen_buffer buffer, int blueOffset, int greenOffset)
{
    // Este método é responsável por desenhar um gradiente
    // que se move no buffer.

    uint8_t *row = (uint8_t *) buffer.memory;
    for (int i = 0; i < buffer.height; ++i) 
    {
        uint32_t *pixel = (uint32_t *) row;
        for (int j = 0; j < buffer.width; ++j) 
        {
            uint8_t blue = (uint8_t)(j + blueOffset);
            uint8_t green = (uint8_t)(i + greenOffset);
            uint8_t red = 255;

            /*
            
                cada pixel possuí 32 bits (4 bytes)

                00000000 00000000 00000000 00000000

                Nós manipulamos cada um separadamente
                e adicionamos no pixel.

            */

            *pixel++ = ((red << 16) | (green << 8) | blue);
        }
        row += buffer.pitch;
    }
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height)
{
    // Este método serve para redimensionar o bitmapMemory
    // para que se mantenhha com o tamanho igual ao da janela

    if (buffer->memory)   // Verifica se a memória está ocupada e libera
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;
    int bytePerPixel = 4;

    // NOTE(Nicholas): Quando o biHeight é negativo, o windows preenche o
    // bitmap de cima para baixo, isso significa que o primeiro pixel da
    // janela é no canto superior esquerdo.

    // Preenchendo todas as informações do buffer
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);         // Especifica o número de byts exigidos pela estrutura
    buffer->info.bmiHeader.biWidth = buffer->width;                         // especifica a largura do bitmap em pixels
    buffer->info.bmiHeader.biHeight = -buffer->height;                      // especifica a altura do bitmap em pixels
    buffer->info.bmiHeader.biPlanes = 1;                                  // Especifica o número de planos para o dispositivo de destino
    buffer->info.bmiHeader.biBitCount = 32;                               // Especifica o número de bits por pixel (32 bits -> 4 bytes)
    buffer->info.bmiHeader.biCompression = BI_RGB;                        // Sem compressão, os bits são as cores puras
    //buffer->info.bmiHeader.biSizeImage = 0;
    //buffer->info.bmiHeader.biXPelsPerMeter = 0;
    //buffer->info.bmiHeader.biYPelsPerMeter = 0;
    //buffer->info.bmiHeader.biClrUsed = 0;
    //buffer->info.bmiHeader.biClrImportant = 0;

    // Calculando o tamanho de bytes da janela
    // e alocando memória através da API do windows
    int bitmapMemorySize {(buffer->width * buffer->height) * bytePerPixel};
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);    

    // A API do windows aloca memória através de páginas
    // isso é, a memória é dividida em pedaços de tamanho
    // fixo, o tamanho varia dependendo da versão e do OS
    // caso nós precisemos alocar poucos bytes, o sistema
    // nos entrega uma certa quantidade de páginas, e a 
    // parte das páginas que nós não usamos são apenas
    // desperdiçadas.

    buffer->pitch = width * bytePerPixel;
}

internal void 
Win32DisplayBufferInWindow(HDC deviceContext, 
                           int windowWidth, int windowHeight, 
                           win32_offscreen_buffer buffer) 
{
    // Copia o Buffer de pixels para dentro da janela
    // copia o bitmapMemory (buffer) para dentro da
    // janela através do deviceContext (Acesso para a janela criada)
    // Graças a anotação "SRCCOPY"

    StretchDIBits(deviceContext,                // Acesso ao buffer que queremos desenhar
                  /*
                  X, Y, width, height,          // Área que vamos pintar
                  X, Y, width, height,          // Qual pedaço do buffer vamos copiar
                  */
                  0, 0, windowWidth, windowHeight,
                  0, 0, buffer.width, buffer.height,
                  buffer.memory,                 // Buffer que será copiado
                  &buffer.info,                  // formato/tamanho do buffer
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

            win32_window_dimension dimension = getWindowDimension(window);

            // Método que atualiza a janela
            // copiando o deviceContext
            Win32DisplayBufferInWindow(deviceContext, dimension.width, dimension.height, 
                                       globalBackBuffer);

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

   Win32ResizeDIBSection(&globalBackBuffer, 1280, 720);   // Atualizando o buffer de desenho

    windowClass.style = CS_HREDRAW|CS_VREDRAW;                // É um conjunto de de sinalizadores binários, um conjunto de bits com propriedades para a janela
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
                renderWeirdGradient(globalBackBuffer, XOffset, YOffset);

                HDC deviceContext = GetDC(window);
                
                win32_window_dimension dimension = getWindowDimension(window);

                Win32DisplayBufferInWindow(deviceContext, dimension.width, dimension.height, 
                                           globalBackBuffer);
                
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

