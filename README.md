# DataPacket for C++
Store data as packet. Send, Recv, Encrypt it.

C#: https://github.com/emrecpp/DataPacket-CSharp

Python: https://github.com/emrecpp/PacketHandler

# Usage sample
```cpp

#include <iostream>
#include <Windows.h>
#include "Packet.h"


int main(){
    // On Send:
    Packet paket = Packet(0x1234, false);
    SYSTEMTIME NowTime; // Custom struct (Note: struct size must be lower than 16 bytes else it will be corrupted.)
    GetSystemTime(&NowTime);

    paket << "Emre" << 123 << 1.5f << NowTime;
    bool sent = paket.Send(socket);



    // On Recv:
    Packet receivedPacket = Packet();
    if (receivedPaket.Recv(socket)) {		
        string name; int number; float f; SYSTEMTIME SystemTimeStruct;
        paket >> name >> number >> f >> SystemTimeStruct;
        printf("Opcode: %X\n\nName: %s\nNumber: %d\nFloat: %f\nSystemTime: %d-%02d-%02d\n", paket.GetOpcode(), name.c_str(), number, f,
            SystemTimeStruct.wYear, SystemTimeStruct.wMonth, SystemTimeStruct.wDay);

        paket.Print("MY TITLE");
    }// else connection lost
    std::getchar();
    return 0;    
}
```


#### Output:
```
Opcode: 1234
Name: Emre
Number: 123
Float: 1.500000
SystemTime: 2022-03-03



Little Endian (Packet(0x1234, false)):

*** MY TITLE (Size: 38) ***
00000000 12 34 00 03 00 00 00 00 00 04 45 6D 72 65 00 00    .4........Emre..
00000010 00 7B 3F C0 00 00 E6 07 03 00 04 00 03 00 0B 00    .{?.............
00000020 38 00 10 00 80 01                                   8.....



Big Endian    (Packet(0x1234, true)):

*** MY TITLE (Size: 38) ***
00000000 12 34 00 03 00 00 00 00 00 04 45 6D 72 65 7B 00    .4........Emre{.
00000010 00 00 00 00 C0 3F E6 07 03 00 04 00 03 00 0B 00    .....?..........
00000020 38 00 10 00 80 01                                   8.....

```

