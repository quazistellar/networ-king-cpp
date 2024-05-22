#define WIN32_LEAN_AND_MEAN // макрос, что-то на умном - директива препроцессора удаляет из заголовочных файлов Win32 некоторые неиспользуемые функции и макросы, что уменьшает размер исполняемого файла

#include <iostream> // стандартная библиотека ввода вывода
#include <Windows.h> // подключает основной заголовочный файл для Windows API, который содержит объявления для большинства функций и структур винды
#include <WinSock2.h> // включает заголовочный файл для Winsock API, библиотека сокетов, которая предоставляет функции и структуры для сетевого программирования
#include <WS2tcpip.h> // библиотека протокола tcpIP, включает заголовочный файл для расширенных функций Winsock (getaddrinfo() и bind())
using namespace std;


int main()
{

    // служебная структура для хранение информации (ВЕРСИИ, СТРУКТУРЫ(НАПРИМЕР СЕМЕЙНУЮ)
   // о реализации Windows Sockets
    WSADATA wsaData;


    SOCKET connectSocket = INVALID_SOCKET; // сокет соединения, пока везде значения invalid тк сокеты недействительны в начале программы
    SOCKET listenSocket = INVALID_SOCKET; // сокет прослушивания


    // две переменные используются для задания критериев поиска адресной информации,
    // они будут их в себе хранить (hints будет использоваться для указания параметров,
    //  которые устанавливаются перед вызовом getaddrinfo()

    ADDRINFO hints; 
    // указатель addrResult типа addrinfo.
    // после вызова функции getaddrinfo(), в эту переменную будет записан результат, 
    // содержащий информацию об адресе, соответствующую заданным критериям.

    //  hints -  для задания параметров поиска адресной информации, 
    //  addrResult - для хранения результата поиска адреса.
    ADDRINFO* addrResult;

    // сообщение от сервера которое будет передано клиенту
    const char* sendBuffer = "Hello from Server\n";


    //переменная для хранения данных которые будут переданы клиенту (размер буфера приема в байтах = 512)
    char recvBuffer[512];

    // старт использования библиотеки сокетов процессом определения версии и структуры
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    // проверка на то, не было ли ошибок при использщовании библиотеки
    if (result != 0) {
        cout << "WSAStartup failed result" << endl;
        return 1;
    }

    // необходимо изначально занулить память структуры hints, 1-ый параметр - что зануляем, 2-ой - сколько
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET;//4-БАЙТНЫЙ Ethernet (семейство адресов для getaddrinfo() )
    hints.ai_socktype = SOCK_STREAM; //задаем потоковый тип сокета
    hints.ai_protocol = IPPROTO_TCP;// Используем протокол TCP
    hints.ai_flags = AI_PASSIVE;// Пассивная сторона, потому что просто ждет соединения


    // выполняет поиск имени хоста для порта 777 и сохраняет результаты в addrResult, если возникла ошибка 
    // (например хост не найден - она выводится на экран)
    // тут в целом функция  хранит в себе адрес, порт,семейство структур, адрес сокета
    result = getaddrinfo(NULL, "777", &hints, &addrResult);
    if (result != 0) {
        // Если инициализация структуры адреса завершилась с ошибкой,
        // выведем сообщением об этом и завершим выполнение программы 
        cout << "getaddrinfo failed with error" << endl;
        WSACleanup(); // очищаем wsastartup
        return 1;
    }

    // тут создаем сокет прослушивания на основе параметров, полученных от getaddrinfo().

    listenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        // если сокет недействительный - выводим ошибку
         // освобождаем память, выделенную под структуру addr,
        cout << "socket error" << endl;
        WSACleanup(); // очищаем wsastartup
        freeaddrinfo(addrResult);
        return 1;
    }

    // Привязываем сокет к IP-адресу (соединились с сервером)
    result = bind(listenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        // Если привязать адрес к сокету не удалось, то выводим сообщение
        // об ошибке, освобождаем память, выделенную под структуру addr
        // и закрываем открытый сокет

        // Выгружаем DLL-библиотеку из памяти и закрываем программу.
        cout << "Binding connect failed <3" << endl;
        closesocket(connectSocket);
        listenSocket = INVALID_SOCKET;
        freeaddrinfo(addrResult);
        WSACleanup(); // очищаем wsastartup
        return 1;
    }

    // Устанавливает сокет прослушивания в режим прослушивания 
    // с помощью  listen(). 
    // SOMAXCONN — это макрос, который указывает
    // максимальное количество ожидающих соединений.
    result = listen(listenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {

        // если сокет не перешел в прослушивание то выводится ошибка

        cout << "Listening failed" << endl;
        closesocket(connectSocket);
        listenSocket = INVALID_SOCKET;
        // освобождаем память выделенную под addrResult
        freeaddrinfo(addrResult);
        WSACleanup(); // очищаем wsastartup
        return 1;
    }


    // функция нужна для блокирования и ожидания того
    // как клиент установит соединение с listenSocket - сокетом прослушивания
    connectSocket = accept(listenSocket, NULL, NULL);
    if (connectSocket == INVALID_SOCKET) {
        // в противном случае (сервер не соединился с клиентом - выводится ошибка)
        cout << "Accepting SOCKET failed" << endl;
        closesocket(connectSocket);
        listenSocket = INVALID_SOCKET;
        freeaddrinfo(addrResult);
        WSACleanup(); // очищаем wsastartup
        return 1;
    }
    // закрытие сокета прослушивания
    closesocket(listenSocket);
   

    // тут происходит обмен данными с клиентом 
    do {
        // зануляем память для буфера приема 
        ZeroMemory(recvBuffer, 512);
        // ожидание данных, recv потом вернет количество байт которые пришли от клиента (recvBuffer - указатель на буфер для хранения полученных данных)
        result = recv(connectSocket, recvBuffer, 512, 0);

        if (result > 0) {
            // вывод полученных данных 
            cout << "Received " << result << "bytes" << endl;
            cout << "Received data " << recvBuffer << endl;

            // функция для отправки сообщения (которое находится в буфере sendBuffer)
            result = send(connectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                // если с сокетом произошла ошибка - выводится сообщение о том что произошла ошибка отправления
                cout << "failed to send" << endl;
                WSACleanup(); // очищаем wsastartup
                return 1;
            }
        }
        // если все хорошо и все отправилось - коннет закрывается
        else if (result == 0)
            cout << "Connection closing" << endl;
        // иначе ошибка принятия данных
        else
            cout << "recv failed with error" << endl;

    } while (result > 0);

    // тут клиент уже получил данные от сервера, соединение закрывается
    result = shutdown(connectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        freeaddrinfo(addrResult);
        WSACleanup(); // очищаем wsastartup
        return 1;
    }
    // закрываем соединение с сокетом и доинициализируем библиотеку
    closesocket(connectSocket);
    freeaddrinfo(addrResult);
    WSACleanup(); 
    return 0;
}