#include <windows.h>

/*
    Para criar uma aplicação Windows, que utiliza a Win32
    para manipular janelas, aúdio, etc. Utilização o método
    WinMain como porta de entrada para código, ao contrário
    do padrão normal da linguagem C/C++, que seria o método
    main.
*/
int CALLBACK WinMain(
    HINSTANCE instance,         // Ponteiro para o endereço de memória inicial onde o .exe foi carregado
    HINSTANCE prevInstance,     // Ponteiro para a antiga instancia na memória (Não é mais utilizado, é mantido por compatibilidade)
    LPSTR     commandLine,      // Equivalente ao argv, para adicionar parâmetros ao executar o programa
    int       showCode)         // Armazena o estado da janela através de anotações SW_*, que determinam como a janela do programa inicia (raramente utilizada)
{

    // Abre uma mensagem para o usuário
    // É necessário adicionar "A" à função
    // pois o MessageBox é genérico e determinado
    // pelo comando de build
    MessageBoxA(0, "This is Handmade Hero!", "Handmade Hero", 
               MB_OK|MB_ICONINFORMATION);

    return(0);
}