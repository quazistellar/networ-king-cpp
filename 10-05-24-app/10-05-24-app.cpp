
// КЛИЕНТ (взаимодействует с сервером)

#define WIN32_LEAN_AND_MEAN //макрос

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

using namespace std;

int main()
{
    // служебная структура для хранение информации (ВЕРСИИ, СТРУКТУРЫ(НАПРИМЕР СЕМЕЙНУЮ)
    // о реализации Windows Sockets
    WSADATA wasData;

    // сокет соединения с сервером
    SOCKET connectSocket = INVALID_SOCKET;

    // две переменные используются для задания критериев поиска адресной информации,
    // они будут их в себе хранить (hints будет использоваться для указания параметров,
    //  которые устанавливаются перед вызовом getaddrinfo()
    ADDRINFO hints;
    ADDRINFO* addrResult;
    /*
       указатель addrResult типа addrinfo.
       после вызова функции getaddrinfo(), в эту переменную будет записан результат, 
       содержащий информацию об адресе, соответствующую заданным критериям.
       hints -  для задания параметров поиска адресной информации, 
       addrResult - для хранения результата поиска адреса.
    */

    // два сообщения от клиента на сервер
    const char* sendBuffer = "Hello from client\n";
    const char* sendBuffer2 = "How are u from client\n";

    //переменная для хранения данных которые будут переданы клиенту (размер буфера приема в байтах = 512)
    char recvBuffer[512];


    // старт использования библиотеки сокетов процессом определения версии и структуры
    int result = WSAStartup(MAKEWORD(2, 2), &wasData);
    // если произошла ошибка подгрузки библиотеки, то выводим ошибку 
    if (result != 0) {
        cout << " WSAStartup FAILED result" << endl;
        return 1;
    }

    // зануляем память и устанавливаем параметры
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;//4-БАЙТНЫЙ Ethernet (семейство адресов для getaddrinfo() )
    hints.ai_socktype = SOCK_STREAM; //задаем потоковый тип сокета
    hints.ai_protocol = IPPROTO_TCP;// Используем протокол TCP


    // получаем адрес сервера (ищем адрес с использованием параметров из hints
    result = getaddrinfo("localhost", "777", &hints, &addrResult);
    if (result != 0) {
        // выводим ошибку в случае если адрес был не получен/получен неверно
        cout << " getaddrinfo FAILED result" << endl;
        WSACleanup(); // очищаем wsastartup
        return 1;
    }

    // создаем клиентский сокет, тут семейство адресов тип протокол из addrResult
    connectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (connectSocket == INVALID_SOCKET) {
        // выводим ошибку если сокет был создан неверно
        cout << " ConnectSocket FAILED result" << endl;
        WSACleanup();
        FreeAddrInfo(addrResult);
        return 1;
    }

    // подключаемся к серверу по указанному адресу 
    result = connect(connectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        // выводим ошибку в случае неудачного подключения
        cout << " Connection FAILED result" << endl;
        closesocket(connectSocket);
        connectSocket = INVALID_SOCKET;
        FreeAddrInfo(addrResult);
        WSACleanup();
        return 1;
    }


    // отправляем первое сообщение на сервер
    result = send(connectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
    if (result == SOCKET_ERROR) {
        // выводим ошибку если сообщение не отправилось/отправилось неверно
        cout << " send FAILED result" << endl;
        WSACleanup();
        return 1;
    }
    //вывод в консоль отправленных данных
    cout << "send" << result << " bytes" << endl;

    // отправляем второе сообщения на сервер.
    result = send(connectSocket, sendBuffer2, (int)strlen(sendBuffer2), 0);
    if (result == SOCKET_ERROR) {
        cout << "Send failed, error: " << result << endl;
        closesocket(connectSocket); // закрытие сокета
        freeaddrinfo(addrResult);   // освобождение памяти, выделенной для addrResult (адреса)
        WSACleanup();               // доинициализируем библиотеку
        return 1;
    }
    //вывод в консоль отправленных данных
    cout << "send: " << result << " bytes" << endl;

 
    // прекращение отправки данных на сервер с помощью функции sd_send -для закрытия части соединения сокета, то есть закрываем отправку данных для этого сокета 
    result = shutdown(connectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        // выводим ошибку в случае если закрытие невозможно
        cout << " shutdown FAILED result" << endl;
        FreeAddrInfo(addrResult);
        WSACleanup();
        return 1;
    }

    // отправка сообщений на сервер
    do {
        // зануляем память, очищаем буфер для отправки данных
        ZeroMemory(recvBuffer, 512);
        // возврат колва полученных байтов
        result = recv(connectSocket, recvBuffer, 512, 0);
        if (result > 0) {
            cout << "Received " << result << "bytes" << endl;
            cout << "Received data " << recvBuffer << endl;
        }

        if (result == 0)
            // соединение закрыто после отправки данных
            cout << "Connection closed" << endl;
        else
            // ошибка при отправке
            cout << "recv failed with error" << endl;

    } while (result > 0); // завершение цикла отправки сообщений

    freeaddrinfo(addrResult); // закрываем сокет
    WSACleanup(); // доинициализируем библиотеку
    return 0; // завершение кода без ошибок
}