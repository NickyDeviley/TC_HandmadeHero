#include <windows.h>

/*
    Para criar uma aplicação Windows, que utiliza a Win32
    para manipular janelas, aúdio, etc. Utilização o método
    WinMain como porta de entrada para código, ao contrário
    do padrão normal da linguagem C/C++, que seria o método
    main.
*/

LRESULT CALLBACK 
mainWindowCallBack(
    HWND window,        // Identificador de janela
    UINT message,       // O OS usa as mensagens para controlar e enviar informações à aplicativos
    WPARAM WParam,      // 
    LPARAM LParam)      // 
{

    LRESULT result {0}; // Valor que será retornado

    // Tratando as mensagens do OS
    switch (message) {
        case WM_SIZE: {                             // Mensagem para redimensinar a janela
            OutputDebugStringA("WM_SIZE\n");        
        } break;
        case WM_DESTROY: {                          // Mensagem para destruir a janela
            OutputDebugStringA("WM_DESTROY\n");
        } break;
        case WM_CLOSE: {                            // Mensagem para fechar através do X
            OutputDebugStringA("WM_CLOSE\n");
        } break;
        case WM_ACTIVATEAPP: {                      // Mensagem para quando a janela está em primeiro plano
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        case WM_PAINT: {                            // Mensagem para pintar algo na janela

            PAINTSTRUCT paint;                      // Criando uma variável composta para pintura
            HDC deviceContext = BeginPaint(window, &paint); // Método que inicia a pintura na janela
            int X {paint.rcPaint.left};
            int Y {paint.rcPaint.top};
            int height {paint.rcPaint.bottom - paint.rcPaint.top};
            int width = {paint.rcPaint.right - paint.rcPaint.left};

            static DWORD operation = WHITENESS;

            PatBlt(deviceContext, X, Y, width, height, operation);  // Método que pinta na janela
            
            // Trocar de cor de branco para preto e visseversa
            if (operation == WHITENESS) {
                operation = BLACKNESS;
            }
            else {
                operation = WHITENESS;
            }

            EndPaint(window, &paint);               // Método que finaliza a pintura na janela

        } break;
        default: {
//            OutputDebugStringA("default\n");
            result = DefWindowProc(window, message, WParam, LParam);
        }   break;

    }

    return(result);
}

int CALLBACK WinMain(
    HINSTANCE instance,         // Ponteiro para o endereço de memória inicial onde o .exe foi carregado
    HINSTANCE prevInstance,     // Ponteiro para a antiga instancia na memória (Não é mais utilizado, é mantido por compatibilidade)
    LPSTR     commandLine,      // Equivalente ao argv, para adicionar parâmetros ao executar o programa
    int       showCode)         // Armazena o estado da janela através de anotações SW_*, que determinam como a janela do programa inicia (raramente utilizada)
{

    // Criando uma Struct de Janela
    WNDCLASSA windowClass {};   // Inicializando a struct com todos os valores zerados

    // Atribuímos um DC (Device Contest) próprio
    windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;     // É um conjunto de de sinalizadores binários, um conjunto de bits com propriedades para a janela
    windowClass.lpfnWndProc = mainWindowCallBack;           // É um ponteiro para uma função para determinar como a janela responde a eventos
    //windowClass.cbClsExtra;                               // Serve para armazenar bytes extras junto a instância da janela
    //windowClass.cbWndExtra;                               // Função semelhante ao cbClsExtra, onde armazenamos memória extra junto a instância da janela
    windowClass.hInstance = instance;                       // A intância que define a janela
    windowClass.hIcon;                                      // Define um ícone para a janela
    //windowClass.hCursor;                                  // Define um cursor
    //windowClass.hbrBackground;                            // Limpa o fundo da janela
    //windowClass.lpszMenuName;                             // Adiciona um menu semelhante ao do notepad do windows
    windowClass.lpszClassName = "HandmadeHeroWindowClass";  // Define um nome para a janela 

    // Método para abrir uma janela que pede um ponteiro para
    // a variável composta windowClass
    if (RegisterClassA(&windowClass)) {
        
        HWND windowHandle = CreateWindowExA(    // Método que cria uma janela
        0,                               // Estilo da janela (A maneira como a janela se comporta a certas ações)
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
        0);              // Ponteiro para um valor a ser passado para a janela

        if (windowHandle) {
            MSG message;
            for (;;) {
                BOOL MessageResult = GetMessage(&message, 0, 0, 0);
                if (MessageResult > 0) {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
                else {
                    break;
                }
            }
        }
        else {
            // TODO(Nicholas): Continuar
        }
    }
    else {
        // TODO(Nicholas): Continuar
    }
    return(0);
}

